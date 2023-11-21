#ifndef DATAMONITORVIEW_H
#define DATAMONITORVIEW_H

#include "datamonitormodel.hpp"

#include <QWidget>
#include <QBoxLayout>
#include <QLabel>
#include <plotwidget.h>
#include <plotaxis.h>
#include <lcdNumber.hpp>

namespace scopy {
namespace datamonitor {

class DataMonitorView : public QFrame
{
	Q_OBJECT
public:
	explicit DataMonitorView(DataMonitorModel *dataMonitorModel, QWidget *parent = nullptr);

	PlotWidget *getPlot() const;
	QWidget *getHeader() const;
	QWidget *getValueWidget() const;
	QWidget *getMinValueWidget() const;
	QWidget *getMaxValueWidget() const;
	void updateValue(double value);
	void updateMinValue(double value);
	void updateMaxValue(double value);
	void updatePrecision(int precision);

	PlotChannel *getPlotch() const;

	double getSampleRate() const;
	void setSampleRate(double newSampleRate);

	unsigned int getNumSamples() const;

	double getInterval() const;
	void setInterval(double newInterval);

	void plotData(double xValue, double yValue);

	void togglePeakHolder(bool toggle);
	void togglePlot(bool toggle);
	void updateCurveStyle(Qt::PenStyle lineStyle);

signals:

private:
	QLabel *measuringUnit;
	QLabel *title;
	PlotWidget *m_plot;
	PlotChannel *plotch;
	QWidget *header;
	QWidget *valueWidget;
	QWidget *minValueWidget;
	QWidget *maxValueWidget;
	LcdNumber *value;
	LcdNumber *minValue;
	LcdNumber *maxValue;

	QVector<double> ydata;
	QVector<double> xdata;
	unsigned int m_numSamples;
	double m_sampleRate;
	double m_timeInterval;

	double plotMaxX;
	double plotMaxY;

	void plotData(double newData);
	void setNumSamples(unsigned int newNumSamples);
};
} // namespace datamonitor
} // namespace scopy
#endif // DATAMONITORVIEW_H
