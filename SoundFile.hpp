#ifndef SOUNDFILE_HPP
#define SOUNDFILE_HPP
#include <QFile>
#include <QBuffer>
constexpr auto X_SAMPLES = 1024;
constexpr auto SampleRate = 48000;
constexpr auto BitDepth = 32;
class SoundFile : public QFile {
	Q_OBJECT
public:
	explicit SoundFile(QObject *parent = nullptr);
	bool open(OpenMode flags) override;
	void setSampleRate(int rate) { samplingRate = rate; }
	void setBitDepth(int bitsPerSample) { bytesPerSample = bitsPerSample / 8; }
	void setSineFrequency(int hertz);
	qint64 readData(char *data, qint64 len) override;
signals:
	void dataRead(const char *data, int len);

private:
	int		bytesPerSample = 4;
	int		samplingRate = 48000;
	QBuffer internalBuffer;
};

#endif // SOUNDFILE_HPP
