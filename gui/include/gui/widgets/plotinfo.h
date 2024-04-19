#ifndef PLOTINFO_H
#define PLOTINFO_H

#include "plotwidget.h"

#include <QLabel>
#include <QWidget>

#include <plot_utils.hpp>
#include <scopy-gui_export.h>
#include <stylehelper.h>

namespace scopy {

class SCOPY_GUI_EXPORT TimePlotHDivInfo : public QLabel
{
	Q_OBJECT
public:
	TimePlotHDivInfo(QWidget *parent = nullptr);
	virtual ~TimePlotHDivInfo();

public Q_SLOTS:
	void update(double val, bool zoomed = false);

private:
	MetricPrefixFormatter *m_mpf;
};

class SCOPY_GUI_EXPORT TimePlotSamplingInfo : public QLabel
{
	Q_OBJECT
public:
	TimePlotSamplingInfo(QWidget *parent = nullptr);
	virtual ~TimePlotSamplingInfo();

public Q_SLOTS:
	void update(int ps, int bs, double sr);

private:
	MetricPrefixFormatter *m_mpf;
};

class SCOPY_GUI_EXPORT TimePlotFPS : public QLabel
{
	Q_OBJECT
public:
	TimePlotFPS(QWidget *parent = nullptr);
	virtual ~TimePlotFPS();

public Q_SLOTS:
	void update(qint64 timestamp);

private:
	QList<qint64> *m_replotTimes;
	qint64 m_lastTimeStamp;
	int m_avgSize;
};
class SCOPY_GUI_EXPORT TimePlotStatusInfo : public QLabel
{
	Q_OBJECT
public:
	TimePlotStatusInfo(QWidget *parent = nullptr);
	virtual ~TimePlotStatusInfo();
};

class SCOPY_GUI_EXPORT TimePlotVDivInfo : public QWidget
{};

class SCOPY_GUI_EXPORT TimePlotInfo : public QWidget
{
	Q_OBJECT
public:
	TimePlotInfo(PlotWidget *plot, QWidget *parent = nullptr);
	virtual ~TimePlotInfo();

public Q_SLOTS:
	void update(PlotSamplingInfo info);

private:
	PlotWidget *m_plot;
	TimePlotHDivInfo *m_hdiv;
	TimePlotSamplingInfo *m_sampling;
	TimePlotStatusInfo *m_status;
	TimePlotFPS *m_fps;
};

} // namespace scopy

#endif // PLOTINFO_H
