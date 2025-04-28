#include "ad936x.h"

#include "fastlockprofileswidget.h"
#include "firfilterqwidget.h"

#include <QLabel>
#include <QTabWidget>
#include <menucombo.h>
#include <toolbuttons.h>
#include <utils.h>
#include <QList>
#include <style.h>
#include <menuonoffswitch.h>
#include <QFutureWatcher>
#include <QtConcurrent>

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

	m_refreshButton = new AnimationPushButton(this);
	m_refreshButton->setIcon(
		Style::getPixmap(":/gui/icons/refresh.svg", Style::getColor(json::theme::content_inverse)));
	m_refreshButton->setIconSize(QSize(25, 25));
	m_refreshButton->setText("Refresh");
	m_refreshButton->setAutoDefault(true);
	Style::setStyle(m_refreshButton, style::properties::button::basicButton);
	QMovie *movie = new QMovie(":/gui/loading.gif");
	m_refreshButton->setAnimation(movie, 20000);
	m_tool->addWidgetToTopContainerHelper(m_refreshButton, TTA_RIGHT);

	connect(m_refreshButton, &QPushButton::clicked, this, [this]() {
		m_refreshButton->startAnimation();

		QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
		connect(
			watcher, &QFutureWatcher<void>::finished, this,
			[this, watcher]() {
				m_refreshButton->stopAnimation();
				watcher->deleteLater();
			},
			Qt::QueuedConnection);

		QFuture<void> future = QtConcurrent::run([this]() { Q_EMIT readRequested(); });

		watcher->setFuture(future);
	});

	QStackedWidget *centralWidget = new QStackedWidget(this);

	m_controlsWidget = new QWidget(this);
	QVBoxLayout *controlsLayout = new QVBoxLayout(m_controlsWidget);
	controlsLayout->setMargin(0);
	controlsLayout->setContentsMargins(0, 0, 0, 0);
	m_controlsWidget->setLayout(controlsLayout);

	QWidget *controlsWidget = new QWidget(this);
	QVBoxLayout *controlWidgetLayout = new QVBoxLayout(controlsWidget);
	controlWidgetLayout->setMargin(0);
	controlWidgetLayout->setContentsMargins(0, 0, 0, 0);
	controlsWidget->setLayout(controlWidgetLayout);

	QScrollArea *scrollArea = new QScrollArea(this);
	scrollArea->setWidgetResizable(true);
	scrollArea->setWidget(controlsWidget);

	controlsLayout->addWidget(scrollArea);

	///  fist widget the global settings can be created with iiowigets only
	controlWidgetLayout->addWidget(generateGlobalSettingsWidget(controlsWidget));

	/// second is Rx ( receive chain)
	controlWidgetLayout->addWidget(generateRxChainWidget(controlsWidget));

	/// third is Tx (transimt chain)
	controlWidgetLayout->addWidget(generateTxChainWidget(controlsWidget));

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
	ad963xBtn->setCheckable(true);
	ad963xBtn->setChecked(true);
	Style::setStyle(ad963xBtn, style::properties::button::blueGrayButton);
	connect(ad963xBtn, &QPushButton::clicked, this,
		[=, this]() { centralWidget->setCurrentWidget(m_controlsWidget); });

	QPushButton *blockDiagramBtn = new QPushButton("Block Diagram", this);
	blockDiagramBtn->setCheckable(true);
	Style::setStyle(blockDiagramBtn, style::properties::button::blueGrayButton);
	connect(blockDiagramBtn, &QPushButton::clicked, this,
		[=, this]() { centralWidget->setCurrentWidget(m_blockDiagramWidget); });

	centralWidgetButtons->addButton(ad963xBtn);
	centralWidgetButtons->addButton(blockDiagramBtn);

	m_tool->addWidgetToTopContainerHelper(ad963xBtn, TTA_LEFT);
	m_tool->addWidgetToTopContainerHelper(blockDiagramBtn, TTA_LEFT);
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
	Style::setStyle(globalSettingsWidget, style::properties::widget::border_interactive);

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
				      .title("ENSM Mode")
				      .uiStrategy(IIOWidgetBuilder::ComboUi)
				      .buildSingle();

	hlayout->addWidget(ensmMode);

	connect(this, &AD936X::readRequested, ensmMode, &IIOWidget::readAsync);

	////calib_mode

	IIOWidget *calibMode = IIOWidgetBuilder(globalSettingsWidget)
				       .device(plutoDevice)
				       .attribute("calib_mode")
				       .optionsAttribute("calib_mode_available")
				       .title("Calibration Mode")
				       .uiStrategy(IIOWidgetBuilder::ComboUi)

				       .buildSingle();
	hlayout->addWidget(calibMode);
	connect(this, &AD936X::readRequested, calibMode, &IIOWidget::readAsync);

	Style::setStyle(calibMode, style::properties::widget::basicBackground, true, true);

	// trx_rate_governor

	IIOWidget *trxRateGovernor = IIOWidgetBuilder(globalSettingsWidget)
					     .device(plutoDevice)
					     .attribute("trx_rate_governor")
					     .optionsAttribute("trx_rate_governor_available")
					     .title("TRX Rate Governor")
					     .uiStrategy(IIOWidgetBuilder::ComboUi)
					     .buildSingle();
	hlayout->addWidget(trxRateGovernor);
	connect(this, &AD936X::readRequested, trxRateGovernor, &IIOWidget::readAsync);

	FirFilterQWidget *firFilter = new FirFilterQWidget(plutoDevice, nullptr, globalSettingsWidget);
	hlayout->addWidget(firFilter);

	layout->addLayout(hlayout);

	// rx_path_rates

	IIOWidget *rxPathRates = IIOWidgetBuilder(globalSettingsWidget)
					 .device(plutoDevice)
					 .attribute("rx_path_rates")
					 .title("RX Path Rates")
					 .buildSingle();
	layout->addWidget(rxPathRates);
	rxPathRates->setEnabled(false);
	connect(this, &AD936X::readRequested, rxPathRates, &IIOWidget::readAsync);

	// tx_path_rates

	IIOWidget *txPathRates = IIOWidgetBuilder(globalSettingsWidget)
					 .device(plutoDevice)
					 .attribute("tx_path_rates")
					 .title("Tx Path Rates")
					 .buildSingle();
	layout->addWidget(txPathRates);
	txPathRates->setEnabled(false);
	connect(this, &AD936X::readRequested, txPathRates, &IIOWidget::readAsync);

	connect(firFilter, &FirFilterQWidget::filterChanged, this, [=, this]() {
		rxPathRates->read();
		txPathRates->read();
	});

	// xo_correction
	IIOWidget *xoCorrection = IIOWidgetBuilder(globalSettingsWidget)
					  .device(plutoDevice)
					  .attribute("xo_correction")
					  .optionsAttribute("xo_correction_available")
					  .title("XO Correction")
					  .uiStrategy(IIOWidgetBuilder::RangeUi)
					  .buildSingle();
	layout->addWidget(xoCorrection);
	connect(this, &AD936X::readRequested, xoCorrection, &IIOWidget::readAsync);

	layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	return globalSettingsWidget;
}

QWidget *AD936X::generateRxChainWidget(QWidget *parent)
{
	QWidget *rxChainWidget = new QWidget(parent);
	Style::setBackgroundColor(rxChainWidget, json::theme::background_primary);
	Style::setStyle(rxChainWidget, style::properties::widget::border_interactive);

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
					 .title("RF Bandwidth(MHz)")
					 .uiStrategy(IIOWidgetBuilder::RangeUi)
					 .buildSingle();
	hLayout->addWidget(rfBandwidth, Qt::AlignTop);
	connect(this, &AD936X::readRequested, rfBandwidth, &IIOWidget::readAsync);

	// voltage0:  sampling_frequency
	IIOWidget *samplingFrequency = IIOWidgetBuilder(rxChainWidget)
					       .channel(voltage0)
					       .attribute("sampling_frequency")
					       .optionsAttribute("sampling_frequency_available")
					       .title("Sampling Rate(MSPS)")
					       .uiStrategy(IIOWidgetBuilder::RangeUi)
					       .buildSingle();
	hLayout->addWidget(samplingFrequency);
	connect(this, &AD936X::readRequested, samplingFrequency, &IIOWidget::readAsync);

	// voltage 0 : rf_port_select
	IIOWidget *rfPortSelect = IIOWidgetBuilder(rxChainWidget)
					  .channel(voltage0)
					  .attribute("rf_port_select")
					  .optionsAttribute("rf_port_select_available")
					  .title("RF Port Select")
					  .uiStrategy(IIOWidgetBuilder::ComboUi)
					  .buildSingle();
	hLayout->addWidget(rfPortSelect);
	connect(this, &AD936X::readRequested, rfPortSelect, &IIOWidget::readAsync);

	// altvoltage0: RX_LO // frequency

	QVBoxLayout *rxLoLayout = new QVBoxLayout();

	// because this channel is marked as output by libiio we need to mark altvoltage0 as output
	iio_channel *altVoltage0 = iio_device_find_channel(plutoDevice, "altvoltage0", true);

	IIOWidget *altVoltage0Frequency = IIOWidgetBuilder(rxChainWidget)
						  .channel(altVoltage0)
						  .attribute("frequency")
						  .optionsAttribute("frequency_available")
						  .title("RX LO Frequency(MHz)")
						  .uiStrategy(IIOWidgetBuilder::RangeUi)
						  .buildSingle();
	connect(this, &AD936X::readRequested, altVoltage0Frequency, &IIOWidget::readAsync);

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

	IIOWidget *quadratureTrackingEn = IIOWidgetBuilder(rxChainWidget)
						  .channel(voltage0)
						  .attribute("quadrature_tracking_en")
						  .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
						  .title("Quadrature")
						  .buildSingle();
	trackingLayout->addWidget(quadratureTrackingEn);
	connect(this, &AD936X::readRequested, quadratureTrackingEn, &IIOWidget::readAsync);

	// rf_dc_offset_tracking_en
	IIOWidget *rcDcOffsetTrackingEn = IIOWidgetBuilder(rxChainWidget)
						  .channel(voltage0)
						  .attribute("rf_dc_offset_tracking_en")
						  .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
						  .title("RF DC")
						  .buildSingle();
	trackingLayout->addWidget(rcDcOffsetTrackingEn);
	connect(this, &AD936X::readRequested, rcDcOffsetTrackingEn, &IIOWidget::readAsync);

	// bb_dc_offset_tracking_en
	IIOWidget *bbDcOffsetTrackingEn = IIOWidgetBuilder(rxChainWidget)
						  .channel(voltage0)
						  .attribute("bb_dc_offset_tracking_en")
						  .title("BB DC")
						  .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
						  .buildSingle();
	trackingLayout->addWidget(bbDcOffsetTrackingEn);
	connect(this, &AD936X::readRequested, bbDcOffsetTrackingEn, &IIOWidget::readAsync);

	hLayout->addLayout(trackingLayout);

	layout->addLayout(hLayout);

	QHBoxLayout *rxWidgetsLayout = new QHBoxLayout();
	rxWidgetsLayout->addWidget(generateRxWidget(voltage0, rxChainWidget));

	// TODO add condition for multiple devices

	rxWidgetsLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Preferred));
	layout->addLayout(rxWidgetsLayout);

	layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	return rxChainWidget;
}

QWidget *AD936X::generateRxWidget(iio_channel *chn, QWidget *parent)
{
	QWidget *rxWidget = new QWidget(parent);
	Style::setStyle(rxWidget, style::properties::widget::border_interactive);

	QVBoxLayout *layout = new QVBoxLayout(rxWidget);
	rxWidget->setLayout(layout);

	layout->addWidget(new QLabel("RX", rxWidget));

	// TODO move to function

	// voltage0: hardwaregain
	IIOWidget *hardwaregain = IIOWidgetBuilder(rxWidget)
					  .channel(chn)
					  .attribute("hardwaregain")
					  .optionsAttribute("hardwaregain_available")
					  .title("Hardware Gain(dB)")
					  .uiStrategy(IIOWidgetBuilder::RangeUi)
					  .buildSingle();
	layout->addWidget(hardwaregain);
	connect(this, &AD936X::readRequested, hardwaregain, &IIOWidget::readAsync);

	// voltage: rssi
	IIOWidget *rssi = IIOWidgetBuilder(rxWidget).channel(chn).attribute("rssi").title("RSSI(dB)").buildSingle();
	layout->addWidget(rssi);

	// voltage: gain_control_mode
	IIOWidget *gainControlMode = IIOWidgetBuilder(rxWidget)
					     .channel(chn)
					     .attribute("gain_control_mode")
					     .optionsAttribute("gain_control_mode_available")
					     .title("Gain Control Mode")
					     .uiStrategy(IIOWidgetBuilder::ComboUi)
					     .buildSingle();
	layout->addWidget(gainControlMode);
	connect(this, &AD936X::readRequested, gainControlMode, &IIOWidget::readAsync);

	return rxWidget;
}

QWidget *AD936X::generateTxChainWidget(QWidget *parent)
{
	QWidget *txChainWidget = new QWidget(parent);
	Style::setBackgroundColor(txChainWidget, json::theme::background_primary);
	Style::setStyle(txChainWidget, style::properties::widget::border_interactive);

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
	connect(this, &AD936X::readRequested, rfBandwidth, &IIOWidget::readAsync);

	// voltage0:  sampling_frequency

	IIOWidget *samplingFrequency = IIOWidgetBuilder(txChainWidget)
					       .channel(voltage0)
					       .attribute("sampling_frequency")
					       .optionsAttribute("sampling_frequency_available")
					       .uiStrategy(IIOWidgetBuilder::RangeUi)
					       .buildSingle();
	hLayout->addWidget(samplingFrequency);
	connect(this, &AD936X::readRequested, samplingFrequency, &IIOWidget::readAsync);

	// voltage0:  rf_port_select
	IIOWidget *rfPortSelect = IIOWidgetBuilder(txChainWidget)
					  .channel(voltage0)
					  .attribute("rf_port_select")
					  .optionsAttribute("rf_port_select_available")
					  .uiStrategy(IIOWidgetBuilder::ComboUi)
					  .buildSingle();
	hLayout->addWidget(rfPortSelect);
	connect(this, &AD936X::readRequested, rfPortSelect, &IIOWidget::readAsync);

	// altvoltage1: TX_LO // frequency

	iio_channel *altVoltage1 = iio_device_find_channel(plutoDevice, "altvoltage1", isOutput);

	QVBoxLayout *txLoLayout = new QVBoxLayout();

	IIOWidget *altVoltage1Frequency = IIOWidgetBuilder(txChainWidget)
						  .channel(altVoltage1)
						  .attribute("frequency")
						  .optionsAttribute("frequency_available")
						  .uiStrategy(IIOWidgetBuilder::RangeUi)
						  .buildSingle();
	connect(this, &AD936X::readRequested, altVoltage1Frequency, &IIOWidget::readAsync);

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

	QHBoxLayout *txWidgetsLayout = new QHBoxLayout();
	txWidgetsLayout->addWidget(generateTxWidget(plutoDevice, txChainWidget));

	// TODO add condition for multiple devices

	txWidgetsLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Preferred));
	layout->addLayout(txWidgetsLayout);

	layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	return txChainWidget;
}

QWidget *AD936X::generateTxWidget(iio_device *dev, QWidget *parent)
{
	QWidget *txWidget = new QWidget(parent);
	Style::setStyle(txWidget, style::properties::widget::border_interactive);

	QVBoxLayout *layout = new QVBoxLayout(txWidget);
	txWidget->setLayout(layout);

	layout->addWidget(new QLabel("TX", txWidget));

	// adi,tx-attenuation-mdB
	IIOWidget *txAttenuation = IIOWidgetBuilder(txWidget)
					   .device(dev)
					   .attribute("adi,tx-attenuation-mdB")
					   .uiStrategy(IIOWidgetBuilder::RangeUi)
					   .title("Attenuation(dB")
					   .buildSingle();
	layout->addWidget(txAttenuation);
	connect(this, &AD936X::readRequested, txAttenuation, &IIOWidget::readAsync);

	bool isOutput = true;
	iio_channel *voltage0 = iio_device_find_channel(dev, "voltage0", isOutput);
	// voltage0: rssi
	IIOWidget *rssi =
		IIOWidgetBuilder(txWidget).channel(voltage0).attribute("rssi").title("RSSI(dB)").buildSingle();
	layout->addWidget(rssi);
	connect(this, &AD936X::readRequested, rssi, &IIOWidget::readAsync);

	return txWidget;
}
