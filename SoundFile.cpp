#include "SoundFile.hpp"

SoundFile::SoundFile(QObject *parent) : QFile(parent) {}

bool SoundFile::open(OpenMode flags) {
	setOpenMode(flags);
	internalBuffer.open(flags);
	return true;
}

qint64 SoundFile::readData(char *data, qint64 len) {
	auto bytesRead = internalBuffer.read(data, len);
	emit dataRead(data, bytesRead);
	if (!internalBuffer.bytesAvailable()) {
		internalBuffer.seek(0);
	}
	return bytesRead;
}

void SoundFile::setFrequency(int hertz) {
	// to be reimplemented
	QByteArray arr;
	arr.resize(1024 * sizeof(float));
	internalBuffer.setData(arr);
	internalBuffer.seek(0);
}
