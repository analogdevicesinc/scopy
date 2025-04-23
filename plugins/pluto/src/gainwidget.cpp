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
	setLayout(m_layout);

	QWidget *gainWidget = new QWidget(parent);
	QVBoxLayout *layout = new QVBoxLayout(gainWidget);
	gainWidget->setLayout(layout);
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

	modeWidgetLayout->addWidget(new QLabel("Mode", modeWidget), 0, 0);

	// adi,gc-rx1-mode
	IIOWidget *rx1Mode = IIOWidgetBuilder(modeWidget)
				     .device(m_device)
				     .attribute("adi,gc-rx1-mode")
				     // .uiStrategy(IIOWidgetBuilder::ComboUi)
				     .title("RX1")
				     .buildSingle();
	modeWidgetLayout->addWidget(rx1Mode, 1, 0);

	// adi,gc-rx2-mode
	IIOWidget *rx2Mode = IIOWidgetBuilder(modeWidget)
				     .device(m_device)
				     .attribute("adi,gc-rx2-mode")
				     // .uiStrategy(IIOWidgetBuilder::ComboUi)
				     .title("RX2")
				     .buildSingle();
	modeWidgetLayout->addWidget(rx2Mode, 1, 1);

	// adi,split-gain-table-mode-enable
	IIOWidget *tableMode = IIOWidgetBuilder(modeWidget)
				       .device(m_device)
				       .attribute("adi,split-gain-table-mode-enable")
				       // .uiStrategy(IIOWidgetBuilder::ComboUi)
				       .title("Table Mode")
				       .buildSingle();
	modeWidgetLayout->addWidget(tableMode, 1, 2);

	// adi,gc-dec-pow-measurement-duration
	IIOWidget *decpowMeasurement = IIOWidgetBuilder(modeWidget)
					       .device(m_device)
					       .attribute("adi,gc-dec-pow-measurement-duration")
					       // .uiStrategy(IIOWidgetBuilder::ComboUi)
					       .title("Dec PWR mess. durattion in MGC and Slow Attack AGC mode")
					       .buildSingle();
	modeWidgetLayout->addWidget(decpowMeasurement, 2, 0);

	// adi,gc-low-power-thresh
	IIOWidget *lowPowerThresh = IIOWidgetBuilder(modeWidget)
					    .device(m_device)
					    .attribute("adi,gc-low-power-thresh")
					    // .uiStrategy(IIOWidgetBuilder::ComboUi)
					    .title("Low PWR Thresh (dbFS)")
					    .buildSingle();
	modeWidgetLayout->addWidget(lowPowerThresh, 3, 0);

	// adi,agc-attack-delay-extra-margin-us
	IIOWidget *attackDelay = IIOWidgetBuilder(modeWidget)
					 .device(m_device)
					 .attribute("adi,agc-attack-delay-extra-margin-us")
					 // .uiStrategy(IIOWidgetBuilder::ComboUi)
					 .title("Attack Delay extra margin (us)")
					 .buildSingle();
	modeWidgetLayout->addWidget(attackDelay, 2, 1);

	// adi,gc-use-rx-fir-out-for-dec-pwr-meas-enable
	IIOWidget *rxFirOut = IIOWidgetBuilder(modeWidget)
				      .device(m_device)
				      .attribute("adi,gc-use-rx-fir-out-for-dec-pwr-meas-enable")
				      .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
				      .title("Use Rx FIR output for Dec. PWR measurements")
				      .buildSingle();
	modeWidgetLayout->addWidget(rxFirOut, 3, 1);

	return modeWidget;
}

QWidget *GainWidget::mgcWidget(QWidget *parent)
{
	QWidget *mgcWidget = new QWidget(parent);
	QGridLayout *mgcWidgetLayout = new QGridLayout(mgcWidget);
	mgcWidget->setLayout(mgcWidgetLayout);

	Style::setStyle(mgcWidget, style::properties::widget::border_interactive);
	Style::setBackgroundColor(mgcWidget, json::theme::background_primary);

	mgcWidgetLayout->addWidget(new QLabel("MGC", mgcWidget), 0, 0);

	// adi,mgc-rx1-ctrl-inp-enable
	IIOWidget *mgcRx1Ctrl = IIOWidgetBuilder(mgcWidget)
					.device(m_device)
					.attribute("adi,mgc-rx1-ctrl-inp-enable")
					.uiStrategy(IIOWidgetBuilder::CheckBoxUi)
					.title("RX1 Control Input")
					.buildSingle();
	mgcWidgetLayout->addWidget(mgcRx1Ctrl, 1, 0);

	// adi,mgc-rx2-ctrl-inp-enable
	IIOWidget *mgcRx2Ctrl = IIOWidgetBuilder(mgcWidget)
					.device(m_device)
					.attribute("adi,mgc-rx2-ctrl-inp-enable")
					.uiStrategy(IIOWidgetBuilder::CheckBoxUi)
					.title("RX2 Control Input")
					.buildSingle();
	mgcWidgetLayout->addWidget(mgcRx2Ctrl, 1, 1);

	// adi,mgc-inc-gain-step
	IIOWidget *mgcIncGain = IIOWidgetBuilder(mgcWidget)
					.device(m_device)
					.attribute("adi,mgc-inc-gain-step")
					.uiStrategy(IIOWidgetBuilder::RangeUi)
					.title("Increase Gain Step")
					.buildSingle();
	mgcWidgetLayout->addWidget(mgcIncGain, 2, 0);

	// adi,mgc-dec-gain-step
	IIOWidget *mgcDecGain = IIOWidgetBuilder(mgcWidget)
					.device(m_device)
					.attribute("adi,mgc-inc-gain-step")
					.title("Decrease Gain Step")
					.uiStrategy(IIOWidgetBuilder::RangeUi)
					.buildSingle();
	mgcWidgetLayout->addWidget(mgcDecGain, 2, 1);

	// adi,mgc-split-table-ctrl-inp-gain-mode
	IIOWidget *mgcSplitTableCtrl = IIOWidgetBuilder(mgcWidget)
					       .device(m_device)
					       .attribute("adi,mgc-inc-gain-step")
					       .title("Split Table Control Input Mode")
					       .buildSingle();
	mgcWidgetLayout->addWidget(mgcSplitTableCtrl, 3, 0);

	return mgcWidget;
}

QWidget *GainWidget::agcThresholdGainChangesWidget(QWidget *parent)
{
	QWidget *agcTresholdGainChangesWidget = new QWidget(parent);
	QGridLayout *agcTresholdGainChangesWidgetLayout = new QGridLayout(agcTresholdGainChangesWidget);
	agcTresholdGainChangesWidget->setLayout(agcTresholdGainChangesWidgetLayout);

	Style::setStyle(agcTresholdGainChangesWidget, style::properties::widget::border_interactive);
	Style::setBackgroundColor(agcTresholdGainChangesWidget, json::theme::background_primary);

	agcTresholdGainChangesWidgetLayout->addWidget(
		new QLabel("AGC Thresholds and Gain Changes", agcTresholdGainChangesWidget), 0, 0);

	// adi,agc-outer-thresh-high
	IIOWidget *outerThreshHigh = IIOWidgetBuilder(agcTresholdGainChangesWidget)
					     .device(m_device)
					     .attribute("adi,agc-outer-thresh-high")
					     .uiStrategy(IIOWidgetBuilder::RangeUi)
					     .title("Outer Threshold High (dBFS)")
					     .buildSingle();
	agcTresholdGainChangesWidgetLayout->addWidget(outerThreshHigh, 1, 0);

	// adi,agc-outer-thresh-high-dec-steps
	IIOWidget *outerThreshHighDecSteps = IIOWidgetBuilder(agcTresholdGainChangesWidget)
						     .device(m_device)
						     .attribute("adi,agc-outer-thresh-high-dec-steps")
						     .uiStrategy(IIOWidgetBuilder::RangeUi)
						     .title("Decrease Steps")
						     .buildSingle();
	agcTresholdGainChangesWidgetLayout->addWidget(outerThreshHighDecSteps, 1, 1);

	// adi,agc-inner-thresh-high
	IIOWidget *innerThreshHigh = IIOWidgetBuilder(agcTresholdGainChangesWidget)
					     .device(m_device)
					     .attribute("adi,agc-inner-thresh-high")
					     .uiStrategy(IIOWidgetBuilder::RangeUi)
					     .title("Inner Threshold High (dBFS) Fast AGC Lock Level (dBFS)")
					     .buildSingle();
	agcTresholdGainChangesWidgetLayout->addWidget(innerThreshHigh, 2, 0);

	// adi,agc-inner-thresh-high-dec-steps
	IIOWidget *innerThreshHighDecSteps = IIOWidgetBuilder(agcTresholdGainChangesWidget)
						     .device(m_device)
						     .attribute("adi,agc-inner-thresh-high-dec-steps")
						     .uiStrategy(IIOWidgetBuilder::RangeUi)
						     .title("Decrease Steps")
						     .buildSingle();
	agcTresholdGainChangesWidgetLayout->addWidget(innerThreshHighDecSteps, 2, 1);

	// adi,agc-inner-thresh-low
	IIOWidget *innerThreshLow = IIOWidgetBuilder(agcTresholdGainChangesWidget)
					    .device(m_device)
					    .attribute("adi,agc-inner-thresh-low")
					    .uiStrategy(IIOWidgetBuilder::RangeUi)
					    .title("Inner Threshold Low (dBFS)")
					    .buildSingle();
	agcTresholdGainChangesWidgetLayout->addWidget(innerThreshLow, 3, 0);

	// adi,agc-inner-thresh-low-inc-steps
	IIOWidget *innerThreshLowIncSteps = IIOWidgetBuilder(agcTresholdGainChangesWidget)
						    .device(m_device)
						    .attribute("adi,agc-inner-thresh-low-inc-steps")
						    .uiStrategy(IIOWidgetBuilder::RangeUi)
						    .title("Increase Steps")
						    .buildSingle();
	agcTresholdGainChangesWidgetLayout->addWidget(innerThreshLowIncSteps, 3, 1);

	// adi,agc-outer-thresh-low
	IIOWidget *outerThreshLow = IIOWidgetBuilder(agcTresholdGainChangesWidget)
					    .device(m_device)
					    .attribute("adi,agc-outer-thresh-low")
					    .uiStrategy(IIOWidgetBuilder::RangeUi)
					    .title("Outer Threshold Low (dBFS)")
					    .buildSingle();
	agcTresholdGainChangesWidgetLayout->addWidget(outerThreshLow, 4, 0);

	// adi,agc-outer-thresh-low-inc-steps
	IIOWidget *outerThreshLowIncSteps = IIOWidgetBuilder(agcTresholdGainChangesWidget)
						    .device(m_device)
						    .attribute("adi,agc-outer-thresh-low-inc-steps")
						    .uiStrategy(IIOWidgetBuilder::RangeUi)
						    .title("Increase Steps")
						    .buildSingle();
	agcTresholdGainChangesWidgetLayout->addWidget(outerThreshLowIncSteps, 4, 1);

	// adi,agc-sync-for-gain-counter-enable
	IIOWidget *sync = IIOWidgetBuilder(agcTresholdGainChangesWidget)
				  .device(m_device)
				  .attribute("adi,agc-sync-for-gain-counter-enable")
				  .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
				  .title("AGC Sync for Gain Coutner")
				  .buildSingle();
	agcTresholdGainChangesWidgetLayout->addWidget(sync, 5, 0);

	// adi,agc-gain-update-interval-us
	IIOWidget *gainUpdateInterval = IIOWidgetBuilder(agcTresholdGainChangesWidget)
						.device(m_device)
						.attribute("adi,agc-gain-update-interval-us")
						.uiStrategy(IIOWidgetBuilder::RangeUi)
						.title("Gain Update Interval (us)")
						.buildSingle();
	agcTresholdGainChangesWidgetLayout->addWidget(gainUpdateInterval, 6, 0);

	return agcTresholdGainChangesWidget;
}

QWidget *GainWidget::adcOverloadWidget(QWidget *parent)
{
	QWidget *adcOverloadWidget = new QWidget(parent);
	QGridLayout *adcOverloadWidgetLayout = new QGridLayout(adcOverloadWidget);
	adcOverloadWidget->setLayout(adcOverloadWidgetLayout);

	Style::setStyle(adcOverloadWidget, style::properties::widget::border_interactive);
	Style::setBackgroundColor(adcOverloadWidget, json::theme::background_primary);

	adcOverloadWidgetLayout->addWidget(new QLabel("ADC Overload", adcOverloadWidget), 0, 0);

	// adi,gc-adc-large-overload-thresh
	IIOWidget *largeOverloadThresh = IIOWidgetBuilder(adcOverloadWidget)
						 .device(m_device)
						 .attribute("adi,gc-adc-large-overload-thresh")
						 .uiStrategy(IIOWidgetBuilder::RangeUi)
						 .title("Large Thresh")
						 .buildSingle();
	adcOverloadWidgetLayout->addWidget(largeOverloadThresh, 1, 0);

	// adi,gc-adc-small-overload-thresh
	IIOWidget *smallOverloadThresh = IIOWidgetBuilder(adcOverloadWidget)
						 .device(m_device)
						 .attribute("adi,gc-adc-small-overload-thresh")
						 .uiStrategy(IIOWidgetBuilder::RangeUi)
						 .title("Small Thresh")
						 .buildSingle();
	adcOverloadWidgetLayout->addWidget(smallOverloadThresh, 1, 1);

	// adi,agc-adc-large-overload-exceed-counter
	IIOWidget *largeOverloadExceed = IIOWidgetBuilder(adcOverloadWidget)
						 .device(m_device)
						 .attribute("adi,agc-adc-large-overload-exceed-counter")
						 .uiStrategy(IIOWidgetBuilder::RangeUi)
						 .title("Large Exceed Cntr")
						 .buildSingle();
	adcOverloadWidgetLayout->addWidget(largeOverloadExceed, 2, 0);

	// adi,agc-adc-small-overload-exceed-counter
	IIOWidget *smallOverloadExceed = IIOWidgetBuilder(adcOverloadWidget)
						 .device(m_device)
						 .attribute("adi,agc-adc-small-overload-exceed-counter")
						 .uiStrategy(IIOWidgetBuilder::RangeUi)
						 .title("Small Exeed Cntr")
						 .buildSingle();
	adcOverloadWidgetLayout->addWidget(smallOverloadExceed, 2, 1);

	// adi,agc-adc-large-overload-inc-steps
	IIOWidget *largeDecSteps = IIOWidgetBuilder(adcOverloadWidget)
					   .device(m_device)
					   .attribute("adi,agc-adc-large-overload-inc-steps")
					   .uiStrategy(IIOWidgetBuilder::RangeUi)
					   .title("Large Decr. Steps ")
					   .buildSingle();
	adcOverloadWidgetLayout->addWidget(largeDecSteps, 3, 0);

	// adi,agc-adc-lmt-small-overload-prevent-gain-inc-enable
	IIOWidget *prevertGainIncrease = IIOWidgetBuilder(adcOverloadWidget)
						 .device(m_device)
						 .attribute("adi,agc-adc-lmt-small-overload-prevent-gain-inc-enable")
						 .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
						 .title("Prevent Gain Increase")
						 .buildSingle();
	adcOverloadWidgetLayout->addWidget(prevertGainIncrease, 3, 1);

	// sum n samples TODO FIND ATTR

	// adi,agc-immed-gain-change-if-large-adc-overload-enable
	IIOWidget *immedGainChange = IIOWidgetBuilder(adcOverloadWidget)
					     .device(m_device)
					     .attribute("adi,agc-immed-gain-change-if-large-adc-overload-enable")
					     .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
					     .title("Immed Gain change if Large Overload")
					     .buildSingle();
	adcOverloadWidgetLayout->addWidget(immedGainChange, 4, 1);

	return adcOverloadWidget;
}

QWidget *GainWidget::lmtOverloadWidget(QWidget *parent)
{
	QWidget *lmtOverloadWidget = new QWidget(parent);
	QGridLayout *lmtOverloadWidgetLayout = new QGridLayout(lmtOverloadWidget);
	lmtOverloadWidget->setLayout(lmtOverloadWidgetLayout);

	Style::setStyle(lmtOverloadWidget, style::properties::widget::border_interactive);
	Style::setBackgroundColor(lmtOverloadWidget, json::theme::background_primary);

	lmtOverloadWidgetLayout->addWidget(new QLabel("LMT Overload", lmtOverloadWidget), 0, 0);

	// adi,gc-lmt-overload-high-thresh
	IIOWidget *highThresh = IIOWidgetBuilder(lmtOverloadWidget)
					.device(m_device)
					.attribute("adi,gc-lmt-overload-high-thresh")
					.title("High Tresh (mV)")
					.buildSingle();
	lmtOverloadWidgetLayout->addWidget(highThresh, 1, 0);

	// adi,gc-lmt-overload-low-thresh
	IIOWidget *lowThresh = IIOWidgetBuilder(lmtOverloadWidget)
				       .device(m_device)
				       .attribute("adi,gc-lmt-overload-low-thresh")
				       .title("Small Thresh")
				       .buildSingle();
	lmtOverloadWidgetLayout->addWidget(lowThresh, 1, 1);

	// adi,agc-lmt-overload-large-exceed-counter
	IIOWidget *largeExced = IIOWidgetBuilder(lmtOverloadWidget)
					.device(m_device)
					.attribute("adi,agc-lmt-overload-large-exceed-counter")
					.title("Large Exceed Cntr")
					.buildSingle();
	lmtOverloadWidgetLayout->addWidget(largeExced, 2, 0);

	// adi,agc-lmt-overload-small-exceed-counter
	IIOWidget *smallExced = IIOWidgetBuilder(lmtOverloadWidget)
					.device(m_device)
					.attribute("adi,agc-lmt-overload-small-exceed-counter")
					.title("Small Exceed Cntr")
					.buildSingle();
	lmtOverloadWidgetLayout->addWidget(smallExced, 2, 1);

	// adi,agc-lmt-overload-large-inc-steps
	IIOWidget *largeInc = IIOWidgetBuilder(lmtOverloadWidget)
				      .device(m_device)
				      .attribute("adi,agc-lmt-overload-large-inc-steps")
				      .title("Large Decr. Steps")
				      .buildSingle();
	lmtOverloadWidgetLayout->addWidget(largeInc, 3, 0);

	// adi,agc-immed-gain-change-if-large-lmt-overload-enable
	IIOWidget *immedGain = IIOWidgetBuilder(lmtOverloadWidget)
				       .device(m_device)
				       .attribute("adi,agc-immed-gain-change-if-large-lmt-overload-enable")
				       .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
				       .title("Immed Gain change if Large Overload")
				       .buildSingle();
	lmtOverloadWidgetLayout->addWidget(immedGain, 3, 1);

	return lmtOverloadWidget;
}

QWidget *GainWidget::digitalGainWidget(QWidget *parent)
{
	QWidget *digitalGainWidget = new QWidget(parent);
	QGridLayout *digitalGainWidgetLayout = new QGridLayout(digitalGainWidget);
	digitalGainWidget->setLayout(digitalGainWidgetLayout);

	Style::setStyle(digitalGainWidget, style::properties::widget::border_interactive);
	Style::setBackgroundColor(digitalGainWidget, json::theme::background_primary);

	digitalGainWidgetLayout->addWidget(new QLabel("Digital Gain", digitalGainWidget), 0, 0);

	// adi,gc-dig-gain-enable
	IIOWidget *gainMode = IIOWidgetBuilder(digitalGainWidget)
				      .device(m_device)
				      .attribute("adi,gc-dig-gain-enable")
				      .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
				      .title("Dig Gain Enable")
				      .buildSingle();
	digitalGainWidgetLayout->addWidget(gainMode, 1, 0);

	// adi,agc-dig-saturation-exceed-counter
	IIOWidget *saturationExceedCounter = IIOWidgetBuilder(digitalGainWidget)
						     .device(m_device)
						     .attribute("adi,agc-dig-saturation-exceed-counter")
						     .uiStrategy(IIOWidgetBuilder::RangeUi)
						     .title("Saturation Exceed Counter")
						     .buildSingle();
	digitalGainWidgetLayout->addWidget(saturationExceedCounter, 2, 0);

	// adi,gc-max-dig-gain
	IIOWidget *maxDigGain = IIOWidgetBuilder(digitalGainWidget)
					.device(m_device)
					.attribute("adi,gc-max-dig-gain")
					.uiStrategy(IIOWidgetBuilder::RangeUi)
					.title("Max Digital Gain")
					.buildSingle();
	digitalGainWidgetLayout->addWidget(maxDigGain, 1, 1);

	// adi,agc-dig-gain-step-size
	IIOWidget *gainStepSize = IIOWidgetBuilder(digitalGainWidget)
					  .device(m_device)
					  .attribute("adi,agc-dig-gain-step-size")
					  .uiStrategy(IIOWidgetBuilder::RangeUi)
					  .title("Decr Step Size")
					  .buildSingle();
	digitalGainWidgetLayout->addWidget(gainStepSize, 2, 1);

	return digitalGainWidget;
}

QWidget *GainWidget::fastAttackAGCWidget(QWidget *parent)
{
	QWidget *fastAttackAGCWidget = new QWidget(parent);
	QGridLayout *layout = new QGridLayout(fastAttackAGCWidget);
	fastAttackAGCWidget->setLayout(layout);

	Style::setStyle(fastAttackAGCWidget, style::properties::widget::border_interactive);
	Style::setBackgroundColor(fastAttackAGCWidget, json::theme::background_primary);

	layout->addWidget(new QLabel("Fast Attack AGC", fastAttackAGCWidget));

	layout->addWidget(stateResponseToPeakOverload(fastAttackAGCWidget));
	layout->addWidget(lowPowerCheck(fastAttackAGCWidget));
	layout->addWidget(agcLockLevelAdjustment(fastAttackAGCWidget));
	layout->addWidget(peakDetectors(fastAttackAGCWidget));
	layout->addWidget(gainUnlock(fastAttackAGCWidget));

	return fastAttackAGCWidget;
}

QWidget *GainWidget::stateResponseToPeakOverload(QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	QVBoxLayout *layout = new QVBoxLayout(widget);
	widget->setLayout(layout);

	Style::setStyle(widget, style::properties::widget::border_interactive);

	layout->addWidget(new QLabel("State 1 Response to Peak Overload", widget));

	// adi,fagc-state-wait-time-ns
	IIOWidget *stateWaitTime = IIOWidgetBuilder(widget)
					   .device(m_device)
					   .attribute("adi,fagc-state-wait-time-ns")
					   .uiStrategy(IIOWidgetBuilder::RangeUi)
					   .title("State Wati Time")
					   .buildSingle();
	layout->addWidget(stateWaitTime);

	return widget;
}

QWidget *GainWidget::lowPowerCheck(QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	QGridLayout *layout = new QGridLayout(widget);
	widget->setLayout(layout);

	Style::setStyle(widget, style::properties::widget::border_interactive);

	layout->addWidget(new QLabel("Low Power Check", widget), 0, 0);

	// adi,fagc-allow-agc-gain-increase-enable
	IIOWidget *allowGainIncrease = IIOWidgetBuilder(widget)
					       .device(m_device)
					       .attribute("adi,fagc-allow-agc-gain-increase-enable")
					       .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
					       .title("Allow AGC to increase Gain if signal below Low PWR Thresh")
					       .buildSingle();
	layout->addWidget(allowGainIncrease, 1, 0);

	// adi,gc-low-power-thresh
	IIOWidget *lowPowerThresh = IIOWidgetBuilder(widget)
					    .device(m_device)
					    .attribute("adi,gc-low-power-thresh")
					    .uiStrategy(IIOWidgetBuilder::RangeUi)
					    .title("Low PWR Thresh (dbFS")
					    .buildSingle();
	layout->addWidget(lowPowerThresh, 1, 1);

	// adi,fagc-lp-thresh-increment-time
	IIOWidget *lowPowerThreshIncrementTime = IIOWidgetBuilder(widget)
							 .device(m_device)
							 .attribute("adi,fagc-lp-thresh-increment-time")
							 .uiStrategy(IIOWidgetBuilder::RangeUi)
							 .title("Low PWR THresh Increment Time")
							 .buildSingle();
	layout->addWidget(lowPowerThreshIncrementTime, 2, 0);

	// adi,fagc-lp-thresh-increment-steps
	IIOWidget *lowPowerThreshIncrementSteps = IIOWidgetBuilder(widget)
							  .device(m_device)
							  .attribute("adi,fagc-lp-thresh-increment-steps")
							  .uiStrategy(IIOWidgetBuilder::RangeUi)
							  .title("Increment Step")
							  .buildSingle();
	layout->addWidget(lowPowerThreshIncrementSteps, 2, 1);

	return widget;
}

QWidget *GainWidget::agcLockLevelAdjustment(QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	QGridLayout *layout = new QGridLayout(widget);
	widget->setLayout(layout);

	Style::setStyle(widget, style::properties::widget::border_interactive);

	layout->addWidget(new QLabel("AGC Lock Level Adjustment", widget), 0, 0);

	// adi,agc-inner-thresh-high
	IIOWidget *innerThreshHigh = IIOWidgetBuilder(widget)
					     .device(m_device)
					     .attribute("adi,agc-inner-thresh-high")
					     .uiStrategy(IIOWidgetBuilder::RangeUi)
					     .title("AGC Lock Level (dBFS) (Inner Threshold High)")
					     .buildSingle();
	layout->addWidget(innerThreshHigh, 1, 0);

	// adi,fagc-lock-level-gain-increase-upper-limit
	IIOWidget *lockLevelGain = IIOWidgetBuilder(widget)
					   .device(m_device)
					   .attribute("adi,fagc-lock-level-gain-increase-upper-limit")
					   .uiStrategy(IIOWidgetBuilder::RangeUi)
					   .title("Lock Lvel Gain Increase Upper Limit")
					   .buildSingle();
	layout->addWidget(lockLevelGain, 2, 0);

	// adi,fagc-lock-level-lmt-gain-increase-enable
	IIOWidget *lockLevelLmtGain = IIOWidgetBuilder(widget)
					      .device(m_device)
					      .attribute("adi,fagc-lock-level-lmt-gain-increase-enable")
					      .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
					      .title("Allow LMT Gain Increase")
					      .buildSingle();
	layout->addWidget(lockLevelLmtGain, 2, 1);

	return widget;
}

QWidget *GainWidget::peakDetectors(QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	QGridLayout *layout = new QGridLayout(widget);
	widget->setLayout(layout);

	Style::setStyle(widget, style::properties::widget::border_interactive);

	layout->addWidget(new QLabel("Peak Detectors (again), Final Low Power Test and Settling", widget));

	// adi,fagc-lpf-final-settling-steps
	IIOWidget *lpfFinalSettingsSteps = IIOWidgetBuilder(widget)
						   .device(m_device)
						   .attribute("adi,fagc-lpf-final-settling-steps")
						   .uiStrategy(IIOWidgetBuilder::RangeUi)
						   .title("LPF / Full Table Final, Final Settling Steps")
						   .buildSingle();
	layout->addWidget(lpfFinalSettingsSteps, 1, 0);

	// adi,fagc-lmt-final-settling-steps
	IIOWidget *lmtFinalSettingsSteps = IIOWidgetBuilder(widget)
						   .device(m_device)
						   .attribute("adi,fagc-lmt-final-settling-steps")
						   .uiStrategy(IIOWidgetBuilder::RangeUi)
						   .title("LMT Final Settling Steps")
						   .buildSingle();
	layout->addWidget(lmtFinalSettingsSteps, 1, 1);

	// adi,fagc-final-overrange-count
	IIOWidget *finalOverrange = IIOWidgetBuilder(widget)
					    .device(m_device)
					    .attribute("adi,fagc-final-overrange-count")
					    .uiStrategy(IIOWidgetBuilder::RangeUi)
					    .title("Final Overrange Count")
					    .buildSingle();
	layout->addWidget(finalOverrange, 2, 0);

	// adi,fagc-gain-increase-after-gain-lock-enable
	IIOWidget *increaseAfterGainLock = IIOWidgetBuilder(widget)
						   .device(m_device)
						   .attribute("adi,fagc-gain-increase-after-gain-lock-enable")
						   .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
						   .title("Low Power Test Gain Increase after Gain Lock")
						   .buildSingle();
	layout->addWidget(increaseAfterGainLock, 2, 1);

	return widget;
}

QWidget *GainWidget::gainUnlock(QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	QGridLayout *layout = new QGridLayout(widget);
	widget->setLayout(layout);

	Style::setStyle(widget, style::properties::widget::border_interactive);

	layout->addWidget(new QLabel("Gain Unlock", widget));

	// adi,fagc-gain-index-type-after-exit-rx-mode
	IIOWidget *gainIndexType = IIOWidgetBuilder(widget)
					   .device(m_device)
					   .attribute("adi,fagc-gain-index-type-after-exit-rx-mode")
					   .uiStrategy(IIOWidgetBuilder::ComboUi)
					   .title("When exit Receive State Restart Gain Lock Algorithm (GLA) and goto")
					   .buildSingle();
	layout->addWidget(gainIndexType, 1, 0);

	// adi,fagc-use-last-lock-level-for-set-gain-enable
	IIOWidget *increaseAfterGainLock = IIOWidgetBuilder(widget)
						   .device(m_device)
						   .attribute("adi,fagc-use-last-lock-level-for-set-gain-enable")
						   .uiStrategy(IIOWidgetBuilder::ComboUi)
						   .title("SET Gain use Gain from")
						   .buildSingle();
	layout->addWidget(increaseAfterGainLock, 2, 0);

	// adi,fagc-optimized-gain-offset
	IIOWidget *optimiezedGainOffset = IIOWidgetBuilder(widget)
						  .device(m_device)
						  .attribute("adi,fagc-optimized-gain-offset")
						  .uiStrategy(IIOWidgetBuilder::RangeUi)
						  .title("OPTIMIZED Gain Offset")
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

	// adi,fagc-rst-gla-stronger-sig-thresh-above-ll
	IIOWidget *fagcRstGlaStrongerSigThresh = IIOWidgetBuilder(widget)
							 .device(m_device)
							 .attribute("adi,fagc-rst-gla-stronger-sig-thresh-above-ll")
							 .uiStrategy(IIOWidgetBuilder::RangeUi)
							 .title(" ")
							 .buildSingle();
	layout->addWidget(fagcRstGlaStrongerSigThresh, 3, 1);

	// adi,fagc-energy-lost-stronger-sig-gain-lock-exit-cnt
	IIOWidget *energyLostStronger = IIOWidgetBuilder(widget)
						.device(m_device)
						.attribute("adi,fagc-energy-lost-stronger-sig-gain-lock-exit-cnt")
						.uiStrategy(IIOWidgetBuilder::RangeUi)
						.title("for (RX Samples) restart GLA")
						.buildSingle();
	layout->addWidget(energyLostStronger, 3, 2);

	// adi,fagc-rst-gla-engergy-lost-sig-thresh-exceeded-enable
	IIOWidget *rstGlaEngLostSigThreshExceedEn =
		IIOWidgetBuilder(widget)
			.device(m_device)
			.attribute("adi,fagc-rst-gla-engergy-lost-sig-thresh-exceeded-enable")
			.uiStrategy(IIOWidgetBuilder::CheckBoxUi)
			.title("If Signal Power decreases below Lock Level by (dBFS)")
			.buildSingle();
	layout->addWidget(rstGlaEngLostSigThreshExceedEn, 4, 0);

	// adi,fagc-rst-gla-engergy-lost-sig-thresh-below-ll
	IIOWidget *rstGlaEnergyLost = IIOWidgetBuilder(widget)
					      .device(m_device)
					      .attribute("adi,fagc-rst-gla-engergy-lost-sig-thresh-below-ll")
					      .uiStrategy(IIOWidgetBuilder::RangeUi)
					      .title(" ")
					      .buildSingle();
	layout->addWidget(rstGlaEnergyLost, 4, 1);

	// adi,fagc-rst-gla-large-lmt-overload-enable
	IIOWidget *rstGlaLargeLmtOverloadEn = IIOWidgetBuilder(widget)
						      .device(m_device)
						      .attribute("adi,fagc-rst-gla-large-lmt-overload-enable")
						      .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
						      .title("If Large LMT Overload do not change Gain but restart GLA")
						      .buildSingle();
	layout->addWidget(rstGlaLargeLmtOverloadEn, 5, 0);

	// adi,fagc-rst-gla-engergy-lost-goto-optim-gain-enable
	IIOWidget *rstGlaEnergyLostGotoOptimGainEn =
		IIOWidgetBuilder(widget)
			.device(m_device)
			.attribute("adi,fagc-rst-gla-engergy-lost-goto-optim-gain-enable")
			.uiStrategy(IIOWidgetBuilder::ComboUi)
			.title("... and GTO")
			.buildSingle();
	layout->addWidget(rstGlaEnergyLostGotoOptimGainEn, 5, 2);

	// adi,fagc-rst-gla-large-adc-overload-enable
	IIOWidget *glaLargeAdcOverloadEn = IIOWidgetBuilder(widget)
						   .device(m_device)
						   .attribute("adi,fagc-rst-gla-large-adc-overload-enable")
						   .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
						   .title("If Large ADC Overload do not change Gain but restart GLA")
						   .buildSingle();
	layout->addWidget(glaLargeAdcOverloadEn, 6, 0);

	// adi,fagc-rst-gla-en-agc-pulled-high-enable
	IIOWidget *rstGlaEnAgc = IIOWidgetBuilder(widget)
					 .device(m_device)
					 .attribute("adi,fagc-gain-increase-after-gain-lock-enable")
					 .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
					 .title("If EN_AGC is pulled high restart GLA and goto")
					 .buildSingle();
	layout->addWidget(rstGlaEnAgc, 7, 0);

	// adi,fagc-rst-gla-if-en-agc-pulled-high-mode
	IIOWidget *rstGlaIfEnAgcPulled = IIOWidgetBuilder(widget)
						 .device(m_device)
						 .attribute("adi,fagc-rst-gla-if-en-agc-pulled-high-mode")
						 .title(" ")
						 .buildSingle();
	layout->addWidget(rstGlaIfEnAgcPulled, 7, 1);

	// adi,fagc-power-measurement-duration-in-state5
	IIOWidget *test = IIOWidgetBuilder(widget)
				  .device(m_device)
				  .attribute("adi,fagc-power-measurement-duration-in-state5")
				  .uiStrategy(IIOWidgetBuilder::RangeUi)
				  .title("When testing signal power against the thresholds above, measure power for "
					 "(sample periods)")
				  .buildSingle();
	layout->addWidget(test, 8, 0);

	return widget;
}

QWidget *GainWidget::miscWidget(QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	QVBoxLayout *layout = new QVBoxLayout(widget);
	widget->setLayout(layout);

	Style::setStyle(widget, style::properties::widget::border_interactive);
	Style::setBackgroundColor(widget, json::theme::background_primary);

	layout->addWidget(new QLabel("MISC", widget));

	// adi,fagc-dec-pow-measurement-duration
	IIOWidget *stateWaitTime = IIOWidgetBuilder(widget)
					   .device(m_device)
					   .attribute("adi,fagc-dec-pow-measurement-duration")
					   .uiStrategy(IIOWidgetBuilder::RangeUi)
					   .title("Dec PWR mess. duration in fast attack mode")
					   .buildSingle();
	layout->addWidget(stateWaitTime);

	return widget;
}
