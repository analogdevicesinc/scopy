#ifndef PLOTINFO_H
#define PLOTINFO_H

#include "buffer_previewer.hpp"
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

class SCOPY_GUI_EXPORT TimePlotStatusInfo : public QLabel
{
	Q_OBJECT
public:
	TimePlotStatusInfo(QWidget *parent = nullptr);
	virtual ~TimePlotStatusInfo();
};

class SCOPY_GUI_EXPORT TimePlotVDivInfo : public QWidget
{};

class SCOPY_GUI_EXPORT PlotBufferPreviewerController : public QWidget
{
	Q_OBJECT
public:
	explicit PlotBufferPreviewerController(PlotWidget *p, BufferPreviewer *b, QWidget *parent = nullptr);
	~PlotBufferPreviewerController();

	void updateDataLimits(double min, double max);
public Q_SLOTS:
	void updateBufferPreviewer();

private:
	double m_bufferPrevInitMin;
	double m_bufferPrevInitMax;

	double m_bufferPrevData;

	void setupBufferPreviewer();
	PlotWidget *m_plot;
	BufferPreviewer *m_bufferPreviewer;
};

class SCOPY_GUI_EXPORT TimePlotInfo : public QWidget
{
	Q_OBJECT
public:
	TimePlotInfo(PlotWidget *plot, QWidget *parent = nullptr);
	virtual ~TimePlotInfo();

public Q_SLOTS:
	void update(PlotSamplingInfo info);
	void updateBufferPreviewer();

private:
	PlotWidget *m_plot;
	TimePlotHDivInfo *m_hdiv;
	TimePlotSamplingInfo *m_sampling;
	TimePlotStatusInfo *m_status;
	PlotBufferPreviewerController *m_bufferController;
	AnalogBufferPreviewer *m_bufferPreviewer;
};

} // namespace scopy

#endif // PLOTINFO_H
