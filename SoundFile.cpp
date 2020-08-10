#include "SoundFile.hpp"
#include <QtMath>
#include <QDebug>
SoundFile::SoundFile(QObject *parent) : QFile(parent) {}

bool SoundFile::open(OpenMode flags) {
	setOpenMode(flags);
	internalBuffer.open(flags);
	return true;
}

qint64 SoundFile::readData(char *data, qint64 len) {
	if (!internalBuffer.bytesAvailable()) {
		internalBuffer.seek(0);
	}
	auto bytesRead = internalBuffer.read(data, len);
	emit dataRead(
		&internalBuffer.data().data()[internalBuffer.pos() - bytesRead],
		bytesRead);
	return bytesRead;
}

void SoundFile::setSineFrequency(int hertz) {
	// to be reimplemented

	QByteArray arr;
	float	   sinusoid[1];

	arr.append(reinterpret_cast<char *>(sinusoid), sizeof(float));
	internalBuffer.setData(arr);
}
