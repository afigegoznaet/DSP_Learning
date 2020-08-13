#include "SoundFile.hpp"
#include <QtMath>
#include <QDebug>
#include <random>
SoundFile::SoundFile(QObject *parent) : QFile(parent) {}

bool SoundFile::open(OpenMode flags) {
	setOpenMode(flags);
	internalBuffer.open(flags);
	return true;
}

void SoundFile::close() {
	setOpenMode(QIODevice::NotOpen);
	internalBuffer.close();
}

qint64 SoundFile::readData(char *data, qint64 len) {
	if (!internalBuffer.bytesAvailable()) {
		internalBuffer.seek(0);
	}
	//qDebug() << internalBuffer.size();
	auto bytesRead = internalBuffer.read(data, len);
	emit dataRead(data, bytesRead);
	return bytesRead;
}

void SoundFile::setSineFrequency(const int hertz) {

	QByteArray arr;
	float	   sinusoid[SampleRate];

	/*
	// to be reimplemented
	std::random_device			   dev;
	std::mt19937				   generator(dev());
	std::uniform_real_distribution distribution(-1., 1.);


	for (int i = 0; i < SampleRate; i++) {
		sinusoid[i] = distribution(generator);
	}
*/
	for (int i = 0; i < SampleRate; i++) {
		sinusoid[i] = sin(hertz * i * 2 * M_PI / SampleRate);
	}
	arr.append(reinterpret_cast<char *>(sinusoid), SampleRate * sizeof(float));
	internalBuffer.setData(arr);
}
