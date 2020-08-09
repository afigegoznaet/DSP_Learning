#ifndef SOUNDFILE_HPP
#define SOUNDFILE_HPP
#include <QFile>
#include <QBuffer>

class SoundFile : public QFile {
	Q_OBJECT
public:
	explicit SoundFile(QObject *parent = nullptr);
	using QFile::open;
	bool open(const QString &fileName);
	void setSamplerate(int freq) { inputFrequency = freq; }
	void setBitDepth(int bitsPerSample) { bytesPerSample = bitsPerSample / 8; }
	void setFrequency(int hertz);
	qint64 readData(char *data, qint64 len) override;

private:
	int		bytesPerSample = 2;
	int		inputFrequency = 48000;
	int		frequency;
	QBuffer internalBuffer;
};

#endif // SOUNDFILE_HPP
