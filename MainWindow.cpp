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
constexpr auto BitDepth = 16;
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
	audioFile->setFrequency(1);
}

MainWindow::~MainWindow() { delete ui; }


void MainWindow::setupAmplitudeChart() {
	auto m_chart = ui->signalView->chart();

	// m_amplitudes->setUseOpenGL(true);
	// m_chart->addSeries(m_amplitudes);
	QValueAxis *axisX = new QValueAxis;
	QValueAxis *axisY = new QValueAxis;

	axisX->setRange(0, X_SAMPLES);
	axisX->setLabelFormat("%g");
	axisX->setTitleText("Data Points");

	axisY->setRange(-1, 1);
	axisY->setTitleText("Audio level");


	m_chart->addAxis(axisX, Qt::AlignBottom);
	m_chart->addAxis(axisY, Qt::AlignLeft);
	m_chart->addSeries(amplitudes);
	amplitudes->attachAxis(axisX);
	amplitudes->attachAxis(axisY);

	m_chart->legend()->hide();
	m_chart->setTitle("Input data");

	amplitudes->setUseOpenGL(true);
}


void MainWindow::setupAudio() {

	const auto deviceInfo = QAudioDeviceInfo::defaultOutputDevice();

	QAudioFormat format;

	format.setSampleRate(SampleRate);
	format.setChannelCount(1);
	format.setSampleSize(BitDepth);
	format.setCodec("audio/pcm");
	format.setByteOrder(QAudioFormat::LittleEndian);
	format.setSampleType(QAudioFormat::SignedInt);

	if (!deviceInfo.isFormatSupported(format)) {
		qWarning() << "Default format not supported - trying to use nearest";
		format = deviceInfo.nearestFormat(format);
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


void MainWindow::showData(char *data, qint64 len) {
	if (!deviceReady)
		return;
	deviceReady = false;
	if (m_buffer.isEmpty()) {
		m_buffer.reserve(maxSamples);
		for (int i = 0; i < maxSamples; ++i)
			m_buffer.append(QPointF(i, 0));
	}

	int resolution = internalBuffer.size() / maxSamples;
	resolution /= bytesPerSample;
	long long divisor = 0;
	switch (bytesPerSample) {
	default:
		divisor = CHAR_MAX;
		break;
	case 2:
		divisor = SHRT_MAX;
		break;
	case 3:
		divisor |= 0x7fffff;
		break;
	case 4:
		divisor = LLONG_MAX;
		break;
	}

	// std::vector<short> data[transferredBytes/2];
	const auto *data =
		reinterpret_cast<const short *>(internalBuffer.data().data());
	for (int i = marker; i < transferredBytes / bytesPerSample;
		 i += resolution) {
		m_buffer[i / resolution].setY(((double)data[i]) / divisor);
		marker++;
	}
	if (marker >= maxSamples)
		marker = 0;
	m_series->replace(m_buffer);
	deviceReady = true;
}
