#include "auxadcdaciowidget.h"

#include <style.h>
#include <iioutil/connectionprovider.h>

using namespace scopy;
using namespace pluto;

AuxAdcDacIoWidget::AuxAdcDacIoWidget(QString uri, QWidget *parent)
	: m_uri(uri)
	, QWidget{parent}
{

	m_layout = new QVBoxLayout(this);
	m_layout->setMargin(0);
	m_layout->setContentsMargins(0, 0, 0, 0);
	setLayout(m_layout);

	QWidget *auxAdcDacIoWidget = new QWidget(parent);
	QVBoxLayout *layout = new QVBoxLayout(auxAdcDacIoWidget);
	layout->setSpacing(10);
	auxAdcDacIoWidget->setLayout(layout);
	layout->setMargin(0);

	QScrollArea *scrollArea = new QScrollArea(parent);
	scrollArea->setWidgetResizable(true);
	scrollArea->setWidget(auxAdcDacIoWidget);

	// Get connection to device
	Connection *conn = ConnectionProvider::GetInstance()->open(m_uri);
	// iio:device0: ad9361-phy
	m_device = iio_context_find_device(conn->context(), "ad9361-phy");

	layout->addWidget(tempSensorWidget(auxAdcDacIoWidget));
	layout->addWidget(auxAdcWidget(auxAdcDacIoWidget));
	layout->addWidget(auxDacWidget(auxAdcDacIoWidget));
	layout->addWidget(controlsOutWidget(auxAdcDacIoWidget));
	layout->addWidget(gposWidget(auxAdcDacIoWidget));

	layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));
	m_layout->addWidget(scrollArea);
}

AuxAdcDacIoWidget::~AuxAdcDacIoWidget() { ConnectionProvider::close(m_uri); }

QWidget *AuxAdcDacIoWidget::tempSensorWidget(QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	QVBoxLayout *tempSensorLayout = new QVBoxLayout(widget);
	widget->setLayout(tempSensorLayout);

	Style::setStyle(widget, style::properties::widget::border_interactive);
	Style::setBackgroundColor(widget, json::theme::background_primary);

	QLabel *title = new QLabel("Temp Sensor", widget);
	Style::setStyle(title, style::properties::label::menuBig);
	tempSensorLayout->addWidget(title);

	// adi,temp-sense-measurement-interval-ms
	IIOWidget *tempSenseMeasurementInterval =
		IIOWidgetBuilder(widget)
			.device(m_device)
			.attribute("adi,temp-sense-measurement-interval-ms")
			.uiStrategy(IIOWidgetBuilder::RangeUi)
			.optionsValues("[0 100 20000]")
			.title("Measurement Interval (ms)")
			.infoMessage("Measurement interval in ms. This data is processed by the driver.")
			.buildSingle();
	tempSensorLayout->addWidget(tempSenseMeasurementInterval);

	// adi,temp-sense-offset-signed
	IIOWidget *tempSenseOffset = IIOWidgetBuilder(widget)
					     .device(m_device)
					     .attribute("adi,temp-sense-offset-signed")
					     .uiStrategy(IIOWidgetBuilder::RangeUi)
					     .optionsValues("[-128 1 127]")
					     .infoMessage("Offset in signed deg. C, range -128…127")
					     .title("Offset")
					     .buildSingle();
	tempSensorLayout->addWidget(tempSenseOffset);

	// adi,temp-sense-decimation
	IIOWidget *tempSenseDecimation = IIOWidgetBuilder(widget)
						 .device(m_device)
						 .attribute("adi,temp-sense-decimation")
						 .uiStrategy(IIOWidgetBuilder::RangeUi)
						 .optionsValues("[256 256 32768]")
						 .title("Decimation")
						 .infoMessage("Decimation of the AuxADC used to derive the "
							      "temperature. This data is processed by the driver.")
						 .buildSingle();
	tempSensorLayout->addWidget(tempSenseDecimation);

	// adi,temp-sense-periodic-measurement-enable
	IIOWidget *tempSensePeriodicMeasurement = IIOWidgetBuilder(widget)
							  .device(m_device)
							  .attribute("adi,temp-sense-periodic-measurement-enable")
							  .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
							  .title("Periodic Measurement")
							  .infoMessage("Enables periodic measurement")
							  .buildSingle();
	tempSensorLayout->addWidget(tempSensePeriodicMeasurement);
	tempSensePeriodicMeasurement->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	tempSensePeriodicMeasurement->showProgressBar(false);

	connect(this, &AuxAdcDacIoWidget::readRequested, this, [=, this]() {
		tempSenseMeasurementInterval->readAsync();
		tempSenseOffset->readAsync();
		tempSenseDecimation->readAsync();
		tempSensePeriodicMeasurement->readAsync();
	});

	return widget;
}

QWidget *AuxAdcDacIoWidget::auxAdcWidget(QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	QVBoxLayout *widgetLayout = new QVBoxLayout(widget);
	widget->setLayout(widgetLayout);

	Style::setStyle(widget, style::properties::widget::border_interactive);
	Style::setBackgroundColor(widget, json::theme::background_primary);

	QLabel *title = new QLabel("Aux ADC", widget);
	Style::setStyle(title, style::properties::label::menuBig);
	widgetLayout->addWidget(title);

	// adi,aux-adc-rate
	IIOWidget *auxAdcRate =
		IIOWidgetBuilder(widget)
			.device(m_device)
			.attribute("adi,aux-adc-rate")
			.uiStrategy(IIOWidgetBuilder::RangeUi)
			.optionsValues("[0 1000 100000000]")
			.title("Rate")
			.infoMessage(
				"This sets the AuxADC clock frequency in Hz. See register 0x01C, bits [D5:D0]. This "
				"data is processed by the driver.")
			.buildSingle();
	widgetLayout->addWidget(auxAdcRate);

	// adi,aux-adc-decimation
	IIOWidget *auxAdcDecimation =
		IIOWidgetBuilder(widget)
			.device(m_device)
			.attribute("adi,aux-adc-decimation")
			.uiStrategy(IIOWidgetBuilder::RangeUi)
			.optionsValues("[256 256 32768]")
			.title("Decimation")
			.infoMessage("This sets the AuxADC decimation, See register 0x01D, bits [D3:D1]. This data is "
				     "processed by the driver.")
			.buildSingle();
	widgetLayout->addWidget(auxAdcDecimation);

	connect(this, &AuxAdcDacIoWidget::readRequested, this, [=, this]() {
		auxAdcRate->readAsync();
		auxAdcDecimation->readAsync();
	});

	return widget;
}

QWidget *AuxAdcDacIoWidget::auxDacWidget(QWidget *parent)
{

	QWidget *auxDacWidget = new QWidget(parent);
	QVBoxLayout *auxDacWidgetLayout = new QVBoxLayout(auxDacWidget);
	auxDacWidget->setLayout(auxDacWidgetLayout);

	Style::setStyle(auxDacWidget, style::properties::widget::border_interactive);
	Style::setBackgroundColor(auxDacWidget, json::theme::background_primary);

	// adi,aux-dac-manual-mode-enable
	IIOWidget *auxDacManualMode = IIOWidgetBuilder(auxDacWidget)
					      .device(m_device)
					      .attribute("adi,aux-dac-manual-mode-enable")
					      .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
					      .title("Manual Mode Enabled")
					      .infoMessage("If enabled the Aux DAC doesn't slave the ENSM")
					      .buildSingle();
	auxDacWidgetLayout->addWidget(auxDacManualMode);
	auxDacManualMode->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	auxDacManualMode->showProgressBar(false);

	connect(this, &AuxAdcDacIoWidget::readRequested, this, [=, this]() { auxDacManualMode->readAsync(); });

	// getAuxAdcDac
	QHBoxLayout *auxDacLayout = new QHBoxLayout();

	// TODO add check for dac counter
	auxDacLayout->addWidget(getAuxDac("1", auxDacWidget));
	auxDacLayout->addWidget(getAuxDac("2", auxDacWidget));

	auxDacWidgetLayout->addLayout(auxDacLayout);

	return auxDacWidget;
}

QWidget *AuxAdcDacIoWidget::getAuxDac(QString dacx, QWidget *parent)
{
	QWidget *auxDacWidget = new QWidget(parent);
	QGridLayout *layout = new QGridLayout(auxDacWidget);
	auxDacWidget->setLayout(layout);

	Style::setStyle(auxDacWidget, style::properties::widget::border_interactive);

	QLabel *dacLabel = new QLabel("DAC" + dacx, auxDacWidget);
	Style::setStyle(dacLabel, style::properties::label::menuBig);
	layout->addWidget(dacLabel, 0, 0);

	// Get connection to device
	Connection *conn = ConnectionProvider::GetInstance()->open(m_uri);
	// iio:device0: ad9361-phy
	iio_device *m_device = iio_context_find_device(conn->context(), "ad9361-phy");

	// adi,aux-dacx-default-value-m
	IIOWidget *dacDefaultValue = IIOWidgetBuilder(auxDacWidget)
					     .device(m_device)
					     .attribute("adi,aux-dac" + dacx + "-default-value-mV")
					     .uiStrategy(IIOWidgetBuilder::RangeUi)
					     .optionsValues("[306 1 3300]")
					     .title("Default Value (mV)")
					     .buildSingle();
	layout->addWidget(dacDefaultValue, 1, 1);

	// adi,aux-dacx-active-in-alert-enable
	IIOWidget *dacActiveInAlert = IIOWidgetBuilder(auxDacWidget)
					      .device(m_device)
					      .attribute("adi,aux-dac" + dacx + "-active-in-alert-enable")
					      .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
					      .title("Enable ALERT")
					      .buildSingle();
	layout->addWidget(dacActiveInAlert, 2, 0);
	dacActiveInAlert->showProgressBar(false);

	layout->addWidget(new QLabel("RX/TX Delay (us)", auxDacWidget), 2, 1);

	// adi,aux-dacx-active-in-rx-enable
	IIOWidget *dacActiveInRx = IIOWidgetBuilder(auxDacWidget)
					   .device(m_device)
					   .attribute("adi,aux-dac" + dacx + "-active-in-rx-enable")
					   .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
					   .title("Enable in RX")
					   .buildSingle();
	layout->addWidget(dacActiveInRx, 3, 0);
	dacActiveInRx->showProgressBar(false);

	// adi,aux-dacx-active-in-tx-enable
	IIOWidget *dacActiveInTx = IIOWidgetBuilder(auxDacWidget)
					   .device(m_device)
					   .attribute("adi,aux-dac" + dacx + "-active-in-tx-enable")
					   .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
					   .title("Enable in Tx")
					   .buildSingle();
	layout->addWidget(dacActiveInTx, 4, 0);
	dacActiveInTx->showProgressBar(false);

	// adi,aux-dacx-rx-delay-us
	IIOWidget *rxDelay = IIOWidgetBuilder(auxDacWidget)
				     .device(m_device)
				     .attribute("adi,aux-dac" + dacx + "-rx-delay-us")
				     .uiStrategy(IIOWidgetBuilder::RangeUi)
				     .optionsValues("[0 1 255]")
				     .title(" ")
				     .buildSingle();
	layout->addWidget(rxDelay, 3, 1);

	// adi,aux-dacx-tx-delay-us
	IIOWidget *txDelay = IIOWidgetBuilder(auxDacWidget)
				     .device(m_device)
				     .attribute("adi,aux-dac" + dacx + "-tx-delay-us")
				     .uiStrategy(IIOWidgetBuilder::RangeUi)
				     .optionsValues("[0 1 255]")
				     .title(" ")
				     .buildSingle();
	layout->addWidget(txDelay, 4, 1);

	connect(this, &AuxAdcDacIoWidget::readRequested, this, [=, this]() {
		dacDefaultValue->readAsync();
		dacActiveInAlert->readAsync();
		dacActiveInRx->readAsync();
		dacActiveInTx->readAsync();
		rxDelay->readAsync();
		txDelay->readAsync();
	});

	return auxDacWidget;
}

QWidget *AuxAdcDacIoWidget::controlsOutWidget(QWidget *parent)
{
	QWidget *controlsOutWidget = new QWidget(parent);
	QVBoxLayout *controlsOutWidgetLayout = new QVBoxLayout(controlsOutWidget);
	controlsOutWidget->setLayout(controlsOutWidgetLayout);

	Style::setStyle(controlsOutWidget, style::properties::widget::border_interactive);
	Style::setBackgroundColor(controlsOutWidget, json::theme::background_primary);

	controlsOutWidgetLayout->addWidget(new QLabel("Control OUTS", controlsOutWidget));

	// adi,ctrl-outs-index
	IIOWidget *ctrlOutsIndex = IIOWidgetBuilder(controlsOutWidget)
					   .device(m_device)
					   .attribute("adi,ctrl-outs-index")
					   .uiStrategy(IIOWidgetBuilder::RangeUi)
					   .optionsValues("[0 1 31]")
					   .title("Index")
					   .buildSingle();
	controlsOutWidgetLayout->addWidget(ctrlOutsIndex);

	// adi,ctrl-outs-enable-mask
	IIOWidget *ctrlOutsMask = IIOWidgetBuilder(controlsOutWidget)
					  .device(m_device)
					  .attribute("adi,ctrl-outs-enable-mask")
					  .uiStrategy(IIOWidgetBuilder::RangeUi)
					  .optionsValues("[0 1 255]")
					  .title("Mask")
					  .buildSingle();
	controlsOutWidgetLayout->addWidget(ctrlOutsMask);

	connect(this, &AuxAdcDacIoWidget::readRequested, this, [=, this]() {
		ctrlOutsIndex->readAsync();
		ctrlOutsMask->readAsync();
	});

	return controlsOutWidget;
}

QWidget *AuxAdcDacIoWidget::gposWidget(QWidget *parent)
{

	QWidget *widget = new QWidget(parent);
	QGridLayout *widgetLayout = new QGridLayout(widget);
	widget->setLayout(widgetLayout);

	Style::setStyle(widget, style::properties::widget::border_interactive);
	Style::setBackgroundColor(widget, json::theme::background_primary);

	QLabel *title = new QLabel("GPO Manual Mode", widget);
	Style::setStyle(title, style::properties::label::menuBig);
	widgetLayout->addWidget(title, 0, 0);

	// adi,gpo-manual-mode-enable
	IIOWidget *gpoManualMode =
		IIOWidgetBuilder(parent)
			.device(m_device)
			.attribute("adi,gpo-manual-mode-enable")
			.uiStrategy(IIOWidgetBuilder::CheckBoxUi)
			.title("Enable")
			.infoMessage(
				"Enables GPO manual mode, this will conflict with automatic ENSM slave and eLNA mode")
			.buildSingle();
	widgetLayout->addWidget(gpoManualMode, 1, 0);
	gpoManualMode->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	gpoManualMode->showProgressBar(false);

	// bitmask
	QLabel *bitmaskLabel = new QLabel("GPO Bit Mask", widget);
	bitmaskLabel->setToolTip("Enable bit mask, setting or clearing bits will change the level of the corresponding "
				 "output. Bit0 → GPO, Bit1 → GPO1, Bit2 → GPO2, Bit3 → GP03");
	widgetLayout->addWidget(bitmaskLabel, 2, 0);
	// GPO0
	m_gpo0Mask = new MenuOnOffSwitch("GPO0", this, false);
	widgetLayout->addWidget(m_gpo0Mask, 3, 0);

	//  GPO1
	m_gpo1Mask = new MenuOnOffSwitch("GPO1", this, false);
	widgetLayout->addWidget(m_gpo1Mask, 3, 1);

	//  GPO2
	m_gpo2Mask = new MenuOnOffSwitch("GPO2", this, false);
	widgetLayout->addWidget(m_gpo2Mask, 3, 2);

	//  GPO3
	m_gpo3Mask = new MenuOnOffSwitch("GPO03", this, false);
	widgetLayout->addWidget(m_gpo3Mask, 3, 3);

	connect(m_gpo0Mask->onOffswitch(), &QAbstractButton::toggled, this, &AuxAdcDacIoWidget::applyGpoMask);
	connect(m_gpo1Mask->onOffswitch(), &QAbstractButton::toggled, this, &AuxAdcDacIoWidget::applyGpoMask);
	connect(m_gpo2Mask->onOffswitch(), &QAbstractButton::toggled, this, &AuxAdcDacIoWidget::applyGpoMask);
	connect(m_gpo3Mask->onOffswitch(), &QAbstractButton::toggled, this, &AuxAdcDacIoWidget::applyGpoMask);

	widgetLayout->addWidget(gpoWidget("0", parent), 4, 0, 1, 2);
	widgetLayout->addWidget(gpoWidget("1", parent), 4, 2, 1, 2);
	widgetLayout->addWidget(gpoWidget("2", parent), 5, 0, 1, 2);
	widgetLayout->addWidget(gpoWidget("3", parent), 5, 2, 1, 2);

	connect(this, &AuxAdcDacIoWidget::readRequested, this, [=, this]() { gpoManualMode->readAsync(); });

	return widget;
}

QWidget *AuxAdcDacIoWidget::gpoWidget(QString gpox, QWidget *parent)
{
	MenuSectionWidget *gpoContainer = new MenuSectionWidget(parent);

	Style::setStyle(gpoContainer, style::properties::widget::border_interactive);

	MenuCollapseSection *gpoSection = new MenuCollapseSection("GPO " + gpox, MenuCollapseSection::MHCW_NONE,
								  MenuCollapseSection::MHW_BASEWIDGET, gpoContainer);

	gpoContainer->contentLayout()->addWidget(gpoSection);

	QWidget *gpoContent = new QWidget(gpoSection);
	QGridLayout *layout = new QGridLayout(gpoContent);
	gpoContent->setLayout(layout);

	// adi,gpoX-inactive-state-high-enable
	IIOWidget *inactiveState = IIOWidgetBuilder(gpoContent)
					   .device(m_device)
					   .attribute("adi,gpo" + gpox + "-inactive-state-high-enable")
					   .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
					   .title("Enable Inactive High State")
					   .buildSingle();
	layout->addWidget(inactiveState, 0, 0);
	inactiveState->showProgressBar(false);

	// adi,gpoX-slave-rx-enable
	IIOWidget *stateRx = IIOWidgetBuilder(gpoContent)
				     .device(m_device)
				     .attribute("adi,gpo" + gpox + "-slave-rx-enable")
				     .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
				     .title("Enable RX State")
				     .buildSingle();
	layout->addWidget(stateRx, 1, 0);
	stateRx->showProgressBar(false);

	// adi,gpoX-slave-tx-enable
	IIOWidget *stateTx = IIOWidgetBuilder(gpoContent)
				     .device(m_device)
				     .attribute("adi,gpo" + gpox + "-slave-tx-enable")
				     .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
				     .title("Enable TX State")
				     .buildSingle();
	layout->addWidget(stateTx, 2, 0);
	stateTx->showProgressBar(false);

	QLabel *rxTxDelay = new QLabel("RX/TX Delay (us)", gpoContent);
	layout->addWidget(rxTxDelay, 0, 1);

	// adi,gpoX-rx-delay-us
	IIOWidget *rxDelay = IIOWidgetBuilder(gpoContent)
				     .device(m_device)
				     .attribute("adi,gpo" + gpox + "-rx-delay-us")
				     .uiStrategy(IIOWidgetBuilder::RangeUi)
				     .optionsValues("[0 1 255]")
				     .title("")
				     .buildSingle();
	layout->addWidget(rxDelay, 1, 1);

	// adi,gpoX-tx-delay-us
	IIOWidget *txDelay = IIOWidgetBuilder(gpoContent)
				     .device(m_device)
				     .attribute("adi,gpo" + gpox + "-tx-delay-us")
				     .uiStrategy(IIOWidgetBuilder::RangeUi)
				     .optionsValues("[0 1 255]")
				     .title("")
				     .buildSingle();
	layout->addWidget(txDelay, 2, 1);

	gpoSection->contentLayout()->addWidget(gpoContent);

	connect(this, &AuxAdcDacIoWidget::readRequested, this, [=, this]() {
		inactiveState->readAsync();
		stateRx->readAsync();
		stateTx->readAsync();
		rxDelay->readAsync();
		txDelay->readAsync();
	});

	return gpoContainer;
}

void AuxAdcDacIoWidget::applyGpoMask()
{

	// adi,gpo-manual-mode-enable-mask
	uint8_t mask = 0;

	// Set bits in the mask based on the checkbox states
	if(m_gpo0Mask->onOffswitch()->isChecked())
		mask |= (1 << 0); // Set bit 0
	if(m_gpo1Mask->onOffswitch()->isChecked())
		mask |= (1 << 1); // Set bit 1
	if(m_gpo2Mask->onOffswitch()->isChecked())
		mask |= (1 << 2); // Set bit 2
	if(m_gpo3Mask->onOffswitch()->isChecked())
		mask |= (1 << 3); // Set bit 3

	iio_device_debug_attr_write_longlong(m_device, "adi,gpo-manual-mode-enable-mask", mask);
}
