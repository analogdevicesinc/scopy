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
	ensmModeWidgetLayout->addWidget(title);

	// ensm_mode  ensm_mode_available
	IIOWidget *ensmMode = IIOWidgetBuilder(ensmModeWidget)
				      .device(m_device)
				      .attribute("ensm_mode")
				      .optionsAttribute("ensm_mode_available")
				      .uiStrategy(IIOWidgetBuilder::ComboUi)
				      .buildSingle();
	ensmModeWidgetLayout->addWidget(ensmMode);

	// adi,ensm-enable-pin-pulse-mode-enable
	IIOWidget *ensmEnablePinPluseMode = IIOWidgetBuilder(ensmModeWidget)
						    .device(m_device)
						    .attribute("adi,ensm-enable-pin-pulse-mode-enable")
						    .buildSingle();
	ensmModeWidgetLayout->addWidget(ensmEnablePinPluseMode);

	// adi,ensm-enable-txnrx-control-enable
	IIOWidget *ensmEnableTxnrx = IIOWidgetBuilder(ensmModeWidget)
					     .device(m_device)
					     .attribute("adi,ensm-enable-txnrx-control-enable")
					     .buildSingle();
	ensmModeWidgetLayout->addWidget(ensmEnableTxnrx);

	QLabel *tddModeLabel = new QLabel("TDD Mode", ensmModeWidget);

	ensmModeWidgetLayout->addWidget(tddModeLabel);

	QHBoxLayout *ensmHBoxLayout = new QHBoxLayout();

	// adi,tdd-use-dual-synth-mode-enable
	IIOWidget *useDualSynth = IIOWidgetBuilder(ensmModeWidget)
					  .device(m_device)
					  .attribute("adi,tdd-use-dual-synth-mode-enable")
					  .buildSingle();
	ensmHBoxLayout->addWidget(useDualSynth);

	// adi,tdd-use-fdd-vco-tables-enable
	IIOWidget *useFddVcoTable = IIOWidgetBuilder(ensmModeWidget)
					    .device(m_device)
					    .attribute("adi,tdd-use-fdd-vco-tables-enable")
					    .buildSingle();
	ensmHBoxLayout->addWidget(useFddVcoTable);

	// adi,tdd-skip-vco-cal-enable
	IIOWidget *skipCvoCal = IIOWidgetBuilder(ensmModeWidget)
					.device(m_device)
					.attribute("adi,tdd-skip-vco-cal-enable")
					.buildSingle();
	ensmHBoxLayout->addWidget(skipCvoCal);

	// adi,update-tx-gain-in-alert-enable
	IIOWidget *updateTxGainInAlert = IIOWidgetBuilder(ensmModeWidget)
						 .device(m_device)
						 .attribute("adi,update-tx-gain-in-alert-enable")
						 .buildSingle();
	ensmHBoxLayout->addWidget(updateTxGainInAlert);

	ensmModeWidgetLayout->addLayout(ensmHBoxLayout);

	return ensmModeWidget;
}

QWidget *EnsmModeClocksWidget::generateModeWidget(QWidget *parent)
{
	QWidget *modeWidget = new QWidget(parent);
	Style::setStyle(modeWidget, style::properties::widget::border_interactive);

	QVBoxLayout *modeWidgetLayout = new QVBoxLayout(modeWidget);
	modeWidget->setLayout(modeWidgetLayout);

	QLabel *modeTitle = new QLabel("Mode", modeWidget);
	modeWidgetLayout->addWidget(modeTitle);

	// adi,rx-rf-port-input-select
	IIOWidget *rxPortInput =
		IIOWidgetBuilder(modeWidget).device(m_device).attribute("adi,rx-rf-port-input-select").buildSingle();
	modeWidgetLayout->addWidget(rxPortInput);

	// adi,tx-rf-port-input-select
	IIOWidget *txPortInput =
		IIOWidgetBuilder(modeWidget).device(m_device).attribute("adi,tx-rf-port-input-select").buildSingle();
	modeWidgetLayout->addWidget(txPortInput);

	// adi,rx1-rx2-phase-inversion-enable
	IIOWidget *rx1Rx2Phase = IIOWidgetBuilder(modeWidget)
					 .device(m_device)
					 .attribute("adi,rx1-rx2-phase-inversion-enable")
					 .buildSingle();
	modeWidgetLayout->addWidget(rx1Rx2Phase);

	return modeWidget;
}

QWidget *EnsmModeClocksWidget::generateClocksWidget(QWidget *parent)
{
	QWidget *clocksWidget = new QWidget(parent);
	Style::setStyle(clocksWidget, style::properties::widget::border_interactive);

	QGridLayout *clocksWidgetLayout = new QGridLayout(clocksWidget);
	clocksWidget->setLayout(clocksWidgetLayout);

	QLabel *clocksTitle = new QLabel("Clocks", clocksWidget);
	clocksWidgetLayout->addWidget(clocksTitle, 0, 0);

	// adi,xo-disable-use-ext-refclk-enable
	IIOWidget *xoDisableUseExtRefclk = IIOWidgetBuilder(clocksWidget)
						   .device(m_device)
						   .attribute("adi,xo-disable-use-ext-refclk-enable")
						   .buildSingle();
	clocksWidgetLayout->addWidget(xoDisableUseExtRefclk, 1, 0);

	// adi,external-rx-lo-enable
	IIOWidget *extrnalRxLo =
		IIOWidgetBuilder(clocksWidget).device(m_device).attribute("adi,external-rx-lo-enable").buildSingle();
	clocksWidgetLayout->addWidget(extrnalRxLo, 2, 0);

	// adi,external-tx-lo-enable
	IIOWidget *extrnalTxLo =
		IIOWidgetBuilder(clocksWidget).device(m_device).attribute("adi,external-tx-lo-enable").buildSingle();
	clocksWidgetLayout->addWidget(extrnalTxLo, 3, 0);

	// adi,rx-fastlock-pincontrol-enable
	IIOWidget *rxFastlockPincontrol = IIOWidgetBuilder(clocksWidget)
						  .device(m_device)
						  .attribute("adi,rx-fastlock-pincontrol-enable")
						  .buildSingle();
	clocksWidgetLayout->addWidget(rxFastlockPincontrol, 4, 0);

	// adi,rx-fastlock-delay-ns
	IIOWidget *rxFastLockDelay =
		IIOWidgetBuilder(clocksWidget).device(m_device).attribute("adi,rx-fastlock-delay-ns").buildSingle();
	clocksWidgetLayout->addWidget(rxFastLockDelay, 4, 1);

	// adi,tx-fastlock-pincontrol-enable
	IIOWidget *txFastlockPincontrol = IIOWidgetBuilder(clocksWidget)
						  .device(m_device)
						  .attribute("adi,tx-fastlock-pincontrol-enable")
						  .buildSingle();
	clocksWidgetLayout->addWidget(txFastlockPincontrol, 5, 0);

	// adi,tx-fastlock-delay-ns
	IIOWidget *txFastLockDelay =
		IIOWidgetBuilder(clocksWidget).device(m_device).attribute("adi,tx-fastlock-delay-ns").buildSingle();
	clocksWidgetLayout->addWidget(txFastLockDelay, 5, 1);

	return clocksWidget;
}
