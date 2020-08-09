#ifndef SOUNDFILE_HPP
#define SOUNDFILE_HPP
#include <QFile>
#include <QBuffer>

class SoundFile : public QFile {
	Q_OBJECT
public:
	explicit SoundFile(QObject *parent = nullptr);
	bool open(OpenMode flags) override;
	void setSampleRate(int freq) { samplingFrequency = freq; }
	void setBitDepth(int bitsPerSample) { bytesPerSample = bitsPerSample / 8; }
	void setFrequency(int hertz);
	qint64 readData(char *data, qint64 len) override;
signals:
	void dataRead(char *data, int len);

private:
	int		bytesPerSample = 2;
	int		samplingFrequency = 48000;
	QBuffer internalBuffer;
};

#endif // SOUNDFILE_HPP
