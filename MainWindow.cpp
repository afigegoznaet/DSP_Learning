#include "MainWindow.hpp"
#include "ui_MainWindow.h"
#include "SoundFile.hpp"

#include <QValueAxis>
#include <QLineSeries>
#include <QAudioDeviceInfo>
#include <QAudioOutput>
#include <QDebug>
constexpr auto X_SAMPLES = 1024;
constexpr auto SampleRate = 48000;
constexpr auto BitDepth = 32;
using namespace QtCharts;
MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent), ui(new Ui::MainWindow), amplitudes(new QLineSeries) {
	ui->setupUi(this);
	setupAudio();
	setupAmplitudeChart();
	connect(ui->volumeSlider, &QSlider::valueChanged, this, [this](int val) {
		audioOutput->setVolume(val / 100.);
		showInfo();
	});

	ui->volumeSlider->setValue(90);
	audioFile = new SoundFile(this);
	connect(audioFile, SIGNAL(dataRead(char *, int)), this,
			SLOT(showData(char *, int)), Qt::QueuedConnection);

	audioFile->setFrequency(1);
	qDebug() << audioFile->open(QIODevice::ReadOnly);
}

MainWindow::~MainWindow() { delete ui; }


void MainWindow::setupAmplitudeChart() {
	auto chart = ui->signalView->chart();

	// m_amplitudes->setUseOpenGL(true);
	// m_chart->addSeries(m_amplitudes);
	QValueAxis *axisX = new QValueAxis;
	QValueAxis *axisY = new QValueAxis;

	axisX->setRange(0, X_SAMPLES);
	axisX->setLabelFormat("%g");
	axisX->setTitleText("Data Points");

	axisY->setRange(-1, 1);
	axisY->setTitleText("Audio level");


	chart->addAxis(axisX, Qt::AlignBottom);
	chart->addAxis(axisY, Qt::AlignLeft);
	chart->addSeries(amplitudes);
	amplitudes->attachAxis(axisX);
	amplitudes->attachAxis(axisY);

	chart->legend()->hide();
	chart->setTitle("Input data");

	amplitudes->setUseOpenGL(true);

	internalBuffer.reserve(X_SAMPLES);
	for (int i = 0; i < X_SAMPLES; ++i)
		internalBuffer.append(QPointF(i, 0));
}


void MainWindow::setupAudio() {

	const auto deviceInfo = QAudioDeviceInfo::defaultOutputDevice();

	QAudioFormat format;

	format.setSampleRate(SampleRate);
	format.setChannelCount(1);
	format.setSampleSize(BitDepth);
	format.setCodec("audio/pcm");
	format.setByteOrder(QAudioFormat::LittleEndian);
	format.setSampleType(QAudioFormat::Float);

	if (!deviceInfo.isFormatSupported(format)) {
		qWarning() << "Default format not supported - trying to use nearest";
		format = deviceInfo.nearestFormat(format);
		qDebug() << format.sampleRate();
		qDebug() << format.channelCount();
		qDebug() << format.sampleSize();
		qDebug() << format.codec();
		qDebug() << format.byteOrder();
		qDebug() << format.sampleType();
	}

	audioOutput = new QAudioOutput(deviceInfo, format, this);
}

void MainWindow::on_startStopButton_toggled(bool checked) {
	if (checked) {
		audioOutput->start(audioFile);
		ui->startStopButton->setText("Stop");
	} else {
		audioOutput->stop();
		ui->startStopButton->setText("Start");
	}
}

void MainWindow::showInfo() {
	auto infoString = QString::asprintf(
		"Audio level: %.2f\n"
		"Sampling frequency: %d\n"
		"Bit depth: %d\n",
		ui->volumeSlider->value() / 100., SampleRate, BitDepth);
	ui->infoLabel->setText(infoString);
}


void MainWindow::showData(char *data, int len) {
	qDebug() << "show data";
	if (!chartReady)
		return;
	chartReady = false;

	const auto *actualData = reinterpret_cast<const float *>(data);

	for (int i = 0; i < X_SAMPLES; i++) {
		internalBuffer[i].setY(actualData[i]);
	}
	amplitudes->replace(internalBuffer);
	chartReady = true;
}
