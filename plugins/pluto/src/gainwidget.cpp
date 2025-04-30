#include "gainwidget.h"
#include <style.h>
#include <iioutil/connectionprovider.h>
#include <iiowidgetbuilder.h>
#include <iiowidgetutils.h>

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
	modeWidgetLayout->setSpacing(10);
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
	QMap<QString, QString> *rxModeOptions = new QMap<QString, QString>();
	rxModeOptions->insert("0", "Manual");
	rxModeOptions->insert("1", "Fast_Attack");
	rxModeOptions->insert("2", "Slow_Attack");
	rxModeOptions->insert("3", "Hybrid");

	auto rxModeValues = rxModeOptions->values();
	QString rxModeOptionsData = "";
	for(int i = 0; i < rxModeValues.size(); i++) {
		rxModeOptionsData += " " + rxModeValues.at(i);
	}
	IIOWidget *rx1Mode = IIOWidgetBuilder(modeWidget)
				     .device(m_device)
				     .attribute("adi,gc-rx1-mode")
				     .uiStrategy(IIOWidgetBuilder::ComboUi)
				     .optionsValues(rxModeOptionsData)
				     .title("RX1")
				     .infoMessage("RX1 Gain control operation: Manual gain (0); Fast attack AGC (1); "
						  "Slow attack AGC (2); "
						  "Hybrid AGC (3). See register 0x0FA, bits [D4], [D1:D0].")
				     .buildSingle();
	modeWidgetLayout->addWidget(rx1Mode, 1, 0);

	rx1Mode->setUItoDataConversion([this, rxModeOptions](QString data) {
		return IIOWidgetUtils::comboUiToDataConversionFunction(data, rxModeOptions);
	});
	rx1Mode->setDataToUIConversion([this, rxModeOptions](QString data) {
		return IIOWidgetUtils::comboDataToUiConversionFunction(data, rxModeOptions);
	});

	// adi,gc-rx2-mode
	IIOWidget *rx2Mode = IIOWidgetBuilder(modeWidget)
				     .device(m_device)
				     .attribute("adi,gc-rx2-mode")
				     .uiStrategy(IIOWidgetBuilder::ComboUi)
				     .optionsValues(rxModeOptionsData)
				     .title("RX2")
				     .infoMessage("RX2 Gain control operation: Manual gain (0); Fast attack AGC (1); "
						  "Slow attack AGC (2); "
						  "Hybrid AGC (3). See register 0x0FA, bits [D4], [D3:D2].")
				     .buildSingle();
	modeWidgetLayout->addWidget(rx2Mode, 1, 1);

	rx2Mode->setUItoDataConversion([this, rxModeOptions](QString data) {
		return IIOWidgetUtils::comboUiToDataConversionFunction(data, rxModeOptions);
	});
	rx2Mode->setDataToUIConversion([this, rxModeOptions](QString data) {
		return IIOWidgetUtils::comboDataToUiConversionFunction(data, rxModeOptions);
	});

	// adi,split-gain-table-mode-enable
	QMap<QString, QString> *tableModeOptions = new QMap<QString, QString>();
	tableModeOptions->insert("0", "Full_Gain_Table");
	tableModeOptions->insert("1", "Split_Gain_Table");

	auto tableModeValues = tableModeOptions->values();
	QString tableModeOptionsData = "";
	for(int i = 0; i < tableModeValues.size(); i++) {
		tableModeOptionsData += " " + tableModeValues.at(i);
	}
	IIOWidget *tableMode = IIOWidgetBuilder(modeWidget)
				       .device(m_device)
				       .attribute("adi,split-gain-table-mode-enable")
				       .uiStrategy(IIOWidgetBuilder::ComboUi)
				       .optionsValues(tableModeOptionsData)
				       .title("Table Mode")
				       .infoMessage("Enable Split Gain Table Mode - default Full Table")
				       .buildSingle();
	modeWidgetLayout->addWidget(tableMode, 1, 2);

	tableMode->setUItoDataConversion([this, tableModeOptions](QString data) {
		return IIOWidgetUtils::comboUiToDataConversionFunction(data, tableModeOptions);
	});
	tableMode->setDataToUIConversion([this, tableModeOptions](QString data) {
		return IIOWidgetUtils::comboDataToUiConversionFunction(data, tableModeOptions);
	});

	// adi,gc-dec-pow-measurement-duration
	IIOWidget *decpowMeasurement =
		IIOWidgetBuilder(modeWidget)
			.device(m_device)
			.attribute("adi,gc-dec-pow-measurement-duration")
			.uiStrategy(IIOWidgetBuilder::RangeUi)
			.optionsValues("[0 1 524288]")
			.title("Dec PWR mess. duration in MGC and Slow Attack AGC mode")
			.infoMessage("The power measurement duration used by the gain control algorithm. See register "
				     "0x15C, bits [D3:D0]. This data is processed by the driver.")
			.buildSingle();
	modeWidgetLayout->addWidget(decpowMeasurement, 2, 0);

	// adi,gc-low-power-thresh
	IIOWidget *lowPowerThresh =
		IIOWidgetBuilder(modeWidget)
			.device(m_device)
			.attribute("adi,gc-low-power-thresh")
			.uiStrategy(IIOWidgetBuilder::RangeUi)
			.optionsValues("[0 1 64]")
			.title("Low PWR Thresh (dbFS)")
			.infoMessage("This threshold is used by the fast AGC to determine if the gain should be "
				     "increased. It can also be used to trigger a CTRL_OUT signal transition in MGC "
				     "mode. See register 0x114, bits [D6:D0]. This data is processed by the driver.")
			.buildSingle();
	modeWidgetLayout->addWidget(lowPowerThresh, 3, 0);

	// adi,agc-attack-delay-extra-margin-us
	IIOWidget *attackDelay =
		IIOWidgetBuilder(modeWidget)
			.device(m_device)
			.attribute("adi,agc-attack-delay-extra-margin-us")
			.uiStrategy(IIOWidgetBuilder::RangeUi)
			.optionsValues("[0 1 20000]")
			.title("Attack Delay extra margin (us)")
			.infoMessage(
				"The AGC Attack Delay prevents the AGC from starting its algorithm until the receive "
				"path has settled. The delay counter starts when the ENSM enters the Rx state. See "
				"register 0x022, bits [D5:D0]. This data is processed by the driver.")
			.buildSingle();
	modeWidgetLayout->addWidget(attackDelay, 2, 1);

	// adi,gc-use-rx-fir-out-for-dec-pwr-meas-enable
	IIOWidget *rxFirOut =
		IIOWidgetBuilder(modeWidget)
			.device(m_device)
			.attribute("adi,gc-use-rx-fir-out-for-dec-pwr-meas-enable")
			.uiStrategy(IIOWidgetBuilder::CheckBoxUi)
			.title("Use Rx FIR output for Dec. PWR measurements")
			.infoMessage(
				"Set to use the RX FIR output for power measurements. Default/Clear to use the HB1 "
				"output. See register 0x15C, bits [D6].")
			.buildSingle();
	modeWidgetLayout->addWidget(rxFirOut, 3, 1);
	rxFirOut->showProgressBar(false);

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
	mgcWidgetLayout->setSpacing(10);
	mgcWidget->setLayout(mgcWidgetLayout);

	Style::setStyle(mgcWidget, style::properties::widget::border_interactive);
	Style::setBackgroundColor(mgcWidget, json::theme::background_primary);

	QLabel *title = new QLabel("MGC", mgcWidget);
	Style::setStyle(title, style::properties::label::menuBig);
	mgcWidgetLayout->addWidget(title, 0, 0);

	// adi,mgc-rx1-ctrl-inp-enable
	IIOWidget *mgcRx1Ctrl =
		IIOWidgetBuilder(mgcWidget)
			.device(m_device)
			.attribute("adi,mgc-rx1-ctrl-inp-enable")
			.uiStrategy(IIOWidgetBuilder::CheckBoxUi)
			.title("RX1 Control Input")
			.infoMessage(
				"If this attribute is clear, SPI writes change the RX1 gain. When this attribute is "
				"set, control input pins control the gain. See register 0x0FB, bit [D0].")
			.buildSingle();
	mgcWidgetLayout->addWidget(mgcRx1Ctrl, 1, 0);
	mgcRx1Ctrl->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	mgcRx1Ctrl->showProgressBar(false);

	// adi,mgc-rx2-ctrl-inp-enable
	IIOWidget *mgcRx2Ctrl =
		IIOWidgetBuilder(mgcWidget)
			.device(m_device)
			.attribute("adi,mgc-rx2-ctrl-inp-enable")
			.uiStrategy(IIOWidgetBuilder::CheckBoxUi)
			.title("RX2 Control Input")
			.infoMessage(
				"If this attribute is clear, SPI writes change the RX2 gain. When this attribute is "
				"set, control input pins control the gain. See register 0x0FB, bit [D1].")
			.buildSingle();
	mgcWidgetLayout->addWidget(mgcRx2Ctrl, 1, 1);
	mgcRx2Ctrl->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	mgcRx2Ctrl->showProgressBar(false);

	// adi,mgc-inc-gain-step
	IIOWidget *mgcIncGain =
		IIOWidgetBuilder(mgcWidget)
			.device(m_device)
			.attribute("adi,mgc-inc-gain-step")
			.uiStrategy(IIOWidgetBuilder::RangeUi)
			.optionsValues("[1 1 8]")
			.title("Increase Gain Step")
			.infoMessage(
				"This attribute applies if the CTRL_IN signals control gain. The gain index increases "
				"by this value when certain CTRL_IN signals transition high. See register 0x0FC, bits "
				"[D7:D5]. This data is processed by the driver.")
			.buildSingle();
	mgcWidgetLayout->addWidget(mgcIncGain, 2, 0);

	// adi,mgc-dec-gain-step
	IIOWidget *mgcDecGain =
		IIOWidgetBuilder(mgcWidget)
			.device(m_device)
			.attribute("adi,mgc-inc-gain-step")
			.uiStrategy(IIOWidgetBuilder::RangeUi)
			.optionsValues("[1 1 8]")
			.title("Decrease Gain Step")
			.infoMessage(
				"This attribute applies if the CTRL_IN signals control gain. The gain index decreases "
				"by this value when certain CTRL_IN signals transition high. See register 0x0FE, bits "
				"[D7:D5]. This data is processed by the driver.")
			.buildSingle();
	mgcWidgetLayout->addWidget(mgcDecGain, 2, 1);

	// adi,mgc-split-table-ctrl-inp-gain-mode
	QMap<QString, QString> *mgcSplitTableCtrlOptions = new QMap<QString, QString>();
	mgcSplitTableCtrlOptions->insert("0", "AGC_determined");
	mgcSplitTableCtrlOptions->insert("1", "onl_in_LPF");
	mgcSplitTableCtrlOptions->insert("2", "onl_in_LMT");

	auto mgcSplitTableCtrlValues = mgcSplitTableCtrlOptions->values();
	QString mgcSplitTableCtrlOptionsData = "";
	for(int i = 0; i < mgcSplitTableCtrlValues.size(); i++) {
		mgcSplitTableCtrlOptionsData += " " + mgcSplitTableCtrlValues.at(i);
	}
	IIOWidget *mgcSplitTableCtrl = IIOWidgetBuilder(mgcWidget)
					       .device(m_device)
					       .attribute("adi,mgc-inc-gain-step")
					       .uiStrategy(IIOWidgetBuilder::ComboUi)
					       .optionsValues(mgcSplitTableCtrlOptionsData)
					       .title("Split Table Control Input Mode")
					       .infoMessage("AGC determine this (0); Only in LPF(1); Only in LMT (2). "
							    "See register 0x0FC, bits [D4], [D3].")
					       .buildSingle();
	mgcWidgetLayout->addWidget(mgcSplitTableCtrl, 3, 0);

	mgcSplitTableCtrl->setUItoDataConversion([this, mgcSplitTableCtrlOptions](QString data) {
		return IIOWidgetUtils::comboUiToDataConversionFunction(data, mgcSplitTableCtrlOptions);
	});
	mgcSplitTableCtrl->setDataToUIConversion([this, mgcSplitTableCtrlOptions](QString data) {
		return IIOWidgetUtils::comboDataToUiConversionFunction(data, mgcSplitTableCtrlOptions);
	});

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
	agcTresholdGainChangesWidgetLayout->setSpacing(10);
	agcTresholdGainChangesWidget->setLayout(agcTresholdGainChangesWidgetLayout);

	Style::setStyle(agcTresholdGainChangesWidget, style::properties::widget::border_interactive);
	Style::setBackgroundColor(agcTresholdGainChangesWidget, json::theme::background_primary);

	QLabel *title = new QLabel("AGC Thresholds and Gain Changes", agcTresholdGainChangesWidget);
	Style::setStyle(title, style::properties::label::menuBig);
	agcTresholdGainChangesWidgetLayout->addWidget(title, 0, 0);

	// adi,agc-outer-thresh-high
	IIOWidget *outerThreshHigh =
		IIOWidgetBuilder(agcTresholdGainChangesWidget)
			.device(m_device)
			.attribute("adi,agc-outer-thresh-high")
			.uiStrategy(IIOWidgetBuilder::RangeUi)
			.optionsValues("[0 1 127]")
			.title("Outer Threshold High (dBFS)")
			.infoMessage("The outer high threshold equals the inner high threshold plus this value. See "
				     "register 0x129, bits [D7:D4]. This data is processed by the driver.")
			.buildSingle();
	agcTresholdGainChangesWidgetLayout->addWidget(outerThreshHigh, 1, 0);

	// adi,agc-outer-thresh-high-dec-steps
	IIOWidget *outerThreshHighDecSteps =
		IIOWidgetBuilder(agcTresholdGainChangesWidget)
			.device(m_device)
			.attribute("adi,agc-outer-thresh-high-dec-steps")
			.uiStrategy(IIOWidgetBuilder::RangeUi)
			.optionsValues("[0 1 15]")
			.title("Decrease Steps")
			.infoMessage("The slow AGC changes gain by this amount when the outer high threshold is "
				     "exceeded. See register 0x12A, bits [D7:D4].")
			.buildSingle();
	agcTresholdGainChangesWidgetLayout->addWidget(outerThreshHighDecSteps, 1, 1);

	// adi,agc-inner-thresh-high
	IIOWidget *innerThreshHigh =
		IIOWidgetBuilder(agcTresholdGainChangesWidget)
			.device(m_device)
			.attribute("adi,agc-inner-thresh-high")
			.uiStrategy(IIOWidgetBuilder::RangeUi)
			.optionsValues("[0 1 127]")
			.title("Inner Threshold High (dBFS) Fast AGC Lock Level (dBFS)")
			.infoMessage("Applies to AGC. This attribute specifies the fast AGC lock level or specifies "
				     "the slow AGC "
				     "inner high threshold. Resolution is −1 dBFS/LSB. See register 0x101 [D6:D0]).")
			.buildSingle();
	agcTresholdGainChangesWidgetLayout->addWidget(innerThreshHigh, 2, 0);

	// adi,agc-inner-thresh-high-dec-steps
	IIOWidget *innerThreshHighDecSteps =
		IIOWidgetBuilder(agcTresholdGainChangesWidget)
			.device(m_device)
			.attribute("adi,agc-inner-thresh-high-dec-steps")
			.uiStrategy(IIOWidgetBuilder::RangeUi)
			.optionsValues("[0 1 7]")
			.title("Decrease Steps")
			.infoMessage("This attribute sets the gain decrease amount when the inner high "
				     "threshold is exceeded. See register 0x123, bits [D6:D4].")
			.buildSingle();
	agcTresholdGainChangesWidgetLayout->addWidget(innerThreshHighDecSteps, 2, 1);

	// adi,agc-inner-thresh-low
	IIOWidget *innerThreshLow = IIOWidgetBuilder(agcTresholdGainChangesWidget)
					    .device(m_device)
					    .attribute("adi,agc-inner-thresh-low")
					    .uiStrategy(IIOWidgetBuilder::RangeUi)
					    .optionsValues("[0 1 127]")
					    .title("Inner Threshold Low (dBFS)")
					    .infoMessage("This attribute sets the slow AGC inner low window threshold. "
							 "See register 0x120, bits [D6:D0].")
					    .buildSingle();
	agcTresholdGainChangesWidgetLayout->addWidget(innerThreshLow, 3, 0);

	// adi,agc-inner-thresh-low-inc-steps
	IIOWidget *innerThreshLowIncSteps =
		IIOWidgetBuilder(agcTresholdGainChangesWidget)
			.device(m_device)
			.attribute("adi,agc-inner-thresh-low-inc-steps")
			.uiStrategy(IIOWidgetBuilder::RangeUi)
			.optionsValues("[0 1 7]")
			.title("Increase Steps")
			.infoMessage("This attribute sets the increase amount used when the gain goes under the "
				     "inner low threshold. See register 0x123, bits [D2:D0].")
			.buildSingle();
	agcTresholdGainChangesWidgetLayout->addWidget(innerThreshLowIncSteps, 3, 1);

	// adi,agc-outer-thresh-low
	IIOWidget *outerThreshLow =
		IIOWidgetBuilder(agcTresholdGainChangesWidget)
			.device(m_device)
			.attribute("adi,agc-outer-thresh-low")
			.uiStrategy(IIOWidgetBuilder::RangeUi)
			.optionsValues("[0 1 127]")
			.title("Outer Threshold Low (dBFS)")
			.infoMessage("The outer low threshold equals the inner low threshold plus this value. See "
				     "register 0x129, bits [D3:D0]. This data is processed by the driver.")
			.buildSingle();
	agcTresholdGainChangesWidgetLayout->addWidget(outerThreshLow, 4, 0);

	// adi,agc-outer-thresh-low-inc-steps
	IIOWidget *outerThreshLowIncSteps =
		IIOWidgetBuilder(agcTresholdGainChangesWidget)
			.device(m_device)
			.attribute("adi,agc-outer-thresh-low-inc-steps")
			.uiStrategy(IIOWidgetBuilder::RangeUi)
			.optionsValues("[0 1 15]")
			.title("Increase Steps")
			.infoMessage("The slow AGC changes gain by this amount when the outer low threshold "
				     "is exceeded. See register 0x12A, bits [D3:D0].")
			.buildSingle();
	agcTresholdGainChangesWidgetLayout->addWidget(outerThreshLowIncSteps, 4, 1);

	// adi,agc-sync-for-gain-counter-enable
	IIOWidget *sync = IIOWidgetBuilder(agcTresholdGainChangesWidget)
				  .device(m_device)
				  .attribute("adi,agc-sync-for-gain-counter-enable")
				  .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
				  .title("AGC Sync for Gain Counter")
				  .infoMessage("If this attribute is set, CTRL_IN2 transitioning high resets the "
					       "counter.See register 0x128, bit D4.")
				  .buildSingle();
	agcTresholdGainChangesWidgetLayout->addWidget(sync, 5, 0);
	sync->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	sync->showProgressBar(false);

	// adi,agc-gain-update-interval-us
	IIOWidget *gainUpdateInterval = IIOWidgetBuilder(agcTresholdGainChangesWidget)
						.device(m_device)
						.attribute("adi,agc-gain-update-interval-us")
						.uiStrategy(IIOWidgetBuilder::RangeUi)
						.optionsValues("[0 10 100000]")
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
	adcOverloadWidgetLayout->setSpacing(10);
	adcOverloadWidget->setLayout(adcOverloadWidgetLayout);

	Style::setStyle(adcOverloadWidget, style::properties::widget::border_interactive);
	Style::setBackgroundColor(adcOverloadWidget, json::theme::background_primary);

	QLabel *title = new QLabel("ADC Overload", adcOverloadWidget);
	Style::setStyle(title, style::properties::label::menuBig);
	adcOverloadWidgetLayout->addWidget(title, 0, 0);

	// adi,gc-adc-large-overload-thresh
	IIOWidget *largeOverloadThresh =
		IIOWidgetBuilder(adcOverloadWidget)
			.device(m_device)
			.attribute("adi,gc-adc-large-overload-thresh")
			.uiStrategy(IIOWidgetBuilder::RangeUi)
			.optionsValues("[0 1 255]")
			.title("Large Thresh")
			.infoMessage("This attribute sets the large ADC overload. See register 0x105.")
			.buildSingle();
	adcOverloadWidgetLayout->addWidget(largeOverloadThresh, 1, 0);

	// adi,gc-adc-small-overload-thresh
	IIOWidget *smallOverloadThresh = IIOWidgetBuilder(adcOverloadWidget)
						 .device(m_device)
						 .attribute("adi,gc-adc-small-overload-thresh")
						 .uiStrategy(IIOWidgetBuilder::RangeUi)
						 .optionsValues("[0 1 255]")
						 .title("Small Thresh")
	                .infoMessage("This attribute sets the small ADC overload. See register 0x104.")
						 .buildSingle();
	adcOverloadWidgetLayout->addWidget(smallOverloadThresh, 1, 1);

	// adi,agc-adc-large-overload-exceed-counter
	IIOWidget *largeOverloadExceed =
		IIOWidgetBuilder(adcOverloadWidget)
			.device(m_device)
			.attribute("adi,agc-adc-large-overload-exceed-counter")
			.uiStrategy(IIOWidgetBuilder::RangeUi)
			.optionsValues("[0 1 15]")
			.title("Large Exceed Cntr")
			.infoMessage("This counter specifies the number of large ADC overloads that must occur before "
				     "the gain will "
				     "decrease by the large ADC overload gain step. See register 0x122, bits [D7:D4].")
			.buildSingle();
	adcOverloadWidgetLayout->addWidget(largeOverloadExceed, 2, 0);

	// adi,agc-adc-small-overload-exceed-counter
	IIOWidget *smallOverloadExceed =
		IIOWidgetBuilder(adcOverloadWidget)
			.device(m_device)
			.attribute("adi,agc-adc-small-overload-exceed-counter")
			.uiStrategy(IIOWidgetBuilder::RangeUi)
			.optionsValues("[0 1 15]")
			.title("Small Exceed Counter")
			.infoMessage("This counter specifies the number of small ADC overloads that must occur to "
				     "prevent a gain increase. See register 0x122, bits [D3:D0].")
			.buildSingle();
	adcOverloadWidgetLayout->addWidget(smallOverloadExceed, 2, 1);

	// adi,agc-adc-large-overload-inc-steps
	IIOWidget *largeDecSteps =
		IIOWidgetBuilder(adcOverloadWidget)
			.device(m_device)
			.attribute("adi,agc-adc-large-overload-inc-steps")
			.uiStrategy(IIOWidgetBuilder::RangeUi)
			.optionsValues("[0 1 15]")
			.title("Large Decr. Steps ")
			.infoMessage(
				"This attribute applies to AGC and determine how much the gain changes for "
				"large LPF in split tablemode or the large LMT and large ADC overloads in full table "
				"mode. See register 0x106, bits [D3:D0]")
			.buildSingle();
	adcOverloadWidgetLayout->addWidget(largeDecSteps, 3, 0);

	// adi,agc-adc-lmt-small-overload-prevent-gain-inc-enable
	IIOWidget *preventGainIncrease = IIOWidgetBuilder(adcOverloadWidget)
						 .device(m_device)
						 .attribute("adi,agc-adc-lmt-small-overload-prevent-gain-inc-enable")
						 .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
						 .title("Prevent Gain Increase")
						 .infoMessage("This attribute set the slow AGC inner low window "
							      "threshold. See register 0x120, bits [D6:D0].")
						 .buildSingle();
	adcOverloadWidgetLayout->addWidget(preventGainIncrease, 3, 1);
	preventGainIncrease->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	preventGainIncrease->showProgressBar(false);

	// sum n samples TODO FIND ATTR

	// adi,agc-immed-gain-change-if-large-adc-overload-enable
	IIOWidget *immedGainChange =
		IIOWidgetBuilder(adcOverloadWidget)
			.device(m_device)
			.attribute("adi,agc-immed-gain-change-if-large-adc-overload-enable")
			.uiStrategy(IIOWidgetBuilder::CheckBoxUi)
			.title("Immediate Gain Change if Large Overload")
			.infoMessage("Set this attribute to allow large ADC overload to reduce gain immediately. See "
				     "register 0x123, bit D3.")
			.buildSingle();
	adcOverloadWidgetLayout->addWidget(immedGainChange, 4, 1);
	immedGainChange->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	immedGainChange->showProgressBar(false);

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
	widgetLayout->setSpacing(10);
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
					.uiStrategy(IIOWidgetBuilder::RangeUi)
					.optionsValues("[16 16 800]")
					.title("High Tresh (mV)")
					.infoMessage("This attribute sets the large LMT overload threshold. See "
						     "register 0x108. This data is "
						     "processed by the driver.")
					.buildSingle();
	widgetLayout->addWidget(highThresh, 1, 0);

	// adi,gc-lmt-overload-low-thresh
	IIOWidget *lowThresh = IIOWidgetBuilder(widget)
				       .device(m_device)
				       .attribute("adi,gc-lmt-overload-low-thresh")
				       .uiStrategy(IIOWidgetBuilder::RangeUi)
				       .optionsValues("[16 16 800]")
				       .title("Small Thresh")
				       .infoMessage("This attribute sets the small LMT overload threshold. See "
						    "register 0x107. This data is "
						    "processed by the driver.")
				       .buildSingle();
	widgetLayout->addWidget(lowThresh, 1, 1);

	// adi,agc-lmt-overload-large-exceed-counter
	IIOWidget *largeExced =
		IIOWidgetBuilder(widget)
			.device(m_device)
			.attribute("adi,agc-lmt-overload-large-exceed-counter")
			.uiStrategy(IIOWidgetBuilder::RangeUi)
			.optionsValues("[0 1 15]")
			.title("Large Exceed Counter")
			.infoMessage(
				"This counter specifies the number of large LMT overloads that must occur before gain "
				"decreases by the LMT Gain Step. See register 0x121, bits [D7:D4].")
			.buildSingle();
	widgetLayout->addWidget(largeExced, 2, 0);

	// adi,agc-lmt-overload-small-exceed-counter
	IIOWidget *smallExced =
		IIOWidgetBuilder(widget)
			.device(m_device)
			.attribute("adi,agc-lmt-overload-small-exceed-counter")
			.uiStrategy(IIOWidgetBuilder::RangeUi)
			.optionsValues("[0 1 15]")
			.title("Small Exceed Cntr")
			.infoMessage(
				"This counter specifies the number of small LMT overloads that much occur to prevent a "
				"gain increase. See register 0x121, bits [D3:D0].")
			.buildSingle();
	widgetLayout->addWidget(smallExced, 2, 1);

	// adi,agc-lmt-overload-large-inc-steps
	IIOWidget *largeInc =
		IIOWidgetBuilder(widget)
			.device(m_device)
			.attribute("adi,agc-lmt-overload-large-inc-steps")
			.uiStrategy(IIOWidgetBuilder::RangeUi)
			.optionsValues("[0 1 7]")
			.title("Large Decr. Steps")
			.infoMessage(
				"This attribute determines how much the gain changes for large LMT in split tablemode "
				"or the small ADC overload for the full table. See register 0x103, bits [D4:D2].")
			.buildSingle();
	widgetLayout->addWidget(largeInc, 3, 0);

	// adi,agc-immed-gain-change-if-large-lmt-overload-enable
	IIOWidget *immedGain =
		IIOWidgetBuilder(widget)
			.device(m_device)
			.attribute("adi,agc-immed-gain-change-if-large-lmt-overload-enable")
			.uiStrategy(IIOWidgetBuilder::CheckBoxUi)
			.title("Immediate Gain Change if Large Overload")
			.infoMessage("Set this attribute to allow large LMT overloads to reduce gain immediately. See "
				     "register 0x123, bit D7.")
			.buildSingle();
	widgetLayout->addWidget(immedGain, 3, 1);
	immedGain->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	immedGain->showProgressBar(false);

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
	widgetLayout->setSpacing(10);
	widget->setLayout(widgetLayout);

	Style::setStyle(widget, style::properties::widget::border_interactive);
	Style::setBackgroundColor(widget, json::theme::background_primary);

	QLabel *title = new QLabel("Digital Gain", widget);
	Style::setStyle(title, style::properties::label::menuBig);
	widgetLayout->addWidget(title, 0, 0);

	// adi,gc-dig-gain-enable
	IIOWidget *gainMode =
		IIOWidgetBuilder(widget)
			.device(m_device)
			.attribute("adi,gc-dig-gain-enable")
			.uiStrategy(IIOWidgetBuilder::CheckBoxUi)
			.title("Dig Gain Enable")
			.infoMessage(
				"This attribute is used in split table mode to enable the digital gain pointer. See "
				"register 0x0FB, bit D2.")
			.buildSingle();
	widgetLayout->addWidget(gainMode, 1, 0);
	gainMode->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	gainMode->showProgressBar(false);

	// adi,agc-dig-saturation-exceed-counter
	IIOWidget *saturationExceedCounter =
		IIOWidgetBuilder(widget)
			.device(m_device)
			.attribute("adi,agc-dig-saturation-exceed-counter")
			.uiStrategy(IIOWidgetBuilder::RangeUi)
			.optionsValues("[0 1 15]")
			.title("Saturation Exceed Counter")
			.infoMessage("This counter specifies the number of digital saturation events that much "
				     "occur to prevent a gain increase. See register 0x128, bits [D3:D0].")
			.buildSingle();
	widgetLayout->addWidget(saturationExceedCounter, 2, 0);

	// adi,gc-max-dig-gain
	IIOWidget *maxDigGain =
		IIOWidgetBuilder(widget)
			.device(m_device)
			.attribute("adi,gc-max-dig-gain")
			.uiStrategy(IIOWidgetBuilder::RangeUi)
			.optionsValues("[0 1 7]")
			.title("Max Digital Gain")
			.infoMessage(
				"This attribute equals the maximum allowable digital gain, and applies to all gain "
				"control modes. See register 0x100, bits [D4:D0].")
			.buildSingle();
	widgetLayout->addWidget(maxDigGain, 1, 1);

	// adi,agc-dig-gain-step-size
	IIOWidget *gainStepSize = IIOWidgetBuilder(widget)
					  .device(m_device)
					  .attribute("adi,agc-dig-gain-step-size")
					  .uiStrategy(IIOWidgetBuilder::RangeUi)
					  .optionsValues("[0 1 8]")
					  .title("Decr Step Size")
					  .infoMessage("If digital saturation occurs, digital gain reduces by this "
						       "value. See register 0x100, bits [D7:D5].")
					  .buildSingle();
	widgetLayout->addWidget(gainStepSize, 2, 1);

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
	layout->setSpacing(10);
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
	layout->setSpacing(10);
	widget->setLayout(layout);

	Style::setStyle(widget, style::properties::widget::border_interactive);

	QLabel *title = new QLabel("State 1 Response to Peak Overload", widget);
	Style::setStyle(title, style::properties::label::menuBig);
	layout->addWidget(title);

	// adi,fagc-state-wait-time-ns
	IIOWidget *stateWaitTime =
		IIOWidgetBuilder(widget)
			.device(m_device)
			.attribute("adi,fagc-state-wait-time-ns")
			.uiStrategy(IIOWidgetBuilder::RangeUi)
			.optionsValues("[0 10 200000]")
			.title("State Wait Time")
			.infoMessage("The fast AGC delays moving from State 1 to State 2 until no peak overloads are "
				     "detected for the value of this counter; measured in ClkRF cycles. See register "
				     "0x117, bits [D4:D0]. This data is processed by the driver.")
			.buildSingle();
	layout->addWidget(stateWaitTime);

	connect(this, &GainWidget::readRequested, this, [=, this]() { stateWaitTime->readAsync(); });

	return widget;
}

QWidget *GainWidget::lowPowerCheck(QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	QGridLayout *layout = new QGridLayout(widget);
	layout->setSpacing(10);
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
					       .infoMessage("Setting this attribute allows the fast AGC to increase "
							    "the gain while optimizing the gain index. "
							    "Clearing it prevents the gain from increasing in any "
							    "condition. See register 0x110, bit D0.")
					       .buildSingle();
	layout->addWidget(allowGainIncrease, 1, 0);
	allowGainIncrease->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	allowGainIncrease->showProgressBar(false);

	// adi,gc-low-power-thresh
	IIOWidget *lowPowerThresh =
		IIOWidgetBuilder(widget)
			.device(m_device)
			.attribute("adi,gc-low-power-thresh")
			.uiStrategy(IIOWidgetBuilder::RangeUi)
			.optionsValues("[0 1 64]")
			.title("Low PWR Thresh (dbFS)")
			.infoMessage("This threshold is used by the fast AGC to determine if the gain should be "
				     "increased. It can also be used to trigger a CTRL_OUT signal transition in MGC "
				     "mode. See register 0x114, bits [D6:D0]. This data is processed by the driver.")
			.buildSingle();
	layout->addWidget(lowPowerThresh, 1, 1);

	// adi,fagc-lp-thresh-increment-time
	IIOWidget *lowPowerThreshIncrementTime =
		IIOWidgetBuilder(widget)
			.device(m_device)
			.attribute("adi,fagc-lp-thresh-increment-time")
			.uiStrategy(IIOWidgetBuilder::RangeUi)
			.optionsValues("[0 1 255]")
			.title("Low PWR THresh Increment Time")
			.infoMessage("This attribute sets the time that the signal power must remain below the Low "
				     "Power Threshold before "
				     "the fast AGC will change gain. Also can be used by the MGC. See register 0x11B, "
				     "bits [D7:D0].")
			.buildSingle();
	layout->addWidget(lowPowerThreshIncrementTime, 2, 0);

	// adi,fagc-lp-thresh-increment-steps
	IIOWidget *lowPowerThreshIncrementSteps =
		IIOWidgetBuilder(widget)
			.device(m_device)
			.attribute("adi,fagc-lp-thresh-increment-steps")
			.uiStrategy(IIOWidgetBuilder::RangeUi)
			.optionsValues("[1 1 8]")
			.title("Increment Step")
			.infoMessage("The Fast AGC will increase the gain index by this amount if signal power "
				     "decreases below the Low "
				     "Power Threshold and only if the Enable Incr Gain is enabled. See register 0x117, "
				     "bits [D7:D5]. This "
				     "data is processed by the driver.")
			.buildSingle();
	layout->addWidget(lowPowerThreshIncrementSteps, 2, 1);

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
	layout->setSpacing(10);
	widget->setLayout(layout);

	Style::setStyle(widget, style::properties::widget::border_interactive);

	QLabel *title = new QLabel("AGC Lock Level Adjustment", widget);
	Style::setStyle(title, style::properties::label::menuBig);
	layout->addWidget(title, 0, 0);

	// adi,agc-inner-thresh-high
	IIOWidget *innerThreshHigh =
		IIOWidgetBuilder(widget)
			.device(m_device)
			.attribute("adi,agc-inner-thresh-high")
			.uiStrategy(IIOWidgetBuilder::RangeUi)
			.optionsValues("[0 1 127]")
			.title("AGC Lock Level (dBFS) (Inner Threshold High)")
			.infoMessage("Applies to AGC. This attribute specifies the fast AGC lock level or specifies "
				     "the slow AGC inner high "
				     "threshold. Resolution is −1 dBFS/LSB. See register 0x101 [D6:D0]).")
			.buildSingle();
	layout->addWidget(innerThreshHigh, 1, 0);

	// adi,fagc-lock-level-gain-increase-upper-limit
	IIOWidget *lockLevelGain =
		IIOWidgetBuilder(widget)
			.device(m_device)
			.attribute("adi,fagc-lock-level-gain-increase-upper-limit")
			.uiStrategy(IIOWidgetBuilder::RangeUi)
			.optionsValues("[0 1 63]")
			.title("Lock Level Gain Increase Upper Limit")
			.infoMessage(
				"This attribute sets the maximum gain index increase that the fast AGC can use for "
				"the lock level adjustment. See register 0x118, bits [D5:D0].")
			.buildSingle();
	layout->addWidget(lockLevelGain, 2, 0);

	// adi,fagc-lock-level-lmt-gain-increase-enable
	IIOWidget *lockLevelLmtGain =
		IIOWidgetBuilder(widget)
			.device(m_device)
			.attribute("adi,fagc-lock-level-lmt-gain-increase-enable")
			.uiStrategy(IIOWidgetBuilder::CheckBoxUi)
			.title("Allow LMT Gain Increase")
			.infoMessage("Set this attribute to allow the AGC to use LMT gain if the gain index needs to "
				     "increase when moving to the AGC Lock Level. See register 0x111, bit D6.")
			.buildSingle();
	layout->addWidget(lockLevelLmtGain, 2, 1);
	lockLevelLmtGain->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	lockLevelLmtGain->showProgressBar(false);

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
	layout->setSpacing(10);
	widget->setLayout(layout);

	Style::setStyle(widget, style::properties::widget::border_interactive);

	QLabel *title = new QLabel("Peak Detectors (again), Final Low Power Test and Settling", widget);
	Style::setStyle(title, style::properties::label::menuBig);
	layout->addWidget(title, 0, 0);

	// adi,fagc-lpf-final-settling-steps
	IIOWidget *lpfFinalSettingsSteps = IIOWidgetBuilder(widget)
						   .device(m_device)
						   .attribute("adi,fagc-lpf-final-settling-steps")
						   .uiStrategy(IIOWidgetBuilder::RangeUi)
						   .optionsValues("[0 1 3]")
						   .title("LPF / Full Table Final, Final Settling Steps")
						   .infoMessage("This attribute sets the reduction to the gain index "
								"if a large LMT or large ADC overload occurs after "
								"Lock Level but before fast AGC state 5. If the number "
								"of overloads exceeds the Final Overrange Count "
								"(fagc_final_overrange_count), the AGC algorithm "
								"restarts. Depending on various conditions if a split "
								"table is used, the gain may reduce in in the LPF or "
								"the LMT (fagc_lmt_final_settling_steps). See "
								"register 0x112, bits [D7:D6].")
						   .buildSingle();
	layout->addWidget(lpfFinalSettingsSteps, 1, 0);

	// adi,fagc-lmt-final-settling-steps
	IIOWidget *lmtFinalSettingsSteps =
		IIOWidgetBuilder(widget)
			.device(m_device)
			.attribute("adi,fagc-lmt-final-settling-steps")
			.uiStrategy(IIOWidgetBuilder::RangeUi)
			.optionsValues("[0 1 3]")
			.title("LMT Final Settling Steps")
			.infoMessage("Post Lock Level Step for LMT Table. See register 0x113, bits [D7:D6].")
			.buildSingle();
	layout->addWidget(lmtFinalSettingsSteps, 1, 1);

	// adi,fagc-final-overrange-count
	IIOWidget *finalOverrange = IIOWidgetBuilder(widget)
					    .device(m_device)
					    .attribute("adi,fagc-final-overrange-count")
					    .uiStrategy(IIOWidgetBuilder::RangeUi)
					    .optionsValues("[0 1 7]")
					    .title("Final Overrange Count")
					    .infoMessage("Final Overrange Count. See register 0x116, bits [D7:D5].")
					    .buildSingle();
	layout->addWidget(finalOverrange, 2, 0);

	// adi,fagc-gain-increase-after-gain-lock-enable
	IIOWidget *increaseAfterGainLock = IIOWidgetBuilder(widget)
						   .device(m_device)
						   .attribute("adi,fagc-gain-increase-after-gain-lock-enable")
						   .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
						   .title("Low Power Test Gain Increase after Gain Lock")
						   .infoMessage("Set this attribute to allow gain increases after the "
								"gain has locked but before State 5. Signal power "
								"must be lower than the low power threshold for longer "
								"than the increment time duration register. See "
								"register 0x110, bit D7.")
						   .buildSingle();
	layout->addWidget(increaseAfterGainLock, 2, 1);
	increaseAfterGainLock->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	increaseAfterGainLock->showProgressBar(false);

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
	layout->setSpacing(10);
	widget->setLayout(layout);

	Style::setStyle(widget, style::properties::widget::border_interactive);

	QLabel *title = new QLabel("Gain Unlock", widget);
	Style::setStyle(title, style::properties::label::menuBig);
	layout->addWidget(title, 0, 0);

	QLabel *row10 = new QLabel("When exit Receive State Restart Gain Lock Algorithm (GLA) and goto", widget);
	Style::setStyle(row10, style::properties::label::subtle);
	layout->addWidget(row10, 1, 0);

	QLabel *row20 = new QLabel("SET Gain use Gain from", widget);
	Style::setStyle(row20, style::properties::label::subtle);
	layout->addWidget(row20, 2, 0);

	QLabel *row80 = new QLabel("When testing signal power against the thresholds above, measure power for "
				   "(sample periods)"
				   ", widget)",
				   widget);
	row80->setWordWrap(true);
	Style::setStyle(row80, style::properties::label::subtle);
	layout->addWidget(row80, 8, 0);

	QLabel *row22 = new QLabel("Optimized Gain Offset", widget);
	Style::setStyle(row22, style::properties::label::subtle);
	layout->addWidget(row22, 2, 2);

	QLabel *row42 = new QLabel("for (RX Samples) restart GLA ...", widget);
	Style::setStyle(row42, style::properties::label::subtle);
	layout->addWidget(row42, 4, 2);

	QLabel *row52 = new QLabel("... and GTO", widget);
	Style::setStyle(row52, style::properties::label::subtle);
	layout->addWidget(row52, 5, 2);

	// adi,fagc-gain-index-type-after-exit-rx-mode
	QMap<QString, QString> *gainIndexTypeOptions = new QMap<QString, QString>();
	gainIndexTypeOptions->insert("0", "MAX_Gain");
	gainIndexTypeOptions->insert("1", "SET_Gain");
	gainIndexTypeOptions->insert("2", "OPTIMIZED _Gain");

	auto gainIndexTypeValues = gainIndexTypeOptions->values();
	QString gainIndexTypeOptionsData = "";
	for(int i = 0; i < gainIndexTypeValues.size(); i++) {
		gainIndexTypeOptionsData += " " + gainIndexTypeValues.at(i);
	}

	IIOWidget *gainIndexType = IIOWidgetBuilder(widget)
					   .device(m_device)
					   .attribute("adi,fagc-gain-index-type-after-exit-rx-mode")
					   .uiStrategy(IIOWidgetBuilder::ComboUi)
					   .optionsValues(gainIndexTypeOptionsData)
					   .title("")
					   .infoMessage("MAX Gain (0); Optimized Gain (1); Set Gain (2)."
							"See register 0x110, bits D4,D2. "
							"This data is processed by the driver.")
					   .buildSingle();
	layout->addWidget(gainIndexType, 1, 1);

	gainIndexType->setUItoDataConversion([this, gainIndexTypeOptions](QString data) {
		return IIOWidgetUtils::comboUiToDataConversionFunction(data, gainIndexTypeOptions);
	});
	gainIndexType->setDataToUIConversion([this, gainIndexTypeOptions](QString data) {
		return IIOWidgetUtils::comboDataToUiConversionFunction(data, gainIndexTypeOptions);
	});

	// adi,fagc-use-last-lock-level-for-set-gain-enable
	QMap<QString, QString> *increaseAfterGainLockOptions = new QMap<QString, QString>();
	increaseAfterGainLockOptions->insert("0", "Beginning_of_previous_Burst");
	increaseAfterGainLockOptions->insert("1", "End_of_previous_Burst");

	auto increaseAfterGainLockValues = increaseAfterGainLockOptions->values();
	QString increaseAfterGainLockOptionsData = "";
	for(int i = 0; i < increaseAfterGainLockValues.size(); i++) {
		increaseAfterGainLockOptionsData += " " + increaseAfterGainLockValues.at(i);
	}

	IIOWidget *increaseAfterGainLock =
		IIOWidgetBuilder(widget)
			.device(m_device)
			.attribute("adi,fagc-use-last-lock-level-for-set-gain-enable")
			.uiStrategy(IIOWidgetBuilder::ComboUi)
			.optionsValues(increaseAfterGainLockOptionsData)
			.title("")
			.infoMessage("Set this attribute to use the last gain index of the previous frame for set "
				     "gain. Clear to use the "
				     "first gain index of the previous frame. See register 0x111, bit D7.")
			.buildSingle();
	layout->addWidget(increaseAfterGainLock, 2, 1);

	increaseAfterGainLock->setUItoDataConversion([this, increaseAfterGainLockOptions](QString data) {
		return IIOWidgetUtils::comboUiToDataConversionFunction(data, increaseAfterGainLockOptions);
	});
	increaseAfterGainLock->setDataToUIConversion([this, increaseAfterGainLockOptions](QString data) {
		return IIOWidgetUtils::comboDataToUiConversionFunction(data, increaseAfterGainLockOptions);
	});

	// adi,fagc-optimized-gain-offset
	IIOWidget *optimiezedGainOffset =
		IIOWidgetBuilder(widget)
			.device(m_device)
			.attribute("adi,fagc-optimized-gain-offset")
			.uiStrategy(IIOWidgetBuilder::RangeUi)
			.optionsValues("[0 1 15]")
			.title("")
			.infoMessage("The offset added to the last gain lock level of the previous frame."
				     "The result is the optimize gain index."
				     "See register 0x116, bits [D3:D0].")
			.buildSingle();
	layout->addWidget(optimiezedGainOffset, 2, 3);

	// adi,fagc-rst-gla-stronger-sig-thresh-exceeded-enable
	IIOWidget *sigThreshExceedEn = IIOWidgetBuilder(widget)
					       .device(m_device)
					       .attribute("adi,fagc-rst-gla-stronger-sig-thresh-exceeded-enable")
					       .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
					       .title("If Signal Power increases above Lock Level by (dBFS)")
					       .infoMessage("If this attribute is set and the fast AGC is in State 5, "
							    "the gain will not change even if the signal "
							    "power increase by more than the Stronger Signal "
							    "Threshold. See register 0x115, bit D7.")
					       .buildSingle();
	layout->addWidget(sigThreshExceedEn, 3, 0);
	sigThreshExceedEn->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	sigThreshExceedEn->showProgressBar(false);

	// adi,fagc-rst-gla-stronger-sig-thresh-above-ll
	IIOWidget *fagcRstGlaStrongerSigThresh =
		IIOWidgetBuilder(widget)
			.device(m_device)
			.attribute("adi,fagc-rst-gla-stronger-sig-thresh-above-ll")
			.uiStrategy(IIOWidgetBuilder::RangeUi)
			.optionsValues("[0 1 63]")
			.title("")
			.infoMessage("If the signal power increases by this threshold and the signal power remains at "
				     "this level or higher "
				     "for a duration that is twice the Gain Lock Exit Count, the gain may unlock, "
				     "depending on other AGC "
				     "configuration bits. See register 0x113, bits [D5:D0].")
			.buildSingle();
	layout->addWidget(fagcRstGlaStrongerSigThresh, 3, 1);

	// adi,fagc-rst-gla-engergy-lost-sig-thresh-exceeded-enable
	IIOWidget *rstGlaEngLostSigThreshExceedEn =
		IIOWidgetBuilder(widget)
			.device(m_device)
			.attribute("adi,fagc-rst-gla-engergy-lost-sig-thresh-exceeded-enable")
			.uiStrategy(IIOWidgetBuilder::CheckBoxUi)
			.title("If Signal Power decreases below Lock Level by (dBFS)")
			.infoMessage("If this attribute is set and the fast AGC is in State 5, the gain will not "
				     "change even if the average "
				     "signal power decreases more than the Energy Lost Threshold register. See "
				     "register 0x110, bit D3.")
			.buildSingle();
	layout->addWidget(rstGlaEngLostSigThreshExceedEn, 4, 0);
	rstGlaEngLostSigThreshExceedEn->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	rstGlaEngLostSigThreshExceedEn->showProgressBar(false);

	// adi,fagc-rst-gla-engergy-lost-sig-thresh-below-ll
	IIOWidget *rstGlaEnergyLost = IIOWidgetBuilder(widget)
					      .device(m_device)
					      .attribute("adi,fagc-rst-gla-engergy-lost-sig-thresh-below-ll")
					      .uiStrategy(IIOWidgetBuilder::RangeUi)
					      .optionsValues("[0 1 63]")
					      .title("")
					      .infoMessage("If the signal power decreases by this threshold and the "
							   "signal power remains at this level or lower "
							   "for a duration that is twice the Gain Lock Exit Count, the "
							   "gain may unlock, depending on other AGC "
							   "configuration bits. See register 0x112, bits [D5:D0].")
					      .buildSingle();
	layout->addWidget(rstGlaEnergyLost, 4, 1);

	// adi,fagc-energy-lost-stronger-sig-gain-lock-exit-cnt
	IIOWidget *energyLostStronger = IIOWidgetBuilder(widget)
						.device(m_device)
						.attribute("adi,fagc-energy-lost-stronger-sig-gain-lock-exit-cnt")
						.uiStrategy(IIOWidgetBuilder::RangeUi)
						.optionsValues("[0 1 63]")
						.title("")
						.infoMessage("Gain Lock Exit Count. See register 0x119, bits [D5:D0].")
						.buildSingle();
	layout->addWidget(energyLostStronger, 4, 3);

	// adi,fagc-rst-gla-large-lmt-overload-enable
	IIOWidget *rstGlaLargeLmtOverloadEn =
		IIOWidgetBuilder(widget)
			.device(m_device)
			.attribute("adi,fagc-rst-gla-large-lmt-overload-enable")
			.uiStrategy(IIOWidgetBuilder::CheckBoxUi)
			.title("If Large LMT Overload, Do Not Change Gain but Restart GLA")
			.infoMessage("Unlock Gain if Lg ADC or LMT Ovrg. See register 0x110, bit D1.")
			.buildSingle();
	layout->addWidget(rstGlaLargeLmtOverloadEn, 5, 0);
	rstGlaLargeLmtOverloadEn->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	rstGlaLargeLmtOverloadEn->showProgressBar(false);

	// adi,fagc-rst-gla-engergy-lost-goto-optim-gain-enable
	QMap<QString, QString> *rstGlaEnergyLostGotoOptimGainEnOptions = new QMap<QString, QString>();
	rstGlaEnergyLostGotoOptimGainEnOptions->insert("0", "MAX_Gain");
	rstGlaEnergyLostGotoOptimGainEnOptions->insert("1", "OPTIMIZED_Gain");

	auto rstGlaEnergyLostGotoOptimGainEnValues = rstGlaEnergyLostGotoOptimGainEnOptions->values();
	QString rstGlaEnergyLostGotoOptimGainEnOptionsData = "";
	for(int i = 0; i < rstGlaEnergyLostGotoOptimGainEnValues.size(); i++) {
		rstGlaEnergyLostGotoOptimGainEnOptionsData += " " + rstGlaEnergyLostGotoOptimGainEnValues.at(i);
	}

	IIOWidget *rstGlaEnergyLostGotoOptimGainEn =
		IIOWidgetBuilder(widget)
			.device(m_device)
			.attribute("adi,fagc-rst-gla-engergy-lost-goto-optim-gain-enable")
			.uiStrategy(IIOWidgetBuilder::ComboUi)
			.optionsValues(rstGlaEnergyLostGotoOptimGainEnOptionsData)
			.title("")
			.infoMessage("If this attribute is set and the fast AGC is in State 5, the gain index will go "
				     "to the "
				     "optimize gain value if an energy lost state occurs or when the EN_AGC signal "
				     "goes high. See register "
				     "0x110, bit D6.")
			.buildSingle();
	layout->addWidget(rstGlaEnergyLostGotoOptimGainEn, 5, 3);

	rstGlaEnergyLostGotoOptimGainEn->setUItoDataConversion([this,
								rstGlaEnergyLostGotoOptimGainEnOptions](QString data) {
		return IIOWidgetUtils::comboUiToDataConversionFunction(data, rstGlaEnergyLostGotoOptimGainEnOptions);
	});
	rstGlaEnergyLostGotoOptimGainEn->setDataToUIConversion([this,
								rstGlaEnergyLostGotoOptimGainEnOptions](QString data) {
		return IIOWidgetUtils::comboDataToUiConversionFunction(data, rstGlaEnergyLostGotoOptimGainEnOptions);
	});

	// adi,fagc-rst-gla-large-adc-overload-enable
	IIOWidget *glaLargeAdcOverloadEn =
		IIOWidgetBuilder(widget)
			.device(m_device)
			.attribute("adi,fagc-rst-gla-large-adc-overload-enable")
			.uiStrategy(IIOWidgetBuilder::CheckBoxUi)
			.title("If Large ADC Overload do not change Gain but restart GLA")
			.infoMessage("Unlock gain if ADC Ovrg, Lg ADC or LMT Ovrg. See register 0x110, bit D1 and "
				     "register 0x114, bit D7. This data is processed by the driver.")
			.buildSingle();
	layout->addWidget(glaLargeAdcOverloadEn, 6, 0);
	glaLargeAdcOverloadEn->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	glaLargeAdcOverloadEn->showProgressBar(false);

	// adi,fagc-rst-gla-en-agc-pulled-high-enable
	IIOWidget *rstGlaEnAgc =
		IIOWidgetBuilder(widget)
			.device(m_device)
			.attribute("adi,fagc-gain-increase-after-gain-lock-enable")
			.uiStrategy(IIOWidgetBuilder::CheckBoxUi)
			.title("If EN_AGC is pulled high restart GLA and goto")
			.infoMessage(
				"See fagc_rst_gla_if_en_agc_pulled_high_mode. This data is processed by the driver.")
			.buildSingle();
	layout->addWidget(rstGlaEnAgc, 7, 0);
	rstGlaEnAgc->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	rstGlaEnAgc->showProgressBar(false);

	// adi,fagc-rst-gla-if-en-agc-pulled-high-mode
	QMap<QString, QString> *rstGlaIfEnAgcPulledOptions = new QMap<QString, QString>();
	rstGlaIfEnAgcPulledOptions->insert("0", "MAX_Gain");
	rstGlaIfEnAgcPulledOptions->insert("1", "OPTIMIZED_Gain");

	auto rstGlaIfEnAgcPulledValues = rstGlaIfEnAgcPulledOptions->values();
	QString rstGlaIfEnAgcPulledOptionsData = "";
	for(int i = 0; i < rstGlaIfEnAgcPulledValues.size(); i++) {
		rstGlaIfEnAgcPulledOptionsData += " " + rstGlaIfEnAgcPulledValues.at(i);
	}

	IIOWidget *rstGlaIfEnAgcPulled = IIOWidgetBuilder(widget)
						 .device(m_device)
						 .attribute("adi,fagc-rst-gla-if-en-agc-pulled-high-mode")
						 .uiStrategy(IIOWidgetBuilder::ComboUi)
						 .optionsValues(rstGlaIfEnAgcPulledOptionsData)
						 .title("")
						 .infoMessage("MAX Gain (0); Optimized Gain (1); Set Gain (2), No gain "
							      "change. See registers 0x110, 0x111")
						 .buildSingle();
	layout->addWidget(rstGlaIfEnAgcPulled, 7, 1);

	rstGlaIfEnAgcPulled->setUItoDataConversion([this, rstGlaIfEnAgcPulledOptions](QString data) {
		return IIOWidgetUtils::comboUiToDataConversionFunction(data, rstGlaIfEnAgcPulledOptions);
	});
	rstGlaIfEnAgcPulled->setDataToUIConversion([this, rstGlaIfEnAgcPulledOptions](QString data) {
		return IIOWidgetUtils::comboDataToUiConversionFunction(data, rstGlaIfEnAgcPulledOptions);
	});

	// adi,fagc-power-measurement-duration-in-state5
	IIOWidget *powerMeasurementDuration =
		IIOWidgetBuilder(widget)
			.device(m_device)
			.attribute("adi,fagc-power-measurement-duration-in-state5")
			.uiStrategy(IIOWidgetBuilder::RangeUi)
			.optionsValues("[0 16 524288]")
			.title("")
			.infoMessage("The power measurement duration used by the gain control algorithm for State 5 "
				     "(gain lock) - fast AGC. "
				     "See register 0x109, bit D7 and 0x10a, bits [D7:D5].")
			.buildSingle();
	layout->addWidget(powerMeasurementDuration, 8, 1);

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
	layout->setSpacing(10);
	widget->setLayout(layout);

	Style::setStyle(widget, style::properties::widget::border_interactive);
	Style::setBackgroundColor(widget, json::theme::background_primary);

	QLabel *title = new QLabel("MISC", widget);
	Style::setStyle(title, style::properties::label::menuBig);
	layout->addWidget(title);

	// adi,fagc-dec-pow-measurement-duration
	IIOWidget *stateWaitTime =
		IIOWidgetBuilder(widget)
			.device(m_device)
			.attribute("adi,fagc-dec-pow-measurement-duration")
			.uiStrategy(IIOWidgetBuilder::RangeUi)
			.optionsValues("[0 1 100]")
			.title("Dec PWR mess. duration in fast attack mode")
			.infoMessage("The power measurement duration used by the gain control algorithm. See register "
				     "0x15C, bits [D3:D0]. This data is processed by the driver.")
			.buildSingle();
	layout->addWidget(stateWaitTime);

	connect(this, &GainWidget::readRequested, this, [=, this]() { stateWaitTime->readAsync(); });

	return widget;
}
