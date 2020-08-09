#include "SoundFile.hpp"

SoundFile::SoundFile(QObject *parent) : QFile(parent) {}

qint64 SoundFile::readData(char *data, qint64 len) {}

void SoundFile::setFrequency(int hertz) {}
