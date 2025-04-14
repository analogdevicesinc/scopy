#include "ad936x.h"

#include "fastlockprofileswidget.h"
#include "firfilterqidget.h"

#include <QLabel>
#include <QTabWidget>
#include <menucombo.h>
#include <toolbuttons.h>
#include <utils.h>
#include <QList>
#include <style.h>
#include <menuonoffswitch.h>

#include <iioutil/connectionprovider.h>

using namespace scopy;
using namespace pluto;

AD936X::AD936X(QString uri, QWidget *parent)
	: QWidget(parent)
	, m_uri(uri)
{

	m_mainLayout = new QVBoxLayout(this);
	m_mainLayout->setMargin(0);
	m_mainLayout->setContentsMargins(0, 0, 0, 0);

	m_tool = new ToolTemplate(this);
	m_tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_tool->topContainer()->setVisible(true);
	m_tool->topContainerMenuControl()->setVisible(false);

	m_mainLayout->addWidget(m_tool);

	///////// info button /////////////////
	InfoBtn *infoBtn = new InfoBtn(this, true);
	m_tool->addWidgetToTopContainerHelper(infoBtn, TTA_LEFT);

	connect(infoBtn, &InfoBtn::clicked, this, [=, this]() {
		// infoBtn->generateInfoPopup(this);
		// TODO
		// connect(infoBtn->getTutorialButton(), &QPushButton::clicked, this, [=]() {
		// 	if(searchBarWidget->isVisible()) {
		// 		startTutorial();
		// 	} else {
		// 		startSimpleTutorial();
		// 	}
		// });

		// connect(infoBtn->getDocumentationButton(), &QAbstractButton::clicked, this, [=]() {
		// 	QDesktopServices::openUrl(
		// 		QUrl("https://analogdevicesinc.github.io/scopy/plugins/registermap/registermap.html"));
		// });
	});

	QStackedWidget *centralWidget = new QStackedWidget(this);

	m_controlsWidget = new QWidget(this);
	QVBoxLayout *controlsLayout = new QVBoxLayout(m_controlsWidget);
	m_controlsWidget->setLayout(controlsLayout);

	/// this tab will have 4 main wigets each of them in a separate class ??

	QWidget *controlsWidget = new QWidget(this);
	QVBoxLayout *controlWidgetLayout = new QVBoxLayout(controlsWidget);
	controlsWidget->setLayout(controlWidgetLayout);

	QScrollArea *scrollArea = new QScrollArea(this);
	scrollArea->setWidgetResizable(true);
	scrollArea->setWidget(controlsWidget);

	controlsLayout->addWidget(scrollArea);

	///  fist widget the global settings can be created with iiowigets only
	controlWidgetLayout->addWidget(generateGlobalSettingsWidget(controlsWidget));

	/// second is Rx ( receive chain)
	controlWidgetLayout->addWidget(generateRxWidget(controlsWidget));

	/// third is Tx (transimt chain)
	controlWidgetLayout->addWidget(generateTxWidget(controlsWidget));

	controlWidgetLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	m_blockDiagramWidget = new QWidget(this);
	Style::setBackgroundColor(m_blockDiagramWidget, json::theme::background_primary);
	QVBoxLayout *blockDiagramLayout = new QVBoxLayout(m_blockDiagramWidget);
	m_blockDiagramWidget->setLayout(blockDiagramLayout);

	QWidget *blockDiagramWidget = new QWidget(this);
	QVBoxLayout *blockDiagramWidgetLayout = new QVBoxLayout(blockDiagramWidget);
	blockDiagramWidget->setLayout(blockDiagramWidgetLayout);

	QScrollArea *blockDiagramWidgetScrollArea = new QScrollArea(this);
	blockDiagramWidgetScrollArea->setWidgetResizable(true);
	blockDiagramWidgetScrollArea->setWidget(blockDiagramWidget);

	blockDiagramLayout->addWidget(blockDiagramWidgetScrollArea);

	QLabel *blockDiagram = new QLabel(m_blockDiagramWidget);
	blockDiagramWidgetLayout->addWidget(blockDiagram);
	blockDiagram->setAlignment(Qt::AlignCenter);
	QPixmap pixmap(":/pluto/ad936x.svg"); // Use the resource path
	blockDiagram->setPixmap(pixmap);

	centralWidget->addWidget(m_controlsWidget);
	centralWidget->addWidget(m_blockDiagramWidget);

	m_tool->addWidgetToCentralContainerHelper(centralWidget);

	QButtonGroup *centralWidgetButtons = new QButtonGroup(this);
	centralWidgetButtons->setExclusive(true);

	QPushButton *ad963xBtn = new QPushButton("Controls", this);
	Style::setStyle(ad963xBtn, style::properties::button::basicButton);
	connect(ad963xBtn, &QPushButton::clicked, this,
		[=, this]() { centralWidget->setCurrentWidget(m_controlsWidget); });

	QPushButton *blockDiagramBtn = new QPushButton("Block Diagram", this);
	Style::setStyle(blockDiagramBtn, style::properties::button::basicButton);
	connect(blockDiagramBtn, &QPushButton::clicked, this,
		[=, this]() { centralWidget->setCurrentWidget(m_blockDiagramWidget); });

	centralWidgetButtons->addButton(ad963xBtn);
	centralWidgetButtons->addButton(blockDiagramBtn);

	m_tool->addWidgetToTopContainerHelper(ad963xBtn, TTA_LEFT);
	m_tool->addWidgetToTopContainerHelper(blockDiagramBtn, TTA_LEFT);

	// Style::setStyle(scrollWidget, style::properties::widget::border_interactive, true, true);
}

AD936X::~AD936X()
{
	// close Connection
	ConnectionProvider::close(m_uri);
}

QWidget *AD936X::generateGlobalSettingsWidget(QWidget *parent)
{
	QWidget *globalSettingsWidget = new QWidget(parent);
	Style::setBackgroundColor(globalSettingsWidget, json::theme::background_primary);
	QVBoxLayout *layout = new QVBoxLayout(globalSettingsWidget);
	globalSettingsWidget->setLayout(layout);

	layout->addWidget(new QLabel("AD9361 / AD9364 Global Settings", globalSettingsWidget));

	// Get connection to device
	Connection *conn = ConnectionProvider::open(m_uri);

	iio_device *plutoDevice = iio_context_find_device(conn->context(), "ad9361-phy");

	QHBoxLayout *hlayout = new QHBoxLayout();

	//// ensm_mode
	IIOWidget *ensmMode = IIOWidgetBuilder(globalSettingsWidget)
				      .device(plutoDevice)
				      .attribute("ensm_mode")
				      .optionsAttribute("ensm_mode_available")
				      .uiStrategy(IIOWidgetBuilder::ComboUi)
				      .buildSingle();

	hlayout->addWidget(ensmMode);

	////calib_mode

	IIOWidget *calibMode = IIOWidgetBuilder(globalSettingsWidget)
				       .device(plutoDevice)
				       .attribute("calib_mode")
				       .optionsAttribute("calib_mode_available")
				       .uiStrategy(IIOWidgetBuilder::ComboUi)

				       .buildSingle();
	hlayout->addWidget(calibMode);
	Style::setStyle(calibMode, style::properties::widget::basicBackground, true, true);

	// trx_rate_governor

	IIOWidget *trxRateGovernor = IIOWidgetBuilder(globalSettingsWidget)
					     .device(plutoDevice)
					     .attribute("trx_rate_governor")
					     .optionsAttribute("trx_rate_governor_available")
					     .uiStrategy(IIOWidgetBuilder::ComboUi)
					     .buildSingle();
	hlayout->addWidget(trxRateGovernor);

	FirFilterQidget *firFilter = new FirFilterQidget(plutoDevice, nullptr, globalSettingsWidget);
	hlayout->addWidget(firFilter);

	layout->addLayout(hlayout);

	// rx_path_rates

	IIOWidget *rxPathRates =
		IIOWidgetBuilder(globalSettingsWidget).device(plutoDevice).attribute("rx_path_rates").buildSingle();
	layout->addWidget(rxPathRates);

	// tx_path_rates

	IIOWidget *txPathRates =
		IIOWidgetBuilder(globalSettingsWidget).device(plutoDevice).attribute("tx_path_rates").buildSingle();

	layout->addWidget(txPathRates);

	connect(firFilter, &FirFilterQidget::filterChangeWasMade, this, [=, this]() {
		rxPathRates->read();
		txPathRates->read();
	});

	// xo_correction
	IIOWidget *xoCorrection = IIOWidgetBuilder(globalSettingsWidget)
					  .device(plutoDevice)
					  .attribute("xo_correction")
					  .optionsAttribute("xo_correction_available")
					  .uiStrategy(IIOWidgetBuilder::RangeUi)
					  .buildSingle();
	layout->addWidget(xoCorrection);

	/// close device connection
	// ConnectionProvider::close(m_uri);

	return globalSettingsWidget;
}

QWidget *AD936X::generateRxWidget(QWidget *parent)
{
	QWidget *rxChainWidget = new QWidget(parent);
	Style::setBackgroundColor(rxChainWidget, json::theme::background_primary);
	QVBoxLayout *layout = new QVBoxLayout(rxChainWidget);
	rxChainWidget->setLayout(layout);

	layout->addWidget(new QLabel("AD9361 / AD9364 Receive Chain", rxChainWidget));

	// Get connection to device
	Connection *conn = ConnectionProvider::open(m_uri);
	iio_device *plutoDevice = iio_context_find_device(conn->context(), "ad9361-phy");

	QHBoxLayout *hLayout = new QHBoxLayout();

	// voltage0: rf_bandwidth

	bool isOutput = false;

	iio_channel *voltage0 = iio_device_find_channel(plutoDevice, "voltage0", isOutput);

	IIOWidget *rfBandwidth = IIOWidgetBuilder(rxChainWidget)
					 .channel(voltage0)
					 .attribute("rf_bandwidth")
					 .optionsAttribute("rf_bandwidth_available")
					 .uiStrategy(IIOWidgetBuilder::RangeUi)
					 .buildSingle();
	hLayout->addWidget(rfBandwidth);

	// voltage0:  sampling_frequency

	IIOWidget *samplingFrequency = IIOWidgetBuilder(rxChainWidget)
					       .channel(voltage0)
					       .attribute("sampling_frequency")
					       .optionsAttribute("sampling_frequency_available")
					       .uiStrategy(IIOWidgetBuilder::RangeUi)
					       .buildSingle();
	hLayout->addWidget(samplingFrequency);

	// voltage 0 : rf_port_select

	IIOWidget *rfPortSelect = IIOWidgetBuilder(rxChainWidget)
					  .channel(voltage0)
					  .attribute("rf_port_select")
					  .optionsAttribute("rf_port_select_available")
					  .uiStrategy(IIOWidgetBuilder::ComboUi)
					  .buildSingle();
	hLayout->addWidget(rfPortSelect);

	// altvoltage0: RX_LO // frequency

	QVBoxLayout *rxLoLayout = new QVBoxLayout();

	// because this channel is marked as output by libiio we need to mark altvoltage0 as output
	iio_channel *altVoltage0 = iio_device_find_channel(plutoDevice, "altvoltage0", true);

	IIOWidget *altVoltage0Frequency = IIOWidgetBuilder(rxChainWidget)
						  .channel(altVoltage0)
						  .attribute("frequency")
						  .optionsAttribute("frequency_available")
						  .uiStrategy(IIOWidgetBuilder::RangeUi)
						  .buildSingle();

	MenuOnOffSwitch *useExternalRxLo = new MenuOnOffSwitch("External Rx LO", rxChainWidget, false);
	useExternalRxLo->onOffswitch()->setChecked(true);

	rxLoLayout->addWidget(altVoltage0Frequency);
	rxLoLayout->addWidget(useExternalRxLo);

	hLayout->addLayout(rxLoLayout);

	// fastlock profile

	FastlockProfilesWidget *fastlockProfile = new FastlockProfilesWidget(altVoltage0, rxChainWidget);

	connect(useExternalRxLo->onOffswitch(), &QAbstractButton::toggled, this,
		[=, this](bool toggled) { fastlockProfile->setEnabled(toggled); });

	hLayout->addWidget(fastlockProfile);

	connect(fastlockProfile, &FastlockProfilesWidget::recallCalled, this,
		[=, this] { altVoltage0Frequency->read(); });

	QVBoxLayout *trackingLayout = new QVBoxLayout();

	// quadrature_tracking_en

	IIOWidget *quadratureTrackingEn =
		IIOWidgetBuilder(rxChainWidget).channel(voltage0).attribute("quadrature_tracking_en").buildSingle();
	trackingLayout->addWidget(quadratureTrackingEn);

	// rf_dc_offset_tracking_en
	IIOWidget *rcDcOffsetTrackingEn =
		IIOWidgetBuilder(rxChainWidget).channel(voltage0).attribute("rf_dc_offset_tracking_en").buildSingle();
	trackingLayout->addWidget(rcDcOffsetTrackingEn);

	// bb_dc_offset_tracking_en
	IIOWidget *bbDcOffsetTrackingEn =
		IIOWidgetBuilder(rxChainWidget).channel(voltage0).attribute("bb_dc_offset_tracking_en").buildSingle();
	trackingLayout->addWidget(bbDcOffsetTrackingEn);

	hLayout->addLayout(trackingLayout);

	layout->addLayout(hLayout);

	// voltage0: hardwaregain
	IIOWidget *hardwaregain = IIOWidgetBuilder(rxChainWidget)
					  .channel(voltage0)
					  .attribute("hardwaregain")
					  .optionsAttribute("hardwaregain_available")
					  .uiStrategy(IIOWidgetBuilder::RangeUi)
					  .buildSingle();
	layout->addWidget(hardwaregain);

	// voltage0: rssi
	IIOWidget *rssi = IIOWidgetBuilder(rxChainWidget).channel(voltage0).attribute("rssi").buildSingle();
	layout->addWidget(rssi);

	// voltage0: gain_control_mode
	IIOWidget *gainControlMode = IIOWidgetBuilder(rxChainWidget)
					     .channel(voltage0)
					     .attribute("gain_control_mode")
					     .optionsAttribute("gain_control_mode_available")
					     .uiStrategy(IIOWidgetBuilder::ComboUi)
					     .buildSingle();
	layout->addWidget(gainControlMode);

	/// close device connection
	// ConnectionProvider::close(m_uri);

	return rxChainWidget;
}

QWidget *AD936X::generateTxWidget(QWidget *parent)
{

	QWidget *txChainWidget = new QWidget(parent);
	Style::setBackgroundColor(txChainWidget, json::theme::background_primary);
	QVBoxLayout *layout = new QVBoxLayout(txChainWidget);
	txChainWidget->setLayout(layout);

	layout->addWidget(new QLabel("AD9361 / AD9364 Transmit Chain", txChainWidget));

	// Get connection to device
	Connection *conn = ConnectionProvider::open(m_uri);
	iio_device *plutoDevice = iio_context_find_device(conn->context(), "ad9361-phy");

	QHBoxLayout *hLayout = new QHBoxLayout();

	bool isOutput = true;
	iio_channel *voltage0 = iio_device_find_channel(plutoDevice, "voltage0", isOutput);

	// voltage0: rf_bandwidth

	IIOWidget *rfBandwidth = IIOWidgetBuilder(txChainWidget)
					 .channel(voltage0)
					 .attribute("rf_bandwidth")
					 .optionsAttribute("rf_bandwidth_available")
					 .uiStrategy(IIOWidgetBuilder::RangeUi)
					 .buildSingle();
	hLayout->addWidget(rfBandwidth);
	rfBandwidth->read();

	// voltage0:  sampling_frequency

	IIOWidget *samplingFrequency = IIOWidgetBuilder(txChainWidget)
					       .channel(voltage0)
					       .attribute("sampling_frequency")
					       .optionsAttribute("sampling_frequency_available")
					       .uiStrategy(IIOWidgetBuilder::RangeUi)
					       .buildSingle();
	hLayout->addWidget(samplingFrequency);
	samplingFrequency->read();

	// voltage0:  rf_port_select
	IIOWidget *rfPortSelect = IIOWidgetBuilder(txChainWidget)
					  .channel(voltage0)
					  .attribute("rf_port_select")
					  .optionsAttribute("rf_port_select_available")
					  .uiStrategy(IIOWidgetBuilder::ComboUi)
					  .buildSingle();
	hLayout->addWidget(rfPortSelect);

	// altvoltage1: TX_LO // frequency

	iio_channel *altVoltage1 = iio_device_find_channel(plutoDevice, "altvoltage1", isOutput);

	QVBoxLayout *txLoLayout = new QVBoxLayout();

	IIOWidget *altVoltage1Frequency = IIOWidgetBuilder(txChainWidget)
						  .channel(altVoltage1)
						  .attribute("frequency")
						  .optionsAttribute("frequency_available")
						  .uiStrategy(IIOWidgetBuilder::RangeUi)
						  .buildSingle();

	MenuOnOffSwitch *useExternalTxLo = new MenuOnOffSwitch("External Tx LO", txChainWidget, false);
	useExternalTxLo->onOffswitch()->setChecked(true);

	txLoLayout->addWidget(altVoltage1Frequency);
	txLoLayout->addWidget(useExternalTxLo);

	hLayout->addLayout(txLoLayout);

	// fastlock profile
	FastlockProfilesWidget *fastlockProfile = new FastlockProfilesWidget(altVoltage1, txChainWidget);

	connect(useExternalTxLo->onOffswitch(), &QAbstractButton::toggled, this,
		[=, this](bool toggled) { fastlockProfile->setEnabled(toggled); });

	hLayout->addWidget(fastlockProfile);

	connect(fastlockProfile, &FastlockProfilesWidget::recallCalled, this,
		[=, this] { altVoltage1Frequency->read(); });

	layout->addLayout(hLayout);

	// adi,tx-attenuation-mdB
	IIOWidget *txAttenuation =
		IIOWidgetBuilder(txChainWidget).device(plutoDevice).attribute("adi,tx-attenuation-mdB").buildSingle();
	layout->addWidget(txAttenuation);

	// voltage0: rssi
	IIOWidget *rssi = IIOWidgetBuilder(txChainWidget).channel(voltage0).attribute("rssi").buildSingle();
	layout->addWidget(rssi);

	// ConnectionProvider::close(m_uri);

	return txChainWidget;
}
