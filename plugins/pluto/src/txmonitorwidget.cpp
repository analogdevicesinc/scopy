#include "txmonitorwidget.h"

#include <style.h>
#include <iioutil/connectionprovider.h>
#include <iiowidgetbuilder.h>

using namespace scopy;
using namespace pluto;

TxMonitorWidget::TxMonitorWidget(QString uri, QWidget *parent)
	: m_uri(uri)
	, QWidget{parent}
{
	Style::setBackgroundColor(this, json::theme::background_primary);
	Style::setStyle(this, style::properties::widget::border_interactive);

	m_layout = new QVBoxLayout(this);
	setLayout(m_layout);

	// Get connection to device
	Connection *conn = ConnectionProvider::GetInstance()->open(m_uri);
	// iio:device0: ad9361-phy
	m_device = iio_context_find_device(conn->context(), "ad9361-phy");

	QGridLayout *gLayout1 = new QGridLayout();

	gLayout1->addWidget(new QLabel("TX1", this), 0, 1);
	gLayout1->addWidget(new QLabel("TX2", this), 0, 2);

	gLayout1->addWidget(new QLabel("Frontend Gain", this), 1, 0);
	gLayout1->addWidget(new QLabel("LO Common Mode", this), 2, 0);

	// adi,txmon-1-front-end-gain
	IIOWidget *tx1FrontendGain =
		IIOWidgetBuilder(this).device(m_device).attribute("adi,txmon-1-front-end-gain").buildSingle();
	gLayout1->addWidget(tx1FrontendGain, 1, 1);

	// adi,txmon-2-front-end-gain
	IIOWidget *tx2FrontendGain =
		IIOWidgetBuilder(this).device(m_device).attribute("adi,txmon-2-front-end-gain").buildSingle();
	gLayout1->addWidget(tx2FrontendGain, 1, 2);

	// adi,txmon-1-lo-cm

	IIOWidget *tx1LoCommonMode = IIOWidgetBuilder(this)
					     .device(m_device)
					     .attribute("adi,txmon-1-lo-cm")
					     .uiStrategy(IIOWidgetBuilder::RangeUi)
					     .buildSingle();
	gLayout1->addWidget(tx1LoCommonMode, 2, 1);

	// adi,txmon-2-lo-cm

	IIOWidget *tx2LoCommonMode = IIOWidgetBuilder(this)
					     .device(m_device)
					     .attribute("adi,txmon-2-lo-cm")
					     .uiStrategy(IIOWidgetBuilder::RangeUi)
					     .buildSingle();
	gLayout1->addWidget(tx2LoCommonMode, 2, 2);

	m_layout->addLayout(gLayout1);

	// adi,txmon-low-high-thresh

	IIOWidget *lowHighTresh = IIOWidgetBuilder(this)
					  .device(m_device)
					  .attribute("adi,txmon-low-high-thresh")
					  .uiStrategy(IIOWidgetBuilder::RangeUi)
					  .buildSingle();
	m_layout->addWidget(lowHighTresh);

	// adi,txmon-low-gain

	IIOWidget *lowGain = IIOWidgetBuilder(this)
				     .device(m_device)
				     .attribute("adi,txmon-low-gain")
				     .uiStrategy(IIOWidgetBuilder::RangeUi)
				     .buildSingle();
	m_layout->addWidget(lowGain);

	// adi,txmon-high-gain

	IIOWidget *highGain = IIOWidgetBuilder(this)
				      .device(m_device)
				      .attribute("adi,txmon-high-gain")
				      .uiStrategy(IIOWidgetBuilder::RangeUi)
				      .buildSingle();
	m_layout->addWidget(highGain);

	// adi,txmon-delay

	IIOWidget *delay = IIOWidgetBuilder(this)
				   .device(m_device)
				   .attribute("adi,txmon-delay")
				   .uiStrategy(IIOWidgetBuilder::RangeUi)
				   .buildSingle();
	m_layout->addWidget(delay);

	// adi,txmon-duration

	IIOWidget *duration = IIOWidgetBuilder(this)
				      .device(m_device)
				      .attribute("adi,txmon-duration")
				      .uiStrategy(IIOWidgetBuilder::RangeUi)
				      .buildSingle();
	m_layout->addWidget(duration);

	// adi,txmon-dc-tracking-enable

	IIOWidget *dcTrackingEnabled =
		IIOWidgetBuilder(this).device(m_device).attribute("adi,txmon-dc-tracking-enable").buildSingle();
	m_layout->addWidget(dcTrackingEnabled);

	// adi,txmon-one-shot-mode-enable
	IIOWidget *oneShotModeEnabled =
		IIOWidgetBuilder(this).device(m_device).attribute("adi,txmon-one-shot-mode-enable").buildSingle();
	m_layout->addWidget(oneShotModeEnabled);

	m_layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));
}

TxMonitorWidget::~TxMonitorWidget() { ConnectionProvider::close(m_uri); }
