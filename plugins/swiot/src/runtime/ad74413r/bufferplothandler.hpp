#ifndef SWIOTPLOTHANDLER_HPP
#define SWIOTPLOTHANDLER_HPP

#include "src/captureplot/oscilloscope_plot.hpp"
#include<qobject.h>
#include "bufferlogic.hpp"
#include <qmutex.h>
#include <QWidget>

namespace adiscope::swiot {
class BufferPlotHandler : public QWidget
{
	Q_OBJECT
public:
	explicit BufferPlotHandler(QWidget *parent = nullptr, int plotChnlsNo = 0, int sampleRate = 0);
	~BufferPlotHandler();

	QColor getCurveColor(int id) const;

	void setPlotActiveAxis(int id);
	void deleteResidualPlotData();
	void resetPlotParameters();
	QWidget *getPlotWidget() const;

public Q_SLOTS:
	void onPlotChnlsChanges(std::vector<bool> m_enabledPlots);
	void onBufferRefilled(QVector<QVector<double>>, int bufferCounter);
	void onBtnExportClicked(QMap<int, bool> exportConfig);
	void onTimespanChanged(double value);
	void onSamplingFreqWritten(int samplingFreq);
private:
	void initPlot(int plotChnlsNo);
	void resetDataPoints();

	CapturePlot *m_plot;
	QWidget *m_plotWidget;

	int m_samplingFreq = 4800;
	double m_timespan = 1;
	int m_plotChnlsNo;

	//all of these will be calculated in functions; for example we will have a spinbox for timespan,
	//and in a slot we will set its value and we will calculate plotSampleRate and the number of necessary buffers
	int m_plotSampleNumber = m_samplingFreq * m_timespan;
	int m_buffersNumber = m_plotSampleNumber / MAX_BUFFER_SIZE;
	int m_bufferIndex = 0;
	int m_bufferSize = 0;
	int m_plotDataIndex = 0;

	std::vector<double*> m_dataPoints;
	std::vector<bool> m_enabledPlots;
	QMutex *m_lock;


};
}

#endif // SWIOTPLOTHANDLER_HPP
