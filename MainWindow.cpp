#include "MainWindow.hpp"
#include "ui_MainWindow.h"
#include "SoundFile.hpp"

#include <QValueAxis>
#include <QCategoryAxis>
#include <QLineSeries>
#include <QAudioDeviceInfo>
#include <QAudioOutput>
#include <QtConcurrent/QtConcurrent>
#include <QDebug>

using namespace QtCharts;
char sinToShow[SampleRate * sizeof(float)];
char specToShow[SampleRate * sizeof(float)];

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent), ui(new Ui::MainWindow), amplitudes(new QLineSeries), frequencies(new QLineSeries) {
	ui->setupUi(this);
	setupAudio();
	setupAmplitudeChart();
	setupFrequencyChart();
	connect(ui->volumeSlider, &QSlider::valueChanged, this, [this](int val) {
		audioOutput->setVolume(val / 100.);
		showInfo();
	});

	ui->volumeSlider->setValue(90);
	audioFile = new SoundFile(this);
	connect(audioFile, &SoundFile::dataRead, this,
			[this](const char *data, int len) {
				QtConcurrent::run(this, &MainWindow::showData, data, len);
			});

	audioFile->setSineFrequency(200);
	qDebug() << audioFile->open(QIODevice::ReadOnly);
}

MainWindow::~MainWindow() {
	audioFile->close();
	delete ui;
}


void MainWindow::setupAmplitudeChart() {
	auto chart = ui->signalView->chart();

	// m_amplitudes->setUseOpenGL(true);
	// m_chart->addSeries(m_amplitudes);
	QValueAxis *axisX = new QValueAxis;
	QValueAxis *axisY = new QValueAxis;

	axisX->setRange(0, X_VALS);
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

	for (auto i = 0; i < X_VALS; i++)
		amplitudesBuffer.append({float(i), 0});
	amplitudes->replace(amplitudesBuffer);
}
int MainWindow::getFreqPos(int freq) {
	return freq * X_VALS * 2 / SampleRate;
}

void MainWindow::setupFrequencyChart() {
	auto chart = ui->dftView->chart();

	// m_amplitudes->setUseOpenGL(true);
	// m_chart->addSeries(m_amplitudes);
	QCategoryAxis *axisX = new QCategoryAxis;
	axisX->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);
	QValueAxis *axisY = new QValueAxis;

	axisX->setRange(0, X_VALS);
	axisX->setLabelFormat("%g");
	axisX->setTitleText("Spectrogram");

	axisX->append("0", 0);
	axisX->append("200", getFreqPos(200));
	axisX->append("6000", getFreqPos(6000));
	axisX->append("7500", getFreqPos(7500));
	axisX->append("12000", getFreqPos(12000));
	axisX->append("18000", getFreqPos(18000));
	axisX->append("24000", getFreqPos(24000));

	axisY->setRange(-X_VALS, X_VALS);
	axisY->setTitleText("Amplitude");

	chart->addAxis(axisX, Qt::AlignBottom);
	chart->addAxis(axisY, Qt::AlignLeft);
	chart->addSeries(frequencies);
	frequencies->attachAxis(axisX);
	frequencies->attachAxis(axisY);

	chart->legend()->hide();
	chart->setTitle("Input data");

	frequencies->setUseOpenGL(true);

	for (auto i = 0; i < X_VALS; i++)
		freqBuffer.append({float(i), 0});
	frequencies->replace(freqBuffer);
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
	if (audioOutput->bufferSize() > SampleRate * sizeof(float))
		audioOutput->setBufferSize(SampleRate * sizeof(float));
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

void MainWindow::showData(const char *data, int len) {
	if (!chartReady)
		return;
	chartReady = false;
	//Q_ASSERT(len == SampleRate);
	memcpy(sinToShow, data, len);


	float *floatData = reinterpret_cast<float *>(sinToShow);
	for (unsigned long i = 0; i < len / sizeof(float); i++) {
		amplitudesBuffer[bufferIndex++ % X_VALS].setY(floatData[i]);
		if (0 == bufferIndex % X_VALS)
			amplitudes->replace(amplitudesBuffer);
	}
	std::call_once(freqFlag, &MainWindow::showSpectrum, this, sinToShow, len);

	chartReady = true;
}

void MainWindow::showSpectrum(const char *data, int len) {
	Q_ASSERT(len * sizeof(float) >= X_VALS);
	memcpy(specToShow, data, len);
	const float *floatData = reinterpret_cast<const float *>(data);
}
