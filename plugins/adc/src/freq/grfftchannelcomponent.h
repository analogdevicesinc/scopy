#ifndef GRFFTCHANNELCOMPONENT_H
#define GRFFTCHANNELCOMPONENT_H

#include "grfftsinkcomponent.h"
#include "iioutil/iiounits.h"
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
#include <gr-util/grfftfloatproxy.h>
#include <gr-util/griiocomplexchannelsrc.h>

namespace scopy {
namespace adc {

using namespace scopy::gui;

class GRDeviceAddon;


class GRFFTComplexChannelSigpath : public QObject, public GRChannel
{
public:
	GRFFTComplexChannelSigpath(QString m_name, ChannelComponent *ch, GRTopBlock *top,GRIIOComplexChannelSrc *src, QObject *parent)
		: QObject(parent)
	{
		m_ch = ch;
		m_grch = src;
		m_signalPath = new GRSignalPath(
			m_name + m_grch->getDeviceSrc()->deviceName() + m_grch->getChannelName(), this);
		m_signalPath->append(m_grch);
		m_fft = new GRFFTComplexProc(m_signalPath);
		m_signalPath->append(m_fft);
		m_signalPath->setEnabled(false);
		m_top = top;
		m_top->registerSignalPath(m_signalPath);
	}
	~GRFFTComplexChannelSigpath() {
		m_top->unregisterSignalPath(m_signalPath);
	}

	void onNewData(const float *xData, const float *yData, size_t size, bool copy) override
	{
		m_ch->chData()->onNewData(xData, yData, size, copy);
	}

	GRSignalPath* sigpath() override {
		return m_signalPath;
	}

	GRTopBlock *m_top;
	ChannelComponent *m_ch;
	GRSignalPath *m_signalPath;
	GRFFTComplexProc *m_fft;
	GRIIOComplexChannelSrc *m_grch;
};

class GRFFTChannelSigpath : public QObject, public GRChannel
{
public:
	GRFFTChannelSigpath(QString m_name, ChannelComponent *ch, GRTopBlock *top,GRIIOFloatChannelSrc *src, QObject *parent)
		: QObject(parent)
	{
		m_ch = ch;
		m_grch = src;
		m_signalPath = new GRSignalPath(
			m_name + m_grch->getDeviceSrc()->deviceName() + m_grch->getChannelName(), this);
		m_signalPath->append(m_grch);
		m_fft = new GRFFTFloatProc(m_signalPath);
		m_signalPath->append(m_fft);
		m_signalPath->setEnabled(false);
		m_top = top;
		m_top->registerSignalPath(m_signalPath);
	}
	~GRFFTChannelSigpath() {
		m_top->unregisterSignalPath(m_signalPath);
	}

	void onNewData(const float *xData, const float *yData, size_t size, bool copy) override
	{
		m_ch->chData()->onNewData(xData, yData, size, copy);
	}

	GRSignalPath* sigpath() override {
		return m_signalPath;
	}


	GRTopBlock *m_top;
	ChannelComponent *m_ch;
	GRSignalPath *m_signalPath;
	GRFFTFloatProc *m_fft;
	GRIIOFloatChannelSrc *m_grch;
};

class SCOPY_ADC_EXPORT GRFFTChannelComponent : public ChannelComponent,
					       public GRChannel,
					       public MeasurementProvider,
					       public SampleRateProvider,
					       public FftInstrumentComponent
{
	Q_OBJECT
public:
	GRFFTChannelComponent(GRIIOFloatChannelNode *node_I, GRIIOFloatChannelNode *node_Q, FFTPlotComponent *m_plot,
						     GRFFTSinkComponent *grtsc, QPen pen, QWidget *parent = nullptr);
	GRFFTChannelComponent(GRIIOFloatChannelNode *node, FFTPlotComponent *m_plot, GRFFTSinkComponent *grtsc,
			       QPen pen, QWidget *parent = nullptr);
	~GRFFTChannelComponent();

	MeasureManagerInterface *getMeasureManager() override;

	GRSignalPath *sigpath() override;
	QVBoxLayout *menuLayout();

	uint32_t fftSize() const;
	void setFftSize(uint32_t newFftSize);

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

	void setYModeHelper(YMode mode);

	void addChannelToPlot() override;
	void removeChannelFromPlot() override;

	bool complexMode() override;
	void setComplexMode(bool b) override;

Q_SIGNALS:
	void yModeChanged();
	void fftSizeChanged();

private:
	GRIIOFloatChannelNode *m_node;
	GRIIOChannel *m_src;

	GRIIOFloatChannelSrc *m_src_I;
	GRIIOFloatChannelSrc *m_src_Q;


	GRChannel *m_grtch;
	QVBoxLayout *m_layScroll;

	//FFTMeasureManager *m_measureMgr;
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
	QWidget *createChAttrMenu(iio_channel *ch, QWidget *parent);
	QWidget *createYAxisMenu(QWidget *parent);
	QWidget *createCurveMenu(QWidget *parent);
	QPushButton *createSnapshotButton(QWidget *parent);

	Q_PROPERTY(uint32_t fftSize READ fftSize WRITE setFftSize NOTIFY fftSizeChanged);

	YMode m_ymode;
	uint32_t m_fftSize;
	void _init();
};




} // namespace adc
} // namespace scopy
#endif // GRFFTCHANNELCOMPONENT_H
