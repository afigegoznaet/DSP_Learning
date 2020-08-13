#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QBuffer>
#include <mutex>

namespace QtCharts {
	class QChart;
	class QLineSeries;
}

class QBarSet;
class QAudioOutput;
class SoundFile;

QT_BEGIN_NAMESPACE
namespace Ui {
	class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
	Q_OBJECT
private:
	void setupAmplitudeChart();
	void setupFrequencyChart();
	void setupAudio();
	int	 getFreqPos(int freq);
signals:
	void startShowSpectrum(const char *data, int len);

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();

public slots:
	void showData(const char *data, int len);
	void showSpectrum(const char *data, int len);

private slots:
	void on_startStopButton_toggled(bool checked);
	void showInfo();

private:
	Ui::MainWindow *	   ui;
	QtCharts::QLineSeries *amplitudes;
	QtCharts::QLineSeries *frequencies;
	QAudioOutput *		   audioOutput;
	SoundFile *			   audioFile;


	QVector<QPointF>  ampBuf0;
	QVector<QPointF>  ampBuf1;
	QVector<QPointF> *amplitudesBuffer;
	QVector<QPointF>  freqBuffer;
	std::atomic_bool  chartReady{true};
	unsigned int	  bufferIndex = 0;

	std::atomic_bool freqShown{true};
};
#endif // MAINWINDOW_HPP
