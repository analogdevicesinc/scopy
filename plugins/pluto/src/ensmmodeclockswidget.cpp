#include "ensmmodeclockswidget.h"

#include <QLabel>
#include <iiowidgetbuilder.h>
#include <style.h>
#include <iioutil/connectionprovider.h>

using namespace scopy;
using namespace pluto;

EnsmModeClocksWidget::EnsmModeClocksWidget(QString uri, QWidget *parent)
	: m_uri(uri)
	, QWidget{parent}
{

	Style::setBackgroundColor(this, json::theme::background_primary);

	m_layout = new QVBoxLayout(this);
	m_layout->setMargin(0);
	m_layout->setSpacing(10);
	m_layout->setContentsMargins(0, 0, 0, 0);
	setLayout(m_layout);

	// Get connection to device
	Connection *conn = ConnectionProvider::GetInstance()->open(m_uri);
	// iio:device0: ad9361-phy
	m_device = iio_context_find_device(conn->context(), "ad9361-phy");

	m_layout->addWidget(generateEnsmModeWidget(this));
	m_layout->addWidget(generateModeWidget(this));
	m_layout->addWidget(generateClocksWidget(this));

	m_layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));
}

EnsmModeClocksWidget::~EnsmModeClocksWidget()
{
	// close Connection
	ConnectionProvider::close(m_uri);
}

QWidget *EnsmModeClocksWidget::generateEnsmModeWidget(QWidget *parent)
{
	QWidget *ensmModeWidget = new QWidget(parent);
	Style::setStyle(ensmModeWidget, style::properties::widget::border_interactive);

	QVBoxLayout *ensmModeWidgetLayout = new QVBoxLayout(ensmModeWidget);
	ensmModeWidget->setLayout(ensmModeWidgetLayout);

	QLabel *title = new QLabel("ENSM Mode", ensmModeWidget);
	Style::setStyle(title, style::properties::label::menuBig);
	ensmModeWidgetLayout->addWidget(title);

	// ensm_mode  ensm_mode_available
	IIOWidget *fddMode = IIOWidgetBuilder(ensmModeWidget)
				     .device(m_device)
				     .attribute("adi,frequency-division-duplex-mode-enable")
				     .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
				     .title("FDD/TDD")
				     .buildSingle();
	ensmModeWidgetLayout->addWidget(fddMode);
	fddMode->setToolTip("Use FDD mode - default TDD");

	// adi,ensm-enable-pin-pulse-mode-enable
	IIOWidget *ensmEnablePinPluseMode = IIOWidgetBuilder(ensmModeWidget)
						    .device(m_device)
						    .attribute("adi,ensm-enable-pin-pulse-mode-enable")
						    .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
						    .title("Pin Pluse Mode")
						    .buildSingle();
	ensmModeWidgetLayout->addWidget(ensmEnablePinPluseMode);
	ensmEnablePinPluseMode->setToolTip("ENSM control Pins (ENABLE/TXNRX) use Pulse mode - default Level Mode");

	// adi,ensm-enable-txnrx-control-enable
	IIOWidget *ensmEnableTxnrx = IIOWidgetBuilder(ensmModeWidget)
					     .device(m_device)
					     .attribute("adi,ensm-enable-txnrx-control-enable")
					     .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
					     .title("TXNRX Pin Control")
					     .buildSingle();
	ensmModeWidgetLayout->addWidget(ensmEnableTxnrx);
	ensmEnableTxnrx->setToolTip("ENSM control Pins (ENABLE/TXNRX) control ENSM state - default SPI writes");

	QLabel *tddModeLabel = new QLabel("TDD Mode", ensmModeWidget);

	ensmModeWidgetLayout->addWidget(tddModeLabel);

	QHBoxLayout *ensmHBoxLayout = new QHBoxLayout();

	// adi,tdd-use-dual-synth-mode-enable
	IIOWidget *useDualSynth = IIOWidgetBuilder(ensmModeWidget)
					  .device(m_device)
					  .attribute("adi,tdd-use-dual-synth-mode-enable")
					  .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
					  .title("Use Dual Synth")
					  .buildSingle();
	ensmHBoxLayout->addWidget(useDualSynth);
	useDualSynth->setToolTip("In TDD mode use Dual Synth mode - default only one Synth is enabled");

	// adi,tdd-use-fdd-vco-tables-enable
	IIOWidget *useFddVcoTable = IIOWidgetBuilder(ensmModeWidget)
					    .device(m_device)
					    .attribute("adi,tdd-use-fdd-vco-tables-enable")
					    .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
					    .title("Use FDD VCO tables")
					    .buildSingle();
	ensmHBoxLayout->addWidget(useFddVcoTable);

	// adi,tdd-skip-vco-cal-enable
	IIOWidget *skipVcoCal = IIOWidgetBuilder(ensmModeWidget)
					.device(m_device)
					.attribute("adi,tdd-skip-vco-cal-enable")
					.uiStrategy(IIOWidgetBuilder::CheckBoxUi)
					.title("Skip VCO cal")
					.buildSingle();
	ensmHBoxLayout->addWidget(skipVcoCal);
	skipVcoCal->setToolTip("Option to skip VCO cal in TDD mode when moving from TX/RX to Alert");

	// adi,update-tx-gain-in-alert-enable
	IIOWidget *updateTxGainInAlert = IIOWidgetBuilder(ensmModeWidget)
						 .device(m_device)
						 .attribute("adi,update-tx-gain-in-alert-enable")
						 .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
						 .title("Update Tx Gain in ALERT")
						 .buildSingle();
	ensmHBoxLayout->addWidget(updateTxGainInAlert);
	updateTxGainInAlert->setToolTip("in TDD mode disable immediate TX Gain update and wait until ENSM moves to Alert");

	ensmModeWidgetLayout->addLayout(ensmHBoxLayout);

	connect(this, &EnsmModeClocksWidget::readRequested, this, [=, this](){
		fddMode->readAsync();
		ensmEnablePinPluseMode->readAsync();
		ensmEnableTxnrx->readAsync();
		useDualSynth->readAsync();
		useFddVcoTable->readAsync();
		skipVcoCal->readAsync();
		updateTxGainInAlert->readAsync();
	});


	return ensmModeWidget;
}

QWidget *EnsmModeClocksWidget::generateModeWidget(QWidget *parent)
{
	QWidget *modeWidget = new QWidget(parent);
	Style::setStyle(modeWidget, style::properties::widget::border_interactive);

	QVBoxLayout *modeWidgetLayout = new QVBoxLayout(modeWidget);
	modeWidget->setLayout(modeWidgetLayout);

	QLabel *modeTitle = new QLabel("Mode", modeWidget);
	Style::setStyle(modeTitle, style::properties::label::menuBig);
	modeWidgetLayout->addWidget(modeTitle);

	// adi,rx-rf-port-input-select
	IIOWidget *rxPortInput = IIOWidgetBuilder(modeWidget)
					 .device(m_device)
					 .attribute("adi,rx-rf-port-input-select")
					 .title("RX port input")
					 .buildSingle();
	modeWidgetLayout->addWidget(rxPortInput);

	// adi,tx-rf-port-input-select
	IIOWidget *txPortInput = IIOWidgetBuilder(modeWidget)
					 .device(m_device)
					 .attribute("adi,tx-rf-port-input-select")
					 .title("TX port input")
					 .buildSingle();
	modeWidgetLayout->addWidget(txPortInput);

	// adi,rx1-rx2-phase-inversion-enable
	IIOWidget *rx1Rx2Phase = IIOWidgetBuilder(modeWidget)
					 .device(m_device)
					 .attribute("adi,rx1-rx2-phase-inversion-enable")
					 .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
					 .title("RX2 Phase Inversion")
					 .buildSingle();
	modeWidgetLayout->addWidget(rx1Rx2Phase);
	rx1Rx2Phase->setToolTip("If enabled RX1 and RX2 are phase aligned");

	connect(this, &EnsmModeClocksWidget::readRequested, this, [=, this](){
		rxPortInput->readAsync();
		txPortInput->readAsync();
		rx1Rx2Phase->readAsync();
	});

	return modeWidget;
}

QWidget *EnsmModeClocksWidget::generateClocksWidget(QWidget *parent)
{
	QWidget *clocksWidget = new QWidget(parent);
	Style::setStyle(clocksWidget, style::properties::widget::border_interactive);

	QGridLayout *clocksWidgetLayout = new QGridLayout(clocksWidget);
	clocksWidget->setLayout(clocksWidgetLayout);

	QLabel *clocksTitle = new QLabel("Clocks", clocksWidget);
	Style::setStyle(clocksTitle, style::properties::label::menuBig);
	clocksWidgetLayout->addWidget(clocksTitle, 0, 0);

	// adi,xo-disable-use-ext-refclk-enable
	IIOWidget *xoDisableUseExtRefclk = IIOWidgetBuilder(clocksWidget)
						   .device(m_device)
						   .attribute("adi,xo-disable-use-ext-refclk-enable")
						   .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
						   .title("XO Disable use EXT RefCLK")
						   .buildSingle();
	clocksWidgetLayout->addWidget(xoDisableUseExtRefclk, 1, 0);
	xoDisableUseExtRefclk->setToolTip("Disable XO use Ext CLK into XTAL_N - default XO into XTAL");

	// adi,external-rx-lo-enable
	IIOWidget *externalRxLo = IIOWidgetBuilder(clocksWidget)
					 .device(m_device)
					 .attribute("adi,external-rx-lo-enable")
					 .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
					 .title("Ext RX LO")
					 .buildSingle();
	clocksWidgetLayout->addWidget(externalRxLo, 2, 0);
	externalRxLo->setToolTip("Enables external LO for RX");

	// adi,clk-output-mode-select
	IIOWidget *clkOutputMode = IIOWidgetBuilder(clocksWidget)
					   .device(m_device)
					   .attribute("adi,clk-output-mode-select")
					   // .uiStrategy(IIOWidgetBuilder::ComboUi)
					   .title("CLOCKOUT")
					   .buildSingle();
	clocksWidgetLayout->addWidget(clkOutputMode, 2, 1);

	// adi,external-tx-lo-enable
	IIOWidget *externalTxLo = IIOWidgetBuilder(clocksWidget)
					 .device(m_device)
					 .attribute("adi,external-tx-lo-enable")
					 .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
					 .title("Ext TX LO")
					 .buildSingle();
	clocksWidgetLayout->addWidget(externalTxLo, 3, 0);
	externalTxLo->setToolTip("Enables external LO for TX");

	// adi,rx-fastlock-pincontrol-enable
	IIOWidget *rxFastlockPincontrol = IIOWidgetBuilder(clocksWidget)
						  .device(m_device)
						  .attribute("adi,rx-fastlock-pincontrol-enable")
						  .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
						  .title("RX Fastlock Pin Control")
						  .buildSingle();
	clocksWidgetLayout->addWidget(rxFastlockPincontrol, 4, 0);
	rxFastlockPincontrol->setToolTip("RX fastlock pin control enable");

	// adi,rx-fastlock-delay-ns
	IIOWidget *rxFastLockDelay = IIOWidgetBuilder(clocksWidget)
					     .device(m_device)
					     .attribute("adi,rx-fastlock-delay-ns")
					     .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
					     .title("RX Fastlock Delay")
					     .buildSingle();
	clocksWidgetLayout->addWidget(rxFastLockDelay, 4, 1);
	rxFastLockDelay->setToolTip("RX fastlock delay in ns");

	// adi,tx-fastlock-pincontrol-enable
	IIOWidget *txFastlockPincontrol = IIOWidgetBuilder(clocksWidget)
						  .device(m_device)
						  .attribute("adi,tx-fastlock-pincontrol-enable")
	                                          .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
						  .title("TX Fastlock Pin Control")
						  .buildSingle();
	clocksWidgetLayout->addWidget(txFastlockPincontrol, 5, 0);
	txFastlockPincontrol->setToolTip("TX fastlock pin control enable");

	// adi,tx-fastlock-delay-ns
	IIOWidget *txFastLockDelay = IIOWidgetBuilder(clocksWidget)
					     .device(m_device)
					     .attribute("adi,tx-fastlock-delay-ns")
					     .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
					     .title("TX Fastlock Delay")
					     .buildSingle();
	clocksWidgetLayout->addWidget(txFastLockDelay, 5, 1);
	txFastLockDelay->setToolTip("TX fastlock delay in ns");

	connect(this, &EnsmModeClocksWidget::readRequested, this, [=, this](){
		xoDisableUseExtRefclk->readAsync();
		externalRxLo->readAsync();
		clkOutputMode->readAsync();
		externalTxLo->readAsync();
		rxFastlockPincontrol->readAsync();
		rxFastLockDelay->readAsync();
		txFastlockPincontrol->readAsync();
		txFastLockDelay->readAsync();
	});

	return clocksWidget;
}
