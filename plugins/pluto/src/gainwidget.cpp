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

	modeWidgetLayout->addWidget(new QLabel("Mode", modeWidget), 0, 0);

	// adi,gc-rx1-mode

	// adi,gc-rx2-mode



	//

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

	//adi,mgc-rx1-ctrl-inp-enable
	IIOWidget *mgcRx1Ctrl =
	        IIOWidgetBuilder(mgcWidget)
	                .device(m_device)
	                .attribute("adi,mgc-rx1-ctrl-inp-enable")
	                .buildSingle();
	mgcWidgetLayout->addWidget(mgcRx1Ctrl, 1, 0);

	//adi,mgc-rx2-ctrl-inp-enable
	IIOWidget *mgcRx2Ctrl =
	        IIOWidgetBuilder(mgcWidget)
	                .device(m_device)
	                .attribute("adi,mgc-rx2-ctrl-inp-enable")
	                .buildSingle();
	mgcWidgetLayout->addWidget(mgcRx2Ctrl, 1, 1);

	// adi,mgc-inc-gain-step
	IIOWidget *mgcIncGain =
	        IIOWidgetBuilder(mgcWidget)
	                .device(m_device)
	                .attribute("adi,mgc-inc-gain-step")
	                .uiStrategy(IIOWidgetBuilder::RangeUi)
	                .buildSingle();
	mgcWidgetLayout->addWidget(mgcIncGain, 2, 0);

	//adi,mgc-dec-gain-step
	IIOWidget *mgcDecGain =
	        IIOWidgetBuilder(mgcWidget)
	                .device(m_device)
	                .attribute("adi,mgc-inc-gain-step")
	                .uiStrategy(IIOWidgetBuilder::RangeUi)
	                .buildSingle();
	mgcWidgetLayout->addWidget(mgcDecGain, 2, 1);

	// adi,mgc-split-table-ctrl-inp-gain-mode
	IIOWidget *mgcSplitTableCtrl =
	        IIOWidgetBuilder(mgcWidget)
	                .device(m_device)
	                .attribute("adi,mgc-inc-gain-step")
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

	agcTresholdGainChangesWidgetLayout->addWidget(new QLabel("AGC Thresholds and Gain Changes", agcTresholdGainChangesWidget), 0, 0);

	// adi,agc-outer-thresh-high
	IIOWidget *outerThreshHigh =
	        IIOWidgetBuilder(agcTresholdGainChangesWidget)
	                .device(m_device)
	                .attribute("adi,agc-outer-thresh-high")
	                .uiStrategy(IIOWidgetBuilder::RangeUi)
	                .buildSingle();
	agcTresholdGainChangesWidgetLayout->addWidget(outerThreshHigh, 1, 0);

	// adi,agc-outer-thresh-high-dec-steps
	IIOWidget *outerThreshHighDecSteps =
	        IIOWidgetBuilder(agcTresholdGainChangesWidget)
	                .device(m_device)
	                .attribute("adi,agc-outer-thresh-high-dec-steps")
	                .uiStrategy(IIOWidgetBuilder::RangeUi)
	                .buildSingle();
	agcTresholdGainChangesWidgetLayout->addWidget(outerThreshHighDecSteps, 1, 1);

	// adi,agc-inner-thresh-high
	IIOWidget *innerThreshHigh =
	        IIOWidgetBuilder(agcTresholdGainChangesWidget)
	                .device(m_device)
	                .attribute("adi,agc-inner-thresh-high")
	                .uiStrategy(IIOWidgetBuilder::RangeUi)
	                .buildSingle();
	agcTresholdGainChangesWidgetLayout->addWidget(innerThreshHigh, 2, 0);

	// adi,agc-inner-thresh-high-dec-steps
	IIOWidget *innerThreshHighDecSteps =
	        IIOWidgetBuilder(agcTresholdGainChangesWidget)
	                .device(m_device)
	                .attribute("adi,agc-inner-thresh-high-dec-steps")
	                .uiStrategy(IIOWidgetBuilder::RangeUi)
	                .buildSingle();
	agcTresholdGainChangesWidgetLayout->addWidget(innerThreshHighDecSteps, 2, 1);

	// adi,agc-inner-thresh-low
	IIOWidget *innerThreshLow =
	        IIOWidgetBuilder(agcTresholdGainChangesWidget)
	                .device(m_device)
	                .attribute("adi,agc-inner-thresh-low")
	                .uiStrategy(IIOWidgetBuilder::RangeUi)
	                .buildSingle();
	agcTresholdGainChangesWidgetLayout->addWidget(innerThreshLow, 3, 0);

	// adi,agc-inner-thresh-low-inc-steps
	IIOWidget *innerThreshLowIncSteps =
	        IIOWidgetBuilder(agcTresholdGainChangesWidget)
	                .device(m_device)
	                .attribute("adi,agc-inner-thresh-low-inc-steps")
	                .uiStrategy(IIOWidgetBuilder::RangeUi)
	                .buildSingle();
	agcTresholdGainChangesWidgetLayout->addWidget(innerThreshLowIncSteps, 3, 1);

	// adi,agc-outer-thresh-low
	IIOWidget *outerThreshLow =
	        IIOWidgetBuilder(agcTresholdGainChangesWidget)
	                .device(m_device)
	                .attribute("adi,agc-outer-thresh-low")
	                .uiStrategy(IIOWidgetBuilder::RangeUi)
	                .buildSingle();
	agcTresholdGainChangesWidgetLayout->addWidget(outerThreshLow, 4, 0);

	// adi,agc-outer-thresh-low-inc-steps
	IIOWidget *outerThreshLowIncSteps =
	        IIOWidgetBuilder(agcTresholdGainChangesWidget)
	                .device(m_device)
	                .attribute("adi,agc-outer-thresh-low-inc-steps")
	                .uiStrategy(IIOWidgetBuilder::RangeUi)
	                .buildSingle();
	agcTresholdGainChangesWidgetLayout->addWidget(outerThreshLowIncSteps, 4, 1);

	// adi,agc-sync-for-gain-counter-enable
	IIOWidget *sync =
	        IIOWidgetBuilder(agcTresholdGainChangesWidget)
	                .device(m_device)
	                .attribute("adi,agc-sync-for-gain-counter-enable")
	                .buildSingle();
	agcTresholdGainChangesWidgetLayout->addWidget(sync, 5, 0);

	// adi,agc-gain-update-interval-us
	IIOWidget *gainUpdateInterval =
	        IIOWidgetBuilder(agcTresholdGainChangesWidget)
	                .device(m_device)
	                .attribute("adi,agc-gain-update-interval-us")
	                .uiStrategy(IIOWidgetBuilder::RangeUi)
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
	IIOWidget *largeOverloadThresh =
	        IIOWidgetBuilder(adcOverloadWidget)
	                .device(m_device)
	                .attribute("adi,gc-adc-large-overload-thresh")
	                .uiStrategy(IIOWidgetBuilder::RangeUi)
	                .buildSingle();
	adcOverloadWidgetLayout->addWidget(largeOverloadThresh, 1, 0);

	// adi,gc-adc-small-overload-thresh
	IIOWidget *smallOverloadThresh =
	        IIOWidgetBuilder(adcOverloadWidget)
	                .device(m_device)
	                .attribute("adi,gc-adc-small-overload-thresh")
	                .uiStrategy(IIOWidgetBuilder::RangeUi)
	                .buildSingle();
	adcOverloadWidgetLayout->addWidget(smallOverloadThresh, 1, 1);

	// adi,agc-adc-large-overload-exceed-counter
	IIOWidget *largeOverloadExceed =
	        IIOWidgetBuilder(adcOverloadWidget)
	                .device(m_device)
	                .attribute("adi,agc-adc-large-overload-exceed-counter")
	                .uiStrategy(IIOWidgetBuilder::RangeUi)
	                .buildSingle();
	adcOverloadWidgetLayout->addWidget(largeOverloadExceed, 2, 0);

	// adi,agc-adc-small-overload-exceed-counter
	IIOWidget *smallOverloadExceed =
	        IIOWidgetBuilder(adcOverloadWidget)
	                .device(m_device)
	                .attribute("adi,agc-adc-small-overload-exceed-counter")
	                .uiStrategy(IIOWidgetBuilder::RangeUi)
	                .buildSingle();
	adcOverloadWidgetLayout->addWidget(smallOverloadExceed, 2, 1);

	// adi,agc-adc-large-overload-inc-steps
	IIOWidget *largeDecSteps =
	        IIOWidgetBuilder(adcOverloadWidget)
	                .device(m_device)
	                .attribute("adi,agc-adc-large-overload-inc-steps")
	                .uiStrategy(IIOWidgetBuilder::RangeUi)
	                .buildSingle();
	adcOverloadWidgetLayout->addWidget(largeDecSteps, 3, 0);

	//adi,agc-adc-lmt-small-overload-prevent-gain-inc-enable
	IIOWidget *prevertGainIncrease =
	        IIOWidgetBuilder(adcOverloadWidget)
	                .device(m_device)
	                .attribute("adi,agc-adc-lmt-small-overload-prevent-gain-inc-enable")
	                .uiStrategy(IIOWidgetBuilder::RangeUi)
	                .buildSingle();
	adcOverloadWidgetLayout->addWidget(prevertGainIncrease, 3, 1);


	// sum n samples TODO FIND ATTR

	// adi,agc-immed-gain-change-if-large-adc-overload-enable
	IIOWidget *immedGainChange =
	        IIOWidgetBuilder(adcOverloadWidget)
	                .device(m_device)
	                .attribute("adi,agc-immed-gain-change-if-large-adc-overload-enable")
	                .uiStrategy(IIOWidgetBuilder::RangeUi)
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
	IIOWidget *highThresh =
	        IIOWidgetBuilder(lmtOverloadWidget)
	                .device(m_device)
	                .attribute("adi,gc-lmt-overload-high-thresh")
	                .buildSingle();
	lmtOverloadWidgetLayout->addWidget(highThresh, 1, 0);

	// adi,gc-lmt-overload-low-thresh
	IIOWidget *lowThresh =
	        IIOWidgetBuilder(lmtOverloadWidget)
	                .device(m_device)
	                .attribute("adi,gc-lmt-overload-low-thresh")
	                .buildSingle();
	lmtOverloadWidgetLayout->addWidget(lowThresh, 1, 1);

	//adi,agc-lmt-overload-large-exceed-counter
	IIOWidget *largeExced =
	        IIOWidgetBuilder(lmtOverloadWidget)
	                .device(m_device)
	                .attribute("adi,agc-lmt-overload-large-exceed-counter")
	                .buildSingle();
	lmtOverloadWidgetLayout->addWidget(largeExced, 2, 0);

	// adi,agc-lmt-overload-small-exceed-counter
	IIOWidget *smallExced =
	        IIOWidgetBuilder(lmtOverloadWidget)
	                .device(m_device)
	                .attribute("adi,agc-lmt-overload-small-exceed-counter")
	                .buildSingle();
	lmtOverloadWidgetLayout->addWidget(smallExced, 2, 1);

	// adi,agc-lmt-overload-large-inc-steps
	IIOWidget *largeInc =
	        IIOWidgetBuilder(lmtOverloadWidget)
	                .device(m_device)
	                .attribute("adi,agc-lmt-overload-large-inc-steps")
	                .buildSingle();
	lmtOverloadWidgetLayout->addWidget(largeInc, 3, 0);

	// adi,agc-immed-gain-change-if-large-lmt-overload-enable
	IIOWidget *immedGain =
	        IIOWidgetBuilder(lmtOverloadWidget)
	                .device(m_device)
	                .attribute("adi,agc-immed-gain-change-if-large-lmt-overload-enable")
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
	IIOWidget *gainMode =
	        IIOWidgetBuilder(digitalGainWidget)
	                .device(m_device)
	                .attribute("adi,gc-dig-gain-enable")
	                .buildSingle();
	digitalGainWidgetLayout->addWidget(gainMode, 1, 0);

	// adi,agc-dig-saturation-exceed-counter
	IIOWidget *saturationExceedCounter =
	        IIOWidgetBuilder(digitalGainWidget)
	                .device(m_device)
	                .attribute("adi,agc-dig-saturation-exceed-counter")
	                .uiStrategy(IIOWidgetBuilder::RangeUi)
	                .buildSingle();
	digitalGainWidgetLayout->addWidget(saturationExceedCounter, 2, 0);

	// adi,gc-max-dig-gain
	IIOWidget *maxDigGain =
	        IIOWidgetBuilder(digitalGainWidget)
	                .device(m_device)
	                .attribute("adi,gc-max-dig-gain")
	                .uiStrategy(IIOWidgetBuilder::RangeUi)
	                .buildSingle();
	digitalGainWidgetLayout->addWidget(maxDigGain, 1, 1);


	// adi,agc-dig-gain-step-size
	IIOWidget *gainStepSize =
	        IIOWidgetBuilder(digitalGainWidget)
	                .device(m_device)
	                .attribute("adi,agc-dig-gain-step-size")
	                .uiStrategy(IIOWidgetBuilder::RangeUi)
	                .buildSingle();
	digitalGainWidgetLayout->addWidget(gainStepSize, 2, 1);

	return digitalGainWidget;
}

QWidget *GainWidget::fastAttackAGCWidget(QWidget *parent)
{
	QWidget *fastAttackAGCWidget = new QWidget(parent);
	QGridLayout *fastAttackAGCWidgetLayout = new QGridLayout(fastAttackAGCWidget);
	fastAttackAGCWidget->setLayout(fastAttackAGCWidgetLayout);

	Style::setStyle(fastAttackAGCWidget, style::properties::widget::border_interactive);
	Style::setBackgroundColor(fastAttackAGCWidget, json::theme::background_primary);

	fastAttackAGCWidgetLayout->addWidget(new QLabel("Fast Attack AGC", fastAttackAGCWidget), 0, 0);

	// adi,fagc-state-wait-time-ns
	IIOWidget *stateWaitTime =
	        IIOWidgetBuilder(fastAttackAGCWidget)
	                .device(m_device)
	                .attribute("adi,fagc-state-wait-time-ns")
	                .uiStrategy(IIOWidgetBuilder::RangeUi)
	                .buildSingle();
	fastAttackAGCWidgetLayout->addWidget(stateWaitTime, 1, 0);

	////////////////////////////  low power check   //////////////////////////////

	// adi,fagc-allow-agc-gain-increase-enable
	IIOWidget *allowGainIncrease =
	        IIOWidgetBuilder(fastAttackAGCWidget)
	                .device(m_device)
	                .attribute("adi,fagc-allow-agc-gain-increase-enable")
	                .buildSingle();
	fastAttackAGCWidgetLayout->addWidget(allowGainIncrease, 2, 0);

	// adi,gc-low-power-thresh
	IIOWidget *lowPowerThresh =
	        IIOWidgetBuilder(fastAttackAGCWidget)
	                .device(m_device)
	                .attribute("adi,gc-low-power-thresh")
	                .uiStrategy(IIOWidgetBuilder::RangeUi)
	                .buildSingle();
	fastAttackAGCWidgetLayout->addWidget(lowPowerThresh, 2, 1);

	// adi,fagc-lp-thresh-increment-time
	IIOWidget *lowPowerThreshIncrementTime =
	        IIOWidgetBuilder(fastAttackAGCWidget)
	                .device(m_device)
	                .attribute("adi,fagc-lp-thresh-increment-time")
	                .uiStrategy(IIOWidgetBuilder::RangeUi)
	                .buildSingle();
	fastAttackAGCWidgetLayout->addWidget(lowPowerThreshIncrementTime, 3, 0);

	// adi,fagc-lp-thresh-increment-steps
	IIOWidget *lowPowerThreshIncrementSteps =
	        IIOWidgetBuilder(fastAttackAGCWidget)
	                .device(m_device)
	                .attribute("adi,fagc-lp-thresh-increment-steps")
	                .uiStrategy(IIOWidgetBuilder::RangeUi)
	                .buildSingle();
	fastAttackAGCWidgetLayout->addWidget(lowPowerThreshIncrementSteps, 3, 1);

	//////////////////////////// end of low power check   //////////////////////////////

	/////////////////////////// AGC Lock Level Adjustment /////////////////////////////

	// adi,agc-inner-thresh-high
	IIOWidget *innerThreshHigh =
	        IIOWidgetBuilder(fastAttackAGCWidget)
	                .device(m_device)
	                .attribute("adi,agc-inner-thresh-high")
	                .uiStrategy(IIOWidgetBuilder::RangeUi)
	                .buildSingle();
	fastAttackAGCWidgetLayout->addWidget(innerThreshHigh, 4, 0);

	// adi,fagc-lock-level-gain-increase-upper-limit
	IIOWidget *lockLevelGain =
	        IIOWidgetBuilder(fastAttackAGCWidget)
	                .device(m_device)
	                .attribute("adi,fagc-lock-level-gain-increase-upper-limit")
	                .uiStrategy(IIOWidgetBuilder::RangeUi)
	                .buildSingle();
	fastAttackAGCWidgetLayout->addWidget(lockLevelGain, 5, 0);

	// adi,fagc-lock-level-lmt-gain-increase-enable
	IIOWidget *lockLevelLmtGain =
	        IIOWidgetBuilder(fastAttackAGCWidget)
	                .device(m_device)
	                .attribute("adi,fagc-lock-level-lmt-gain-increase-enable")
	                .buildSingle();
	fastAttackAGCWidgetLayout->addWidget(lockLevelLmtGain, 5, 1);

	/////////////////////////// end of  AGC Lock Level Adjustment /////////////////////////////

	/////////////////////////// peak detectors /////////////////////////////

	// adi,fagc-lpf-final-settling-steps
	IIOWidget *lpfFinalSettingsSteps =
	        IIOWidgetBuilder(fastAttackAGCWidget)
	                .device(m_device)
	                .attribute("adi,fagc-lpf-final-settling-steps")
	                .uiStrategy(IIOWidgetBuilder::RangeUi)
	                .buildSingle();
	fastAttackAGCWidgetLayout->addWidget(lpfFinalSettingsSteps, 6, 0);

	// adi,fagc-lmt-final-settling-steps
	IIOWidget *lmtFinalSettingsSteps =
	        IIOWidgetBuilder(fastAttackAGCWidget)
	                .device(m_device)
	                .attribute("adi,fagc-lmt-final-settling-steps")
	                .uiStrategy(IIOWidgetBuilder::RangeUi)
	                .buildSingle();
	fastAttackAGCWidgetLayout->addWidget(lmtFinalSettingsSteps, 6, 1);

	// adi,fagc-final-overrange-count
	IIOWidget *finalOverrange =
	        IIOWidgetBuilder(fastAttackAGCWidget)
	                .device(m_device)
	                .attribute("adi,fagc-final-overrange-count")
	                .uiStrategy(IIOWidgetBuilder::RangeUi)
	                .buildSingle();
	fastAttackAGCWidgetLayout->addWidget(finalOverrange, 7, 0);

	// adi,fagc-gain-increase-after-gain-lock-enable
	IIOWidget *increaseAfterGainLock =
	        IIOWidgetBuilder(fastAttackAGCWidget)
	                .device(m_device)
	                .attribute("adi,fagc-gain-increase-after-gain-lock-enable")
	                .buildSingle();
	fastAttackAGCWidgetLayout->addWidget(increaseAfterGainLock, 7, 1);

	/////////////////////////// end of  peak detectors /////////////////////////////

	return fastAttackAGCWidget;
}
