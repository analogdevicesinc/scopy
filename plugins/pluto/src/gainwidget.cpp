#include "gainwidget.h"
#include <style.h>
#include <iioutil/connectionprovider.h>
#include <iiowidgetbuilder.h>

using namespace scopy;
using namespace pluto;

GainWidget::GainWidget(QString uri, QWidget *parent)
	: m_uri(uri)
	, QWidget{parent}
{
	m_layout = new QVBoxLayout(this);
	m_layout->setMargin(0);
	m_layout->setContentsMargins(0, 0, 0, 0);
	setLayout(m_layout);

	QWidget *gainWidget = new QWidget(parent);
	QVBoxLayout *layout = new QVBoxLayout(gainWidget);
	gainWidget->setLayout(layout);
	layout->setSpacing(10);
	layout->setMargin(0);

	QScrollArea *scrollArea = new QScrollArea(parent);
	scrollArea->setWidgetResizable(true);
	scrollArea->setWidget(gainWidget);

	// Get connection to device
	Connection *conn = ConnectionProvider::GetInstance()->open(m_uri);
	// iio:device0: ad9361-phy
	m_device = iio_context_find_device(conn->context(), "ad9361-phy");

	layout->addWidget(modeWidget(gainWidget));
	layout->addWidget(mgcWidget(gainWidget));
	layout->addWidget(agcThresholdGainChangesWidget(gainWidget));
	layout->addWidget(adcOverloadWidget(gainWidget));
	layout->addWidget(lmtOverloadWidget(gainWidget));
	layout->addWidget(digitalGainWidget(gainWidget));
	layout->addWidget(fastAttackAGCWidget(gainWidget));
	layout->addWidget(miscWidget(gainWidget));

	layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	m_layout->addWidget(scrollArea);
}

GainWidget::~GainWidget() { ConnectionProvider::close(m_uri); }

QWidget *GainWidget::modeWidget(QWidget *parent)
{
	QWidget *modeWidget = new QWidget(parent);
	QGridLayout *modeWidgetLayout = new QGridLayout(modeWidget);
	modeWidget->setLayout(modeWidgetLayout);

	Style::setStyle(modeWidget, style::properties::widget::border_interactive);
	Style::setBackgroundColor(modeWidget, json::theme::background_primary);

	// Get connection to device
	Connection *conn = ConnectionProvider::GetInstance()->open(m_uri);
	// iio:device0: ad9361-phy
	m_device = iio_context_find_device(conn->context(), "ad9361-phy");

	QLabel *title = new QLabel("Mode", modeWidget);
	Style::setStyle(title, style::properties::label::menuBig);
	modeWidgetLayout->addWidget(title, 0, 0);

	// adi,gc-rx1-mode
	IIOWidget *rx1Mode = IIOWidgetBuilder(modeWidget)
				     .device(m_device)
				     .attribute("adi,gc-rx1-mode")
				     // .uiStrategy(IIOWidgetBuilder::ComboUi)
				     .title("RX1")
				     .buildSingle();
	modeWidgetLayout->addWidget(rx1Mode, 1, 0);
	rx1Mode->setToolTip("RX1 Gain control operation: Manual gain (0); Fast attack AGC (1); Slow attack AGC (2); "
			    "Hybrid AGC (3). See register 0x0FA, bits [D4], [D1:D0].");

	// adi,gc-rx2-mode
	IIOWidget *rx2Mode = IIOWidgetBuilder(modeWidget)
				     .device(m_device)
				     .attribute("adi,gc-rx2-mode")
				     // .uiStrategy(IIOWidgetBuilder::ComboUi)
				     .title("RX2")
				     .buildSingle();
	modeWidgetLayout->addWidget(rx2Mode, 1, 1);
	rx2Mode->setToolTip("RX2 Gain control operation: Manual gain (0); Fast attack AGC (1); Slow attack AGC (2); "
			    "Hybrid AGC (3). See register 0x0FA, bits [D4], [D3:D2].");

	// adi,split-gain-table-mode-enable
	IIOWidget *tableMode = IIOWidgetBuilder(modeWidget)
				       .device(m_device)
				       .attribute("adi,split-gain-table-mode-enable")
				       // .uiStrategy(IIOWidgetBuilder::ComboUi)
				       .title("Table Mode")
				       .buildSingle();
	modeWidgetLayout->addWidget(tableMode, 1, 2);
	tableMode->setToolTip("Enable Split Gain Table Mode - default Full Table");

	// adi,gc-dec-pow-measurement-duration
	IIOWidget *decpowMeasurement = IIOWidgetBuilder(modeWidget)
					       .device(m_device)
					       .attribute("adi,gc-dec-pow-measurement-duration")
					       // .uiStrategy(IIOWidgetBuilder::ComboUi)
					       .title("Dec PWR mess. duration in MGC and Slow Attack AGC mode")
					       .buildSingle();
	modeWidgetLayout->addWidget(decpowMeasurement, 2, 0);
	decpowMeasurement->setToolTip("The power measurement duration used by the gain control algorithm. See register "
				      "0x15C, bits [D3:D0]. This data is processed by the driver.");

	// adi,gc-low-power-thresh
	IIOWidget *lowPowerThresh = IIOWidgetBuilder(modeWidget)
					    .device(m_device)
					    .attribute("adi,gc-low-power-thresh")
					    // .uiStrategy(IIOWidgetBuilder::ComboUi)
					    .title("Low PWR Thresh (dbFS)")
					    .buildSingle();
	modeWidgetLayout->addWidget(lowPowerThresh, 3, 0);
	lowPowerThresh->setToolTip("This threshold is used by the fast AGC to determine if the gain should be "
				   "increased. It can also be used to trigger a CTRL_OUT signal transition in MGC "
				   "mode. See register 0x114, bits [D6:D0]. This data is processed by the driver.");

	// adi,agc-attack-delay-extra-margin-us
	IIOWidget *attackDelay = IIOWidgetBuilder(modeWidget)
					 .device(m_device)
					 .attribute("adi,agc-attack-delay-extra-margin-us")
					 // .uiStrategy(IIOWidgetBuilder::ComboUi)
					 .title("Attack Delay extra margin (us)")
					 .buildSingle();
	modeWidgetLayout->addWidget(attackDelay, 2, 1);
	attackDelay->setToolTip("The AGC Attack Delay prevents the AGC from starting its algorithm until the receive "
				"path has settled. The delay counter starts when the ENSM enters the Rx state. See "
				"register 0x022, bits [D5:D0]. This data is processed by the driver.");

	// adi,gc-use-rx-fir-out-for-dec-pwr-meas-enable
	IIOWidget *rxFirOut = IIOWidgetBuilder(modeWidget)
				      .device(m_device)
				      .attribute("adi,gc-use-rx-fir-out-for-dec-pwr-meas-enable")
				      .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
				      .title("Use Rx FIR output for Dec. PWR measurements")
				      .buildSingle();
	modeWidgetLayout->addWidget(rxFirOut, 3, 1);
	rxFirOut->setToolTip("	Set to use the RX FIR output for power measurements. Default/Clear to use the HB1 "
			     "output. See register 0x15C, bits [D6].");

	connect(this, &GainWidget::readRequested, this, [=, this]() {
		rx1Mode->readAsync();
		rx2Mode->readAsync();
		tableMode->readAsync();
		decpowMeasurement->readAsync();
		lowPowerThresh->readAsync();
		attackDelay->readAsync();
		rxFirOut->readAsync();
	});

	return modeWidget;
}

QWidget *GainWidget::mgcWidget(QWidget *parent)
{
	QWidget *mgcWidget = new QWidget(parent);
	QGridLayout *mgcWidgetLayout = new QGridLayout(mgcWidget);
	mgcWidget->setLayout(mgcWidgetLayout);

	Style::setStyle(mgcWidget, style::properties::widget::border_interactive);
	Style::setBackgroundColor(mgcWidget, json::theme::background_primary);

	QLabel *title = new QLabel("MGC", mgcWidget);
	Style::setStyle(title, style::properties::label::menuBig);
	mgcWidgetLayout->addWidget(title, 0, 0);

	// adi,mgc-rx1-ctrl-inp-enable
	IIOWidget *mgcRx1Ctrl = IIOWidgetBuilder(mgcWidget)
					.device(m_device)
					.attribute("adi,mgc-rx1-ctrl-inp-enable")
					.uiStrategy(IIOWidgetBuilder::CheckBoxUi)
					.title("RX1 Control Input")
					.buildSingle();
	mgcWidgetLayout->addWidget(mgcRx1Ctrl, 1, 0);
	mgcRx1Ctrl->setToolTip("If this attribute is clear, SPI writes change the RX1 gain. When this attribute is "
			       "set, control input pins control the gain. See register 0x0FB, bit [D0].");

	// adi,mgc-rx2-ctrl-inp-enable
	IIOWidget *mgcRx2Ctrl = IIOWidgetBuilder(mgcWidget)
					.device(m_device)
					.attribute("adi,mgc-rx2-ctrl-inp-enable")
					.uiStrategy(IIOWidgetBuilder::CheckBoxUi)
					.title("RX2 Control Input")
					.buildSingle();
	mgcWidgetLayout->addWidget(mgcRx2Ctrl, 1, 1);
	mgcRx2Ctrl->setToolTip("If this attribute is clear, SPI writes change the RX2 gain. When this attribute is "
			       "set, control input pins control the gain. See register 0x0FB, bit [D1].");

	// adi,mgc-inc-gain-step
	IIOWidget *mgcIncGain = IIOWidgetBuilder(mgcWidget)
					.device(m_device)
					.attribute("adi,mgc-inc-gain-step")
					// //.uiStrategy(IIOWidgetBuilder::RangeUi)
					.title("Increase Gain Step")
					.buildSingle();
	mgcWidgetLayout->addWidget(mgcIncGain, 2, 0);
	mgcIncGain->setToolTip("This attribute applies if the CTRL_IN signals control gain. The gain index increases "
			       "by this value when certain CTRL_IN signals transition high. See register 0x0FC, bits "
			       "[D7:D5]. This data is processed by the driver.");

	// adi,mgc-dec-gain-step
	IIOWidget *mgcDecGain = IIOWidgetBuilder(mgcWidget)
					.device(m_device)
					.attribute("adi,mgc-inc-gain-step")
					.title("Decrease Gain Step")
					// //.uiStrategy(IIOWidgetBuilder::RangeUi)
					.buildSingle();
	mgcWidgetLayout->addWidget(mgcDecGain, 2, 1);
	mgcDecGain->setToolTip("This attribute applies if the CTRL_IN signals control gain. The gain index decreases "
			       "by this value when certain CTRL_IN signals transition high. See register 0x0FE, bits "
			       "[D7:D5]. This data is processed by the driver.");

	// adi,mgc-split-table-ctrl-inp-gain-mode
	IIOWidget *mgcSplitTableCtrl = IIOWidgetBuilder(mgcWidget)
					       .device(m_device)
					       .attribute("adi,mgc-inc-gain-step")
					       .title("Split Table Control Input Mode")
					       .buildSingle();
	mgcWidgetLayout->addWidget(mgcSplitTableCtrl, 3, 0);
	mgcSplitTableCtrl->setToolTip(
		"AGC determine this (0); Only in LPF(1); Only in LMT (2). See register 0x0FC, bits [D4], [D3].");

	connect(this, &GainWidget::readRequested, this, [=, this]() {
		mgcRx1Ctrl->readAsync();
		mgcRx2Ctrl->readAsync();
		mgcIncGain->readAsync();
		mgcDecGain->readAsync();
		mgcSplitTableCtrl->readAsync();
	});

	return mgcWidget;
}

QWidget *GainWidget::agcThresholdGainChangesWidget(QWidget *parent)
{
	QWidget *agcTresholdGainChangesWidget = new QWidget(parent);
	QGridLayout *agcTresholdGainChangesWidgetLayout = new QGridLayout(agcTresholdGainChangesWidget);
	agcTresholdGainChangesWidget->setLayout(agcTresholdGainChangesWidgetLayout);

	Style::setStyle(agcTresholdGainChangesWidget, style::properties::widget::border_interactive);
	Style::setBackgroundColor(agcTresholdGainChangesWidget, json::theme::background_primary);

	QLabel *title = new QLabel("AGC Thresholds and Gain Changes", agcTresholdGainChangesWidget);
	Style::setStyle(title, style::properties::label::menuBig);
	agcTresholdGainChangesWidgetLayout->addWidget(title, 0, 0);

	// adi,agc-outer-thresh-high
	IIOWidget *outerThreshHigh = IIOWidgetBuilder(agcTresholdGainChangesWidget)
					     .device(m_device)
					     .attribute("adi,agc-outer-thresh-high")
					     // //.uiStrategy(IIOWidgetBuilder::RangeUi)
					     .title("Outer Threshold High (dBFS)")
					     .buildSingle();
	agcTresholdGainChangesWidgetLayout->addWidget(outerThreshHigh, 1, 0);
	outerThreshHigh->setToolTip("The outer high threshold equals the inner high threshold plus this value. See "
				    "register 0x129, bits [D7:D4]. This data is processed by the driver.");

	// adi,agc-outer-thresh-high-dec-steps
	IIOWidget *outerThreshHighDecSteps = IIOWidgetBuilder(agcTresholdGainChangesWidget)
						     .device(m_device)
						     .attribute("adi,agc-outer-thresh-high-dec-steps")
						     // //.uiStrategy(IIOWidgetBuilder::RangeUi)
						     .title("Decrease Steps")
						     .buildSingle();
	agcTresholdGainChangesWidgetLayout->addWidget(outerThreshHighDecSteps, 1, 1);
	outerThreshHighDecSteps->setToolTip("The slow AGC changes gain by this amount when the outer high threshold is "
					    "exceeded. See register 0x12A, bits [D7:D4].");

	// adi,agc-inner-thresh-high
	IIOWidget *innerThreshHigh = IIOWidgetBuilder(agcTresholdGainChangesWidget)
					     .device(m_device)
					     .attribute("adi,agc-inner-thresh-high")
					     // //.uiStrategy(IIOWidgetBuilder::RangeUi)
					     .title("Inner Threshold High (dBFS) Fast AGC Lock Level (dBFS)")
					     .buildSingle();
	agcTresholdGainChangesWidgetLayout->addWidget(innerThreshHigh, 2, 0);
	innerThreshHigh->setToolTip(
		"	Applies to AGC. This attribute specifies the fast AGC lock level or specifies the slow AGC "
		"inner high threshold. Resolution is −1 dBFS/LSB. See register 0x101 [D6:D0]).");

	// adi,agc-inner-thresh-high-dec-steps
	IIOWidget *innerThreshHighDecSteps = IIOWidgetBuilder(agcTresholdGainChangesWidget)
						     .device(m_device)
						     .attribute("adi,agc-inner-thresh-high-dec-steps")
						     // //.uiStrategy(IIOWidgetBuilder::RangeUi)
						     .title("Decrease Steps")
						     .buildSingle();
	agcTresholdGainChangesWidgetLayout->addWidget(innerThreshHighDecSteps, 2, 1);
	innerThreshHighDecSteps->setToolTip("This attribute sets the gain decrease amount when the inner high "
					    "threshold is exceeded. See register 0x123, bits [D6:D4].");

	// adi,agc-inner-thresh-low
	IIOWidget *innerThreshLow = IIOWidgetBuilder(agcTresholdGainChangesWidget)
					    .device(m_device)
					    .attribute("adi,agc-inner-thresh-low")
					    // //.uiStrategy(IIOWidgetBuilder::RangeUi)
					    .title("Inner Threshold Low (dBFS)")
					    .buildSingle();
	agcTresholdGainChangesWidgetLayout->addWidget(innerThreshLow, 3, 0);
	innerThreshLow->setToolTip(
		"This attribute sets the slow AGC inner low window threshold. See register 0x120, bits [D6:D0].");

	// adi,agc-inner-thresh-low-inc-steps
	IIOWidget *innerThreshLowIncSteps = IIOWidgetBuilder(agcTresholdGainChangesWidget)
						    .device(m_device)
						    .attribute("adi,agc-inner-thresh-low-inc-steps")
						    // //.uiStrategy(IIOWidgetBuilder::RangeUi)
						    .title("Increase Steps")
						    .buildSingle();
	agcTresholdGainChangesWidgetLayout->addWidget(innerThreshLowIncSteps, 3, 1);
	innerThreshLowIncSteps->setToolTip("This attribute sets the increase amount used when the gain goes under the "
					   "inner low threshold. See register 0x123, bits [D2:D0].");

	// adi,agc-outer-thresh-low
	IIOWidget *outerThreshLow = IIOWidgetBuilder(agcTresholdGainChangesWidget)
					    .device(m_device)
					    .attribute("adi,agc-outer-thresh-low")
					    // //.uiStrategy(IIOWidgetBuilder::RangeUi)
					    .title("Outer Threshold Low (dBFS)")
					    .buildSingle();
	agcTresholdGainChangesWidgetLayout->addWidget(outerThreshLow, 4, 0);
	outerThreshLow->setToolTip("The outer low threshold equals the inner low threshold plus this value. See "
				   "register 0x129, bits [D3:D0]. This data is processed by the driver.");

	// adi,agc-outer-thresh-low-inc-steps
	IIOWidget *outerThreshLowIncSteps = IIOWidgetBuilder(agcTresholdGainChangesWidget)
						    .device(m_device)
						    .attribute("adi,agc-outer-thresh-low-inc-steps")
						    // //.uiStrategy(IIOWidgetBuilder::RangeUi)
						    .title("Increase Steps")
						    .buildSingle();
	agcTresholdGainChangesWidgetLayout->addWidget(outerThreshLowIncSteps, 4, 1);
	outerThreshLowIncSteps->setToolTip("	The slow AGC changes gain by this amount when the outer low threshold "
					   "is exceeded. See register 0x12A, bits [D3:D0].");

	// adi,agc-sync-for-gain-counter-enable
	IIOWidget *sync = IIOWidgetBuilder(agcTresholdGainChangesWidget)
				  .device(m_device)
				  .attribute("adi,agc-sync-for-gain-counter-enable")
				  .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
				  .title("AGC Sync for Gain Counter")
				  .buildSingle();
	agcTresholdGainChangesWidgetLayout->addWidget(sync, 5, 0);
	sync->setToolTip(
		"If this attribute is set, CTRL_IN2 transitioning high resets the counter.See register 0x128, bit D4.");

	// adi,agc-gain-update-interval-us
	IIOWidget *gainUpdateInterval = IIOWidgetBuilder(agcTresholdGainChangesWidget)
						.device(m_device)
						.attribute("adi,agc-gain-update-interval-us")
						// //.uiStrategy(IIOWidgetBuilder::RangeUi)
						.title("Gain Update Interval (us)")
						.buildSingle();
	agcTresholdGainChangesWidgetLayout->addWidget(gainUpdateInterval, 6, 0);

	connect(this, &GainWidget::readRequested, this, [=, this]() {
		outerThreshHigh->readAsync();
		outerThreshHighDecSteps->readAsync();
		innerThreshHigh->readAsync();
		innerThreshHighDecSteps->readAsync();
		innerThreshLow->readAsync();
		innerThreshLowIncSteps->readAsync();
		outerThreshLow->readAsync();
		outerThreshLowIncSteps->readAsync();
		sync->readAsync();
		gainUpdateInterval->readAsync();
	});

	return agcTresholdGainChangesWidget;
}

QWidget *GainWidget::adcOverloadWidget(QWidget *parent)
{
	QWidget *adcOverloadWidget = new QWidget(parent);
	QGridLayout *adcOverloadWidgetLayout = new QGridLayout(adcOverloadWidget);
	adcOverloadWidget->setLayout(adcOverloadWidgetLayout);

	Style::setStyle(adcOverloadWidget, style::properties::widget::border_interactive);
	Style::setBackgroundColor(adcOverloadWidget, json::theme::background_primary);

	QLabel *title = new QLabel("ADC Overload", adcOverloadWidget);
	Style::setStyle(title, style::properties::label::menuBig);
	adcOverloadWidgetLayout->addWidget(title, 0, 0);

	// adi,gc-adc-large-overload-thresh
	IIOWidget *largeOverloadThresh = IIOWidgetBuilder(adcOverloadWidget)
						 .device(m_device)
						 .attribute("adi,gc-adc-large-overload-thresh")
						 // //.uiStrategy(IIOWidgetBuilder::RangeUi)
						 .title("Large Thresh")
						 .buildSingle();
	adcOverloadWidgetLayout->addWidget(largeOverloadThresh, 1, 0);
	largeOverloadThresh->setToolTip("This attribute sets the large ADC overload. See register 0x105.");

	// adi,gc-adc-small-overload-thresh
	IIOWidget *smallOverloadThresh = IIOWidgetBuilder(adcOverloadWidget)
						 .device(m_device)
						 .attribute("adi,gc-adc-small-overload-thresh")
						 // //.uiStrategy(IIOWidgetBuilder::RangeUi)
						 .title("Small Thresh")
						 .buildSingle();
	adcOverloadWidgetLayout->addWidget(smallOverloadThresh, 1, 1);
	smallOverloadThresh->setToolTip("gc-adc-small-overload-thresh");

	// adi,agc-adc-large-overload-exceed-counter
	IIOWidget *largeOverloadExceed = IIOWidgetBuilder(adcOverloadWidget)
						 .device(m_device)
						 .attribute("adi,agc-adc-large-overload-exceed-counter")
						 //.uiStrategy(IIOWidgetBuilder::RangeUi)
						 .title("Large Exceed Cntr")
						 .buildSingle();
	adcOverloadWidgetLayout->addWidget(largeOverloadExceed, 2, 0);
	largeOverloadExceed->setToolTip(
		"This counter specifies the number of large ADC overloads that must occur before the gain will "
		"decrease by the large ADC overload gain step. See register 0x122, bits [D7:D4].");

	// adi,agc-adc-small-overload-exceed-counter
	IIOWidget *smallOverloadExceed = IIOWidgetBuilder(adcOverloadWidget)
						 .device(m_device)
						 .attribute("adi,agc-adc-small-overload-exceed-counter")
						 //.uiStrategy(IIOWidgetBuilder::RangeUi)
						 .title("Small Exceed Counter")
						 .buildSingle();
	adcOverloadWidgetLayout->addWidget(smallOverloadExceed, 2, 1);
	smallOverloadExceed->setToolTip("This counter specifies the number of small ADC overloads that must occur to "
					"prevent a gain increase. See register 0x122, bits [D3:D0].");

	// adi,agc-adc-large-overload-inc-steps
	IIOWidget *largeDecSteps = IIOWidgetBuilder(adcOverloadWidget)
					   .device(m_device)
					   .attribute("adi,agc-adc-large-overload-inc-steps")
					   //.uiStrategy(IIOWidgetBuilder::RangeUi)
					   .title("Large Decr. Steps ")
					   .buildSingle();
	adcOverloadWidgetLayout->addWidget(largeDecSteps, 3, 0);
	largeDecSteps->setToolTip("	This attribute applies to AGC and determine how much the gain changes for "
				  "large LPF in split tablemode or the large LMT and large ADC overloads in full table "
				  "mode. See register 0x106, bits [D3:D0]");

	// adi,agc-adc-lmt-small-overload-prevent-gain-inc-enable
	IIOWidget *preventGainIncrease = IIOWidgetBuilder(adcOverloadWidget)
						 .device(m_device)
						 .attribute("adi,agc-adc-lmt-small-overload-prevent-gain-inc-enable")
						 .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
						 .title("Prevent Gain Increase")
						 .buildSingle();
	adcOverloadWidgetLayout->addWidget(preventGainIncrease, 3, 1);
	preventGainIncrease->setToolTip(
		"This attribute set the slow AGC inner low window threshold. See register 0x120, bits [D6:D0].");

	// sum n samples TODO FIND ATTR

	// adi,agc-immed-gain-change-if-large-adc-overload-enable
	IIOWidget *immedGainChange = IIOWidgetBuilder(adcOverloadWidget)
					     .device(m_device)
					     .attribute("adi,agc-immed-gain-change-if-large-adc-overload-enable")
					     .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
					     .title("Immediate Gain Change if Large Overload")
					     .buildSingle();
	adcOverloadWidgetLayout->addWidget(immedGainChange, 4, 1);
	immedGainChange->setToolTip("Set this attribute to allow large ADC overload to reduce gain immediately. See "
				    "register 0x123, bit D3.");

	connect(this, &GainWidget::readRequested, this, [=, this]() {
		largeOverloadThresh->readAsync();
		smallOverloadThresh->readAsync();
		largeOverloadExceed->readAsync();
		smallOverloadExceed->readAsync();
		largeDecSteps->readAsync();
		preventGainIncrease->readAsync();
		immedGainChange->readAsync();
	});

	return adcOverloadWidget;
}

QWidget *GainWidget::lmtOverloadWidget(QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	QGridLayout *widgetLayout = new QGridLayout(widget);
	widget->setLayout(widgetLayout);

	Style::setStyle(widget, style::properties::widget::border_interactive);
	Style::setBackgroundColor(widget, json::theme::background_primary);

	QLabel *title = new QLabel("LMT Overload", widget);
	Style::setStyle(title, style::properties::label::menuBig);
	widgetLayout->addWidget(title, 0, 0);

	// adi,gc-lmt-overload-high-thresh
	IIOWidget *highThresh = IIOWidgetBuilder(widget)
					.device(m_device)
					.attribute("adi,gc-lmt-overload-high-thresh")
					.title("High Tresh (mV)")
					.buildSingle();
	widgetLayout->addWidget(highThresh, 1, 0);
	highThresh->setToolTip("This attribute sets the large LMT overload threshold. See register 0x108. This data is "
			       "processed by the driver.");

	// adi,gc-lmt-overload-low-thresh
	IIOWidget *lowThresh = IIOWidgetBuilder(widget)
				       .device(m_device)
				       .attribute("adi,gc-lmt-overload-low-thresh")
				       .title("Small Thresh")
				       .buildSingle();
	widgetLayout->addWidget(lowThresh, 1, 1);
	lowThresh->setToolTip("This attribute sets the small LMT overload threshold. See register 0x107. This data is "
			      "processed by the driver.");

	// adi,agc-lmt-overload-large-exceed-counter
	IIOWidget *largeExced = IIOWidgetBuilder(widget)
					.device(m_device)
					.attribute("adi,agc-lmt-overload-large-exceed-counter")
					.title("Large Exceed Counter")
					.buildSingle();
	widgetLayout->addWidget(largeExced, 2, 0);
	largeExced->setToolTip("This counter specifies the number of large LMT overloads that must occur before gain "
			       "decreases by the LMT Gain Step. See register 0x121, bits [D7:D4].");

	// adi,agc-lmt-overload-small-exceed-counter
	IIOWidget *smallExced = IIOWidgetBuilder(widget)
					.device(m_device)
					.attribute("adi,agc-lmt-overload-small-exceed-counter")
					.title("Small Exceed Cntr")
					.buildSingle();
	widgetLayout->addWidget(smallExced, 2, 1);
	smallExced->setToolTip("This counter specifies the number of small LMT overloads that much occur to prevent a "
			       "gain increase. See register 0x121, bits [D3:D0].");

	// adi,agc-lmt-overload-large-inc-steps
	IIOWidget *largeInc = IIOWidgetBuilder(widget)
				      .device(m_device)
				      .attribute("adi,agc-lmt-overload-large-inc-steps")
				      .title("Large Decr. Steps")
				      .buildSingle();
	widgetLayout->addWidget(largeInc, 3, 0);
	largeInc->setToolTip("	This attribute determines how much the gain changes for large LMT in split tablemode "
			     "or the small ADC overload for the full table. See register 0x103, bits [D4:D2].");

	// adi,agc-immed-gain-change-if-large-lmt-overload-enable
	IIOWidget *immedGain = IIOWidgetBuilder(widget)
				       .device(m_device)
				       .attribute("adi,agc-immed-gain-change-if-large-lmt-overload-enable")
				       .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
				       .title("Immediate Gain Change if Large Overload")
				       .buildSingle();
	widgetLayout->addWidget(immedGain, 3, 1);
	immedGain->setToolTip("Set this attribute to allow large LMT overloads to reduce gain immediately. See "
			      "register 0x123, bit D7.");

	connect(this, &GainWidget::readRequested, this, [=, this]() {
		highThresh->readAsync();
		lowThresh->readAsync();
		largeExced->readAsync();
		smallExced->readAsync();
		largeInc->readAsync();
		immedGain->readAsync();
	});

	return widget;
}

QWidget *GainWidget::digitalGainWidget(QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	QGridLayout *widgetLayout = new QGridLayout(widget);
	widget->setLayout(widgetLayout);

	Style::setStyle(widget, style::properties::widget::border_interactive);
	Style::setBackgroundColor(widget, json::theme::background_primary);

	QLabel *title = new QLabel("Digital Gain", widget);
	Style::setStyle(title, style::properties::label::menuBig);
	widgetLayout->addWidget(title, 0, 0);

	// adi,gc-dig-gain-enable
	IIOWidget *gainMode = IIOWidgetBuilder(widget)
				      .device(m_device)
				      .attribute("adi,gc-dig-gain-enable")
				      .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
				      .title("Dig Gain Enable")
				      .buildSingle();
	widgetLayout->addWidget(gainMode, 1, 0);
	gainMode->setToolTip("	This attribute is used in split table mode to enable the digital gain pointer. See "
			     "register 0x0FB, bit D2.");

	// adi,agc-dig-saturation-exceed-counter
	IIOWidget *saturationExceedCounter = IIOWidgetBuilder(widget)
						     .device(m_device)
						     .attribute("adi,agc-dig-saturation-exceed-counter")
						     //.uiStrategy(IIOWidgetBuilder::RangeUi)
						     .title("Saturation Exceed Counter")
						     .buildSingle();
	widgetLayout->addWidget(saturationExceedCounter, 2, 0);
	saturationExceedCounter->setToolTip("This counter specifies the number of digital saturation events that much "
					    "occur to prevent a gain increase. See register 0x128, bits [D3:D0].");

	// adi,gc-max-dig-gain
	IIOWidget *maxDigGain = IIOWidgetBuilder(widget)
					.device(m_device)
					.attribute("adi,gc-max-dig-gain")
					//.uiStrategy(IIOWidgetBuilder::RangeUi)
					.title("Max Digital Gain")
					.buildSingle();
	widgetLayout->addWidget(maxDigGain, 1, 1);
	maxDigGain->setToolTip("This attribute equals the maximum allowable digital gain, and applies to all gain "
			       "control modes. See register 0x100, bits [D4:D0].");

	// adi,agc-dig-gain-step-size
	IIOWidget *gainStepSize = IIOWidgetBuilder(widget)
					  .device(m_device)
					  .attribute("adi,agc-dig-gain-step-size")
					  //.uiStrategy(IIOWidgetBuilder::RangeUi)
					  .title("Decr Step Size")
					  .buildSingle();
	widgetLayout->addWidget(gainStepSize, 2, 1);
	gainStepSize->setToolTip(
		"If digital saturation occurs, digital gain reduces by this value. See register 0x100, bits [D7:D5].");

	connect(this, &GainWidget::readRequested, this, [=, this]() {
		gainMode->readAsync();
		saturationExceedCounter->readAsync();
		maxDigGain->readAsync();
		gainStepSize->readAsync();
	});

	return widget;
}

QWidget *GainWidget::fastAttackAGCWidget(QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	QGridLayout *layout = new QGridLayout(widget);
	widget->setLayout(layout);

	Style::setStyle(widget, style::properties::widget::border_interactive);
	Style::setBackgroundColor(widget, json::theme::background_primary);

	QLabel *title = new QLabel("Fast Attack AGC", widget);
	Style::setStyle(title, style::properties::label::menuBig);
	layout->addWidget(title);

	layout->addWidget(stateResponseToPeakOverload(widget));
	layout->addWidget(lowPowerCheck(widget));
	layout->addWidget(agcLockLevelAdjustment(widget));
	layout->addWidget(peakDetectors(widget));
	layout->addWidget(gainUnlock(widget));

	return widget;
}

QWidget *GainWidget::stateResponseToPeakOverload(QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	QVBoxLayout *layout = new QVBoxLayout(widget);
	widget->setLayout(layout);

	Style::setStyle(widget, style::properties::widget::border_interactive);

	QLabel *title = new QLabel("State 1 Response to Peak Overload", widget);
	Style::setStyle(title, style::properties::label::menuBig);
	layout->addWidget(title);

	// adi,fagc-state-wait-time-ns
	IIOWidget *stateWaitTime = IIOWidgetBuilder(widget)
					   .device(m_device)
					   .attribute("adi,fagc-state-wait-time-ns")
					   //.uiStrategy(IIOWidgetBuilder::RangeUi)
					   .title("State Wait Time")
					   .buildSingle();
	layout->addWidget(stateWaitTime);
	stateWaitTime->setToolTip("The fast AGC delays moving from State 1 to State 2 until no peak overloads are "
				  "detected for the value of this counter; measured in ClkRF cycles. See register "
				  "0x117, bits [D4:D0]. This data is processed by the driver.");

	connect(this, &GainWidget::readRequested, this, [=, this]() { stateWaitTime->readAsync(); });

	return widget;
}

QWidget *GainWidget::lowPowerCheck(QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	QGridLayout *layout = new QGridLayout(widget);
	widget->setLayout(layout);

	Style::setStyle(widget, style::properties::widget::border_interactive);

	QLabel *title = new QLabel("Low Power Check", widget);
	Style::setStyle(title, style::properties::label::menuBig);
	layout->addWidget(title, 0, 0);

	// adi,fagc-allow-agc-gain-increase-enable
	IIOWidget *allowGainIncrease = IIOWidgetBuilder(widget)
					       .device(m_device)
					       .attribute("adi,fagc-allow-agc-gain-increase-enable")
					       .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
					       .title("Allow AGC to increase Gain if signal below Low PWR Thresh")
					       .buildSingle();
	layout->addWidget(allowGainIncrease, 1, 0);
	allowGainIncrease->setToolTip(
		"Setting this attribute allows the fast AGC to increase the gain while optimizing the gain index. "
		"Clearing it prevents the gain from increasing in any condition. See register 0x110, bit D0.");

	// adi,gc-low-power-thresh
	IIOWidget *lowPowerThresh = IIOWidgetBuilder(widget)
					    .device(m_device)
					    .attribute("adi,gc-low-power-thresh")
					    //.uiStrategy(IIOWidgetBuilder::RangeUi)
					    .title("Low PWR Thresh (dbFS)")
					    .buildSingle();
	layout->addWidget(lowPowerThresh, 1, 1);
	lowPowerThresh->setToolTip("This threshold is used by the fast AGC to determine if the gain should be "
				   "increased. It can also be used to trigger a CTRL_OUT signal transition in MGC "
				   "mode. See register 0x114, bits [D6:D0]. This data is processed by the driver.");

	// adi,fagc-lp-thresh-increment-time
	IIOWidget *lowPowerThreshIncrementTime = IIOWidgetBuilder(widget)
							 .device(m_device)
							 .attribute("adi,fagc-lp-thresh-increment-time")
							 //.uiStrategy(IIOWidgetBuilder::RangeUi)
							 .title("Low PWR THresh Increment Time")
							 .buildSingle();
	layout->addWidget(lowPowerThreshIncrementTime, 2, 0);
	lowPowerThreshIncrementTime->setToolTip(
		"This attribute sets the time that the signal power must remain below the Low Power Threshold before "
		"the fast AGC will change gain. Also can be used by the MGC. See register 0x11B, bits [D7:D0].");

	// adi,fagc-lp-thresh-increment-steps
	IIOWidget *lowPowerThreshIncrementSteps = IIOWidgetBuilder(widget)
							  .device(m_device)
							  .attribute("adi,fagc-lp-thresh-increment-steps")
							  //.uiStrategy(IIOWidgetBuilder::RangeUi)
							  .title("Increment Step")
							  .buildSingle();
	layout->addWidget(lowPowerThreshIncrementSteps, 2, 1);
	lowPowerThreshIncrementSteps->setToolTip(
		"The Fast AGC will increase the gain index by this amount if signal power decreases below the Low "
		"Power Threshold and only if the Enable Incr Gain is enabled. See register 0x117, bits [D7:D5]. This "
		"data is processed by the driver.");

	connect(this, &GainWidget::readRequested, this, [=, this]() {
		allowGainIncrease->readAsync();
		lowPowerThresh->readAsync();
		lowPowerThreshIncrementTime->readAsync();
		lowPowerThreshIncrementSteps->readAsync();
	});

	return widget;
}

QWidget *GainWidget::agcLockLevelAdjustment(QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	QGridLayout *layout = new QGridLayout(widget);
	widget->setLayout(layout);

	Style::setStyle(widget, style::properties::widget::border_interactive);

	QLabel *title = new QLabel("AGC Lock Level Adjustment", widget);
	Style::setStyle(title, style::properties::label::menuBig);
	layout->addWidget(title, 0, 0);

	// adi,agc-inner-thresh-high
	IIOWidget *innerThreshHigh = IIOWidgetBuilder(widget)
					     .device(m_device)
					     .attribute("adi,agc-inner-thresh-high")
					     //.uiStrategy(IIOWidgetBuilder::RangeUi)
					     .title("AGC Lock Level (dBFS) (Inner Threshold High)")
					     .buildSingle();
	layout->addWidget(innerThreshHigh, 1, 0);
	innerThreshHigh->setToolTip(
		"Applies to AGC. This attribute specifies the fast AGC lock level or specifies the slow AGC inner high "
		"threshold. Resolution is −1 dBFS/LSB. See register 0x101 [D6:D0]).");

	// adi,fagc-lock-level-gain-increase-upper-limit
	IIOWidget *lockLevelGain = IIOWidgetBuilder(widget)
					   .device(m_device)
					   .attribute("adi,fagc-lock-level-gain-increase-upper-limit")
					   //.uiStrategy(IIOWidgetBuilder::RangeUi)
					   .title("Lock Level Gain Increase Upper Limit")
					   .buildSingle();
	layout->addWidget(lockLevelGain, 2, 0);
	lockLevelGain->setToolTip("This attribute sets the maximum gain index increase that the fast AGC can use for "
				  "the lock level adjustment. See register 0x118, bits [D5:D0].");

	// adi,fagc-lock-level-lmt-gain-increase-enable
	IIOWidget *lockLevelLmtGain = IIOWidgetBuilder(widget)
					      .device(m_device)
					      .attribute("adi,fagc-lock-level-lmt-gain-increase-enable")
					      .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
					      .title("Allow LMT Gain Increase")
					      .buildSingle();
	layout->addWidget(lockLevelLmtGain, 2, 1);
	lockLevelLmtGain->setToolTip("Set this attribute to allow the AGC to use LMT gain if the gain index needs to "
				     "increase when moving to the AGC Lock Level. See register 0x111, bit D6.");

	connect(this, &GainWidget::readRequested, this, [=, this]() {
		innerThreshHigh->readAsync();
		lockLevelGain->readAsync();
		lockLevelLmtGain->readAsync();
	});

	return widget;
}

QWidget *GainWidget::peakDetectors(QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	QGridLayout *layout = new QGridLayout(widget);
	widget->setLayout(layout);

	Style::setStyle(widget, style::properties::widget::border_interactive);

	QLabel *title = new QLabel("Peak Detectors (again), Final Low Power Test and Settling", widget);
	Style::setStyle(title, style::properties::label::menuBig);
	layout->addWidget(title, 0, 0);

	// adi,fagc-lpf-final-settling-steps
	IIOWidget *lpfFinalSettingsSteps = IIOWidgetBuilder(widget)
						   .device(m_device)
						   .attribute("adi,fagc-lpf-final-settling-steps")
						   //.uiStrategy(IIOWidgetBuilder::RangeUi)
						   .title("LPF / Full Table Final, Final Settling Steps")
						   .buildSingle();
	layout->addWidget(lpfFinalSettingsSteps, 1, 0);
	lpfFinalSettingsSteps->setToolTip(
		"This attribute sets the reduction to the gain index if a large LMT or large ADC overload occurs after "
		"Lock Level but before fast AGC state 5. If the number of overloads exceeds the Final Overrange Count "
		"(fagc_final_overrange_count), the AGC algorithm restarts. Depending on various conditions if a split "
		"table is used, the gain may reduce in in the LPF or the LMT (fagc_lmt_final_settling_steps). See "
		"register 0x112, bits [D7:D6].");

	// adi,fagc-lmt-final-settling-steps
	IIOWidget *lmtFinalSettingsSteps = IIOWidgetBuilder(widget)
						   .device(m_device)
						   .attribute("adi,fagc-lmt-final-settling-steps")
						   //.uiStrategy(IIOWidgetBuilder::RangeUi)
						   .title("LMT Final Settling Steps")
						   .buildSingle();
	layout->addWidget(lmtFinalSettingsSteps, 1, 1);
	lmtFinalSettingsSteps->setToolTip("Post Lock Level Step for LMT Table. See register 0x113, bits [D7:D6].");

	// adi,fagc-final-overrange-count
	IIOWidget *finalOverrange = IIOWidgetBuilder(widget)
					    .device(m_device)
					    .attribute("adi,fagc-final-overrange-count")
					    //.uiStrategy(IIOWidgetBuilder::RangeUi)
					    .title("Final Overrange Count")
					    .buildSingle();
	layout->addWidget(finalOverrange, 2, 0);
	finalOverrange->setToolTip("Final Overrange Count. See register 0x116, bits [D7:D5].");

	// adi,fagc-gain-increase-after-gain-lock-enable
	IIOWidget *increaseAfterGainLock = IIOWidgetBuilder(widget)
						   .device(m_device)
						   .attribute("adi,fagc-gain-increase-after-gain-lock-enable")
						   .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
						   .title("Low Power Test Gain Increase after Gain Lock")
						   .buildSingle();
	layout->addWidget(increaseAfterGainLock, 2, 1);
	increaseAfterGainLock->setToolTip(
		"Set this attribute to allow gain increases after the gain has locked but before State 5. Signal power "
		"must be lower than the low power threshold for longer than the increment time duration register. See "
		"register 0x110, bit D7.");

	connect(this, &GainWidget::readRequested, this, [=, this]() {
		lpfFinalSettingsSteps->readAsync();
		lmtFinalSettingsSteps->readAsync();
		finalOverrange->readAsync();
		increaseAfterGainLock->readAsync();
	});

	return widget;
}

QWidget *GainWidget::gainUnlock(QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	QGridLayout *layout = new QGridLayout(widget);
	widget->setLayout(layout);

	Style::setStyle(widget, style::properties::widget::border_interactive);

	QLabel *title = new QLabel("Gain Unlock", widget);
	Style::setStyle(title, style::properties::label::menuBig);
	layout->addWidget(title, 0, 0);

	// adi,fagc-gain-index-type-after-exit-rx-mode
	IIOWidget *gainIndexType = IIOWidgetBuilder(widget)
					   .device(m_device)
					   .attribute("adi,fagc-gain-index-type-after-exit-rx-mode")
					   .uiStrategy(IIOWidgetBuilder::ComboUi)
					   .title("When exit Receive State Restart Gain Lock Algorithm (GLA) and goto")
					   .buildSingle();
	layout->addWidget(gainIndexType, 1, 0);
	gainIndexType->setToolTip("MAX Gain (0); Optimized Gain (1); Set Gain (2). See register 0x110, bits D4,D2. "
				  "This data is processed by the driver.");

	// adi,fagc-use-last-lock-level-for-set-gain-enable
	IIOWidget *increaseAfterGainLock = IIOWidgetBuilder(widget)
						   .device(m_device)
						   .attribute("adi,fagc-use-last-lock-level-for-set-gain-enable")
						   .uiStrategy(IIOWidgetBuilder::ComboUi)
						   .title("SET Gain use Gain from")
						   .buildSingle();
	layout->addWidget(increaseAfterGainLock, 2, 0);
	increaseAfterGainLock->setToolTip(
		"Set this attribute to use the last gain index of the previous frame for set gain. Clear to use the "
		"first gain index of the previous frame. See register 0x111, bit D7.");

	// adi,fagc-optimized-gain-offset
	IIOWidget *optimiezedGainOffset = IIOWidgetBuilder(widget)
						  .device(m_device)
						  .attribute("adi,fagc-optimized-gain-offset")
						  //.uiStrategy(IIOWidgetBuilder::RangeUi)
						  .title("Optimized Gain Offset")
						  .buildSingle();
	layout->addWidget(optimiezedGainOffset, 2, 2);

	// adi,fagc-rst-gla-stronger-sig-thresh-exceeded-enable
	IIOWidget *sigThreshExceedEn = IIOWidgetBuilder(widget)
					       .device(m_device)
					       .attribute("adi,fagc-rst-gla-stronger-sig-thresh-exceeded-enable")
					       .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
					       .title("If Signal Power increases above Lock Level by (dBFS)")
					       .buildSingle();
	layout->addWidget(sigThreshExceedEn, 3, 0);
	sigThreshExceedEn->setToolTip(
		"If this attribute is set and the fast AGC is in State 5, the gain will not change even if the signal "
		"power increase by more than the Stronger Signal Threshold. See register 0x115, bit D7.");

	// adi,fagc-rst-gla-stronger-sig-thresh-above-ll
	IIOWidget *fagcRstGlaStrongerSigThresh = IIOWidgetBuilder(widget)
							 .device(m_device)
							 .attribute("adi,fagc-rst-gla-stronger-sig-thresh-above-ll")
							 //.uiStrategy(IIOWidgetBuilder::RangeUi)
							 .title(" ")
							 .buildSingle();
	layout->addWidget(fagcRstGlaStrongerSigThresh, 3, 1);
	fagcRstGlaStrongerSigThresh->setToolTip(
		"If the signal power increases by this threshold and the signal power remains at this level or higher "
		"for a duration that is twice the Gain Lock Exit Count, the gain may unlock, depending on other AGC "
		"configuration bits. See register 0x113, bits [D5:D0].");

	// adi,fagc-energy-lost-stronger-sig-gain-lock-exit-cnt
	IIOWidget *energyLostStronger = IIOWidgetBuilder(widget)
						.device(m_device)
						.attribute("adi,fagc-energy-lost-stronger-sig-gain-lock-exit-cnt")
						//.uiStrategy(IIOWidgetBuilder::RangeUi)
						.title("for (RX Samples) restart GLA")
						.buildSingle();
	layout->addWidget(energyLostStronger, 3, 2);
	energyLostStronger->setToolTip("Gain Lock Exit Count. See register 0x119, bits [D5:D0].");

	// adi,fagc-rst-gla-engergy-lost-sig-thresh-exceeded-enable
	IIOWidget *rstGlaEngLostSigThreshExceedEn =
		IIOWidgetBuilder(widget)
			.device(m_device)
			.attribute("adi,fagc-rst-gla-engergy-lost-sig-thresh-exceeded-enable")
			.uiStrategy(IIOWidgetBuilder::CheckBoxUi)
			.title("If Signal Power decreases below Lock Level by (dBFS)")
			.buildSingle();
	layout->addWidget(rstGlaEngLostSigThreshExceedEn, 4, 0);
	rstGlaEngLostSigThreshExceedEn->setToolTip(
		"If this attribute is set and the fast AGC is in State 5, the gain will not change even if the average "
		"signal power decreases more than the Energy Lost Threshold register. See register 0x110, bit D3.");

	// adi,fagc-rst-gla-engergy-lost-sig-thresh-below-ll
	IIOWidget *rstGlaEnergyLost = IIOWidgetBuilder(widget)
					      .device(m_device)
					      .attribute("adi,fagc-rst-gla-engergy-lost-sig-thresh-below-ll")
					      //.uiStrategy(IIOWidgetBuilder::RangeUi)
					      .title(" ")
					      .buildSingle();
	layout->addWidget(rstGlaEnergyLost, 4, 1);
	rstGlaEnergyLost->setToolTip(
		"If the signal power decreases by this threshold and the signal power remains at this level or lower "
		"for a duration that is twice the Gain Lock Exit Count, the gain may unlock, depending on other AGC "
		"configuration bits. See register 0x112, bits [D5:D0].");

	// adi,fagc-rst-gla-large-lmt-overload-enable
	IIOWidget *rstGlaLargeLmtOverloadEn =
		IIOWidgetBuilder(widget)
			.device(m_device)
			.attribute("adi,fagc-rst-gla-large-lmt-overload-enable")
			.uiStrategy(IIOWidgetBuilder::CheckBoxUi)
			.title("If Large LMT Overload, Do Not Change Gain but Restart GLA")
			.buildSingle();
	layout->addWidget(rstGlaLargeLmtOverloadEn, 5, 0);
	rstGlaLargeLmtOverloadEn->setToolTip("Unlock Gain if Lg ADC or LMT Ovrg. See register 0x110, bit D1.");

	// adi,fagc-rst-gla-engergy-lost-goto-optim-gain-enable
	IIOWidget *rstGlaEnergyLostGotoOptimGainEn =
		IIOWidgetBuilder(widget)
			.device(m_device)
			.attribute("adi,fagc-rst-gla-engergy-lost-goto-optim-gain-enable")
			.uiStrategy(IIOWidgetBuilder::ComboUi)
			.title("... and GTO")
			.buildSingle();
	layout->addWidget(rstGlaEnergyLostGotoOptimGainEn, 5, 2);
	rstGlaEnergyLostGotoOptimGainEn->setToolTip(
		"	If this attribute is set and the fast AGC is in State 5, the gain index will go to the "
		"optimize gain value if an energy lost state occurs or when the EN_AGC signal goes high. See register "
		"0x110, bit D6.");

	// adi,fagc-rst-gla-large-adc-overload-enable
	IIOWidget *glaLargeAdcOverloadEn = IIOWidgetBuilder(widget)
						   .device(m_device)
						   .attribute("adi,fagc-rst-gla-large-adc-overload-enable")
						   .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
						   .title("If Large ADC Overload do not change Gain but restart GLA")
						   .buildSingle();
	layout->addWidget(glaLargeAdcOverloadEn, 6, 0);
	glaLargeAdcOverloadEn->setToolTip("Unlock gain if ADC Ovrg, Lg ADC or LMT Ovrg. See register 0x110, bit D1 and "
					  "register 0x114, bit D7. This data is processed by the driver.");

	// adi,fagc-rst-gla-en-agc-pulled-high-enable
	IIOWidget *rstGlaEnAgc = IIOWidgetBuilder(widget)
					 .device(m_device)
					 .attribute("adi,fagc-gain-increase-after-gain-lock-enable")
					 .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
					 .title("If EN_AGC is pulled high restart GLA and goto")
					 .buildSingle();
	layout->addWidget(rstGlaEnAgc, 7, 0);
	rstGlaEnAgc->setToolTip("See fagc_rst_gla_if_en_agc_pulled_high_mode. This data is processed by the driver.");

	// adi,fagc-rst-gla-if-en-agc-pulled-high-mode
	IIOWidget *rstGlaIfEnAgcPulled = IIOWidgetBuilder(widget)
						 .device(m_device)
						 .attribute("adi,fagc-rst-gla-if-en-agc-pulled-high-mode")
						 .title(" ")
						 .buildSingle();
	layout->addWidget(rstGlaIfEnAgcPulled, 7, 1);
	rstGlaIfEnAgcPulled->setToolTip(
		"MAX Gain (0); Optimized Gain (1); Set Gain (2), No gain change. See registers 0x110, 0x111");

	// adi,fagc-power-measurement-duration-in-state5
	IIOWidget *powerMeasurementDuration =
		IIOWidgetBuilder(widget)
			.device(m_device)
			.attribute("adi,fagc-power-measurement-duration-in-state5")
			//.uiStrategy(IIOWidgetBuilder::RangeUi)
			.title("When testing signal power against the thresholds above, measure power for "
			       "(sample periods)")
			.buildSingle();
	layout->addWidget(powerMeasurementDuration, 8, 0);
	powerMeasurementDuration->setToolTip(
		"The power measurement duration used by the gain control algorithm for State 5 (gain lock) - fast AGC. "
		"See register 0x109, bit D7 and 0x10a, bits [D7:D5].");

	connect(this, &GainWidget::readRequested, this, [=, this]() {
		gainIndexType->readAsync();
		increaseAfterGainLock->readAsync();
		optimiezedGainOffset->readAsync();
		sigThreshExceedEn->readAsync();
		fagcRstGlaStrongerSigThresh->readAsync();
		energyLostStronger->readAsync();
		rstGlaEngLostSigThreshExceedEn->readAsync();
		rstGlaEnergyLost->readAsync();
		rstGlaLargeLmtOverloadEn->readAsync();
		rstGlaEnergyLostGotoOptimGainEn->readAsync();
		glaLargeAdcOverloadEn->readAsync();
		rstGlaEnAgc->readAsync();
		rstGlaIfEnAgcPulled->readAsync();
		powerMeasurementDuration->readAsync();
	});

	return widget;
}

QWidget *GainWidget::miscWidget(QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	QVBoxLayout *layout = new QVBoxLayout(widget);
	widget->setLayout(layout);

	Style::setStyle(widget, style::properties::widget::border_interactive);
	Style::setBackgroundColor(widget, json::theme::background_primary);

	QLabel *title = new QLabel("MISC", widget);
	Style::setStyle(title, style::properties::label::menuBig);
	layout->addWidget(title);

	// adi,fagc-dec-pow-measurement-duration
	IIOWidget *stateWaitTime = IIOWidgetBuilder(widget)
					   .device(m_device)
					   .attribute("adi,fagc-dec-pow-measurement-duration")
					   //.uiStrategy(IIOWidgetBuilder::RangeUi)
					   .title("Dec PWR mess. duration in fast attack mode")
					   .buildSingle();
	layout->addWidget(stateWaitTime);
	stateWaitTime->setToolTip("The power measurement duration used by the gain control algorithm. See register "
				  "0x15C, bits [D3:D0]. This data is processed by the driver.");

	connect(this, &GainWidget::readRequested, this, [=, this]() { stateWaitTime->readAsync(); });

	return widget;
}
