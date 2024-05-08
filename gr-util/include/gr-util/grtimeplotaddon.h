#ifndef GRTIMEPLOTADDON_H
#define GRTIMEPLOTADDON_H

#include "plotinfo.h"
#include "scopy-gr-util_export.h"
#include "time_sink_f.h"
#include "timechanneladdon.h"
#include "tooladdon.h"

#include <QFuture>
#include <QGridLayout>
#include <QTimer>
#include <QVBoxLayout>
#include <QtConcurrent>

#include <gui/oscilloscope_plot.hpp>
#include <gui/widgets/plotbufferpreviewer.h>
#include <plotwidget.h>
#include <gnuradio/blocks/stream_to_vector.h>
#include <gnuradio/fft/fft_v.h>
#include <gnuradio/fft/window.h>
#include <gnuradio/blocks/float_to_complex.h>
#include <gnuradio/blocks/vector_sink.h>
#include <gnuradio/blocks/complex_to_mag_squared.h>
#include <gnuradio/blocks/nlog10_ff.h>
#include <gnuradio/blocks/multiply_const.h>

namespace scopy {
class TimePlotHDivInfo;
class TimePlotSamplingInfo;
namespace grutil {
class GRTopBlock;
class GRTimeChannelAddon;
class GRTimePlotAddonSettings;
class ChannelAddon;

class SCOPY_GR_UTIL_EXPORT PlotAddon
{
public:
	virtual PlotWidget *plot() = 0;
	virtual void replot() = 0;
	virtual double sampleRate() = 0;
};

class SCOPY_GR_UTIL_EXPORT GRTimePlotAddon : public QObject,
					     public ToolAddon,
					     public ChannelConfigAware,
					     public PlotAddon,
					     public GRTopAddon
{
	Q_OBJECT
public:
	GRTimePlotAddon(QString name, GRTopBlock *top, QObject *parent = nullptr);
	virtual ~GRTimePlotAddon();

	QString getName() override;
	QWidget *getWidget() override;
	PlotWidget *plot() override;
	PlotWidget *fftplot();
	PlotChannel *fftplotch();

	PlotWidget *xyplot();
	PlotChannel *xyplotch();
	double sampleRate() override;
	double freqOffset();
	int xMode();

	QList<GRTimeChannelAddon *> getGrChannels() const;

Q_SIGNALS:
	void requestRebuild();
	void requestStop();
	void newData();
	void xAxisUpdated();
	void statusChanged(QString status);

public Q_SLOTS:
	void enable() override;
	void disable() override;
	void onStart() override;
	void onStop() override;
	void onInit() override;
	void onDeinit() override;
	void preFlowStart() override;
	void postFlowStart() override;
	void preFlowStop() override;
	void postFlowStop() override;

	void onChannelAdded(ChannelAddon *t) override;
	void onChannelRemoved(ChannelAddon *t) override;

	void replot() override;
	void connectSignalPaths();
	void tearDownSignalPaths();
	void onNewData();
	void updateBufferPreviewer();

	void setRollingMode(bool b);
	void setDrawPlotTags(bool b);
	void setSampleRate(double);
	void setFreqOffset(double);

	void setBufferSize(uint32_t size);
	void setComplexMode(bool b);
	void setPlotSize(uint32_t size);
	void handlePreferences(QString, QVariant);
	void setSingleShot(bool);
	void setFrameRate(double);
	void setXMode(int mode);

	void setXYSource(GRTimeChannelAddon *, GRTimeChannelAddon *);
	void setFFTSource(GRTimeChannelAddon *, GRTimeChannelAddon *);
	void setFftWindow(int idx);

private Q_SLOTS:
	void stopPlotRefresh();
	void startPlotRefresh();
	void drawPlot();

private:
	QTimer *m_plotTimer;
	GRTopBlock *m_top;
	QString name;
	QWidget *widget;

	PlotWidget *m_plotWidget;
	PlotWidget *m_fftPlotWidget;
	PlotWidget *m_xyPlotWidget;

	PlotChannel *m_xy_channel;
	PlotChannel *m_fft_channel;

	PlotAxis *xy_xPlotAxis;
	PlotAxis *xy_yPlotAxis;

	PlotAxis *fft_xPlotAxis;
	PlotAxis *fft_yPlotAxis;
	QMetaObject::Connection xy_min_max_connections[4];

	GRTimeChannelAddon *m_xy_source[2];

	GRTimeChannelAddon *m_fft_source[2];

	PlotBufferPreviewer *m_bufferPreviewer;
	time_sink_f::sptr time_sink;
	//	fft_sink_f::sptr fft_sink;
	gr::blocks::stream_to_vector::sptr s2v, s2v_complex;
	gr::fft::fft_v<float, true>::sptr fft;
	gr::fft::fft_v<gr_complex, true>::sptr fft_complex;
	gr::blocks::vector_sink_f::sptr vector_sink;
	gr::blocks::vector_sink_c::sptr vector_complex_sink;
	gr::blocks::float_to_complex::sptr f2c;
	gr::blocks::complex_to_mag_squared::sptr ctm;
	gr::blocks::multiply_const_ff::sptr mult_const1, mult_const2;
	gr::blocks::nlog10_ff::sptr nlog10;

	QList<GRTimeChannelAddon *> grChannels;
	QVBoxLayout *m_lay;
	void setupBufferPreviewer();
	void setupPlotInfo();

	QFuture<void> refillFuture;
	QFutureWatcher<void> *fw;
	QMetaObject::Connection futureWatcherConn;
	std::mutex refillMutex;

	PlotSamplingInfo m_currentSamplingInfo;
	bool m_started;
	bool m_rollingMode;
	bool m_singleShot;
	bool m_showPlotTags;
	bool m_refreshTimerRunning;
	bool fftComplexMode;
	int m_xmode;
	gr::fft::window::win_type m_fftwindow;

	QMap<QString, int> time_channel_map;

	void setRawSamplesPtr();
	void updateXAxis();
	void updateFrameRate();
	void drawTags();
};
} // namespace grutil
} // namespace scopy

#endif // GRTIMEPLOTADDON_H
