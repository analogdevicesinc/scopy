#ifndef PLOTINFOWIDGETS_H
#define PLOTINFOWIDGETS_H

#include <QLabel>
#include <QWidget>
#include <plot_utils.hpp>
#include <plotwidget.h>
#include <scopy-gui_export.h>

namespace scopy {

class SCOPY_GUI_EXPORT HDivInfo : public QLabel
{
	Q_OBJECT
public:
	HDivInfo(PlotWidget *plot, QWidget *parent = nullptr);
	virtual ~HDivInfo();

public Q_SLOTS:
	void update(double val, bool zoomed = false);
	void onRectChanged();

private:
	MetricPrefixFormatter *m_mpf;
	PlotWidget *m_plot;
};

class SCOPY_GUI_EXPORT TimeSamplingInfo : public QLabel
{
	Q_OBJECT
public:
	TimeSamplingInfo(QWidget *parent = nullptr);
	virtual ~TimeSamplingInfo();

public Q_SLOTS:
	void update(SamplingInfo info);

private:
	MetricPrefixFormatter *m_mpf;
};

class SCOPY_GUI_EXPORT FPSInfo : public QLabel
{
	Q_OBJECT
public:
	FPSInfo(PlotWidget *plot, QWidget *parent = nullptr);
	virtual ~FPSInfo();

public Q_SLOTS:
	void update(qint64 timestamp);

private:
	PlotWidget *m_plot;
	QList<qint64> *m_replotTimes;
	qint64 m_lastTimeStamp;
	int m_avgSize;
};

class SCOPY_GUI_EXPORT TimestampInfo : public QLabel
{
	Q_OBJECT
public:
	TimestampInfo(PlotWidget *plot, QWidget *parent = nullptr);
	virtual ~TimestampInfo();

private:
	PlotWidget *m_plot;
};

} // namespace scopy

#endif // PLOTINFOWIDGETS_H
