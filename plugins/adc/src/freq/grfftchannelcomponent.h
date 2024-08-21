#ifndef GRFFTCHANNELCOMPONENT_H
#define GRFFTCHANNELCOMPONENT_H

#include "grfftsinkcomponent.h"
#include "scopy-adc_export.h"
#include "channelcomponent.h"
#include <gui/plotautoscaler.h>
#include <gui/widgets/menuonoffswitch.h>
#include <gui/widgets/menucombo.h>
#include <gui/widgets/menuplotchannelcurvestylecontrol.h>
#include <gui/widgets/menuplotaxisrangecontrol.h>
#include <gui/widgets/menucontrolbutton.h>
#include "interfaces.h"
#include <iio-widgets/iiowidget.h>
#include <gui/widgets/menuwidget.h>
#include "freq/fftplotcomponentchannel.h"
#include "freq/fftplotcomponent.h"
#include <gr-util/grsignalpath.h>
#include <gr-util/grfftfloatproxy.h>
#include <gr-util/griiocomplexchannelsrc.h>
#include "markercontroller.h"

namespace scopy {
namespace adc {

using namespace scopy::gui;

class GRDeviceAddon;

class GRFFTComplexChannelSigpath : public QObject, public GRChannel, public FFTChannel
{
public:
	GRFFTComplexChannelSigpath(QString m_name, ChannelComponent *ch, GRTopBlock *top, GRIIOComplexChannelSrc *src,
				   QObject *parent)
		: QObject(parent)
	{
		m_ch = ch;
		m_grch = src;
		m_signalPath = new GRSignalPath(
			m_name + m_grch->getDeviceSrc()->deviceName() + m_grch->getChannelName(), this);
		m_signalPath->append(m_grch);
		m_fft = new GRFFTComplexProc(m_signalPath);
		int nrBits = src->getFmt()->bits - src->getFmt()->is_signed;
		m_fft->setNrBits(nrBits);
		m_signalPath->append(m_fft);
		m_signalPath->setEnabled(false);
		m_top = top;
		m_top->registerSignalPath(m_signalPath);
	}
	~GRFFTComplexChannelSigpath() { m_top->unregisterSignalPath(m_signalPath); }

	void onNewData(const float *xData, const float *yData, size_t size, bool copy) override
	{
		m_ch->chData()->onNewData(xData, yData, size, copy);
	}

	GRSignalPath *sigpath() override { return m_signalPath; }

	void setPowerOffset(double val) override
	{
		m_powerOffset = val;
		m_fft->setPowerOffset(val);
	}

	double powerOffset() { return m_powerOffset; }

	void setWindow(int w) override {
		m_fft->setWindow(static_cast<gr::fft::window::win_type>(w));
	}

	void setWindowCorrection(bool b) override {
		m_fft->setWindowCorrection(b);
	}

	GRTopBlock *m_top;
	ChannelComponent *m_ch;
	GRSignalPath *m_signalPath;
	GRFFTComplexProc *m_fft;
	GRIIOComplexChannelSrc *m_grch;
	double m_powerOffset;
};

class GRFFTChannelSigpath : public QObject, public GRChannel, public FFTChannel
{
public:
	GRFFTChannelSigpath(QString m_name, ChannelComponent *ch, GRTopBlock *top, GRIIOFloatChannelSrc *src,
			    QObject *parent)
		: QObject(parent)
	{
		m_powerOffset = 0;
		m_ch = ch;
		m_grch = src;
		m_signalPath = new GRSignalPath(
			m_name + m_grch->getDeviceSrc()->deviceName() + m_grch->getChannelName(), this);
		m_signalPath->append(m_grch);
		m_fft = new GRFFTFloatProc(m_signalPath);
		int nrBits = src->getFmt()->bits - src->getFmt()->is_signed;
		m_fft->setNrBits(nrBits);
		m_signalPath->append(m_fft);
		m_signalPath->setEnabled(false);
		m_top = top;
		m_top->registerSignalPath(m_signalPath);
	}
	~GRFFTChannelSigpath() { m_top->unregisterSignalPath(m_signalPath); }

	void onNewData(const float *xData, const float *yData, size_t size, bool copy) override
	{
		m_ch->chData()->onNewData(xData, yData, size, copy);
	}

	GRSignalPath *sigpath() override { return m_signalPath; }

	void setPowerOffset(double val) override
	{
		m_powerOffset = val;
		m_fft->setPowerOffset(val);
	}

	double powerOffset() { return m_powerOffset; }

	void setWindow(int w) override {
		m_fft->setWindow(static_cast<gr::fft::window::win_type>(w));
	}

	void setWindowCorrection(bool b) override {
		m_fft->setWindowCorrection(b);
	}

	GRTopBlock *m_top;
	ChannelComponent *m_ch;
	GRSignalPath *m_signalPath;
	GRFFTFloatProc *m_fft;
	GRIIOFloatChannelSrc *m_grch;
	double m_powerOffset;
};

class SCOPY_ADC_EXPORT GRFFTChannelComponent : public ChannelComponent,
					       public GRChannel,
					       public MeasurementProvider,
					       public SampleRateProvider,
					       public FFTChannel
{
	Q_OBJECT
public:
	GRFFTChannelComponent(GRIIOFloatChannelNode *node_I, GRIIOFloatChannelNode *node_Q, FFTPlotComponent *m_plot,
			      GRFFTSinkComponent *grtsc, QPen pen, QWidget *parent = nullptr);
	GRFFTChannelComponent(GRIIOFloatChannelNode *node, FFTPlotComponent *m_plot, GRFFTSinkComponent *grtsc,
			      QPen pen, QWidget *parent = nullptr);
	~GRFFTChannelComponent();

	MeasureManagerInterface *getMeasureManager() override;
	MarkerController* markerController();

	GRSignalPath *sigpath() override;
	QVBoxLayout *menuLayout();

	double powerOffset();
	void setPowerOffset(double) override;
	void setWindow(int) override;
	void setSamplingInfo(SamplingInfo p) override;
	int window() const;
	bool windowCorrection() const;
	void setWindowCorrection(bool newWindowCorr) override;
	virtual bool enabled() const override;

public Q_SLOTS:
	void enable() override;
	void disable() override;
	void onStart() override;
	void onStop() override;
	void onInit() override;
	void onDeinit() override;

	void onNewData(const float *xData, const float *yData, size_t size, bool copy) override;

	bool sampleRateAvailable() override;
	double sampleRate() override;

	void addChannelToPlot() override;
	void removeChannelFromPlot() override;


Q_SIGNALS:
	void yModeChanged();
	void fftSizeChanged();
	void powerOffsetChanged(double);
	void windowChanged(int);
	void windowCorrectionChanged(bool);

private:
	double m_powerOffset;
	int m_window;
	bool m_windowCorrection;

	GRIIOFloatChannelNode *m_node;
	GRIIOChannel *m_src;

	GRIIOFloatChannelSrc *m_src_I;
	GRIIOFloatChannelSrc *m_src_Q;

	GRChannel *m_grtch;
	QVBoxLayout *m_layScroll;

	// FFTMeasureManager *m_measureMgr;
	MenuPlotAxisRangeControl *m_yCtrl;
	MenuCombo *m_ymodeCb;
	IIOWidget *m_scaleWidget;

	MenuPlotChannelCurveStyleControl *m_curvemenu;
	MenuSectionCollapseWidget *m_yaxisMenu;
	FFTPlotComponentChannel *m_fftPlotComponentChannel;

	QPushButton *m_snapBtn;

	bool m_running;
	bool m_scaleAvailable;
	bool m_complex;

	QWidget *createMenu(QWidget *parent = nullptr);
	QWidget *createChAttrMenu(iio_channel *ch, QString title, QWidget *parent);
	QWidget *createMarkerMenu(QWidget *parent);
	QWidget *createYAxisMenu(QWidget *parent);
	QWidget *createCurveMenu(QWidget *parent);
	QPushButton *createSnapshotButton(QWidget *parent);

	YMode m_ymode;
	void _init();

	Q_PROPERTY(double powerOffset READ powerOffset WRITE setPowerOffset NOTIFY powerOffsetChanged)
	Q_PROPERTY(int window READ window WRITE setWindow NOTIFY windowChanged)
	Q_PROPERTY(bool windowCorrection READ windowCorrection WRITE setWindowCorrection NOTIFY windowCorrectionChanged)
};

} // namespace adc
} // namespace scopy
#endif // GRFFTCHANNELCOMPONENT_H
