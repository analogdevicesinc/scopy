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

	m_layout = new QVBoxLayout(this);
	m_layout->setMargin(0);
	m_layout->setContentsMargins(0, 0, 0, 0);
	setLayout(m_layout);

	QWidget *widget = new QWidget(this);
	QVBoxLayout *layout = new QVBoxLayout(widget);
	widget->setLayout(layout);

	m_layout->addWidget(widget);

	Style::setStyle(widget, style::properties::widget::border_interactive);

	// Get connection to device
	Connection *conn = ConnectionProvider::GetInstance()->open(m_uri);
	// iio:device0: ad9361-phy
	m_device = iio_context_find_device(conn->context(), "ad9361-phy");

	QGridLayout *gLayout1 = new QGridLayout();

	gLayout1->addWidget(new QLabel("TX1", widget), 0, 1);
	gLayout1->addWidget(new QLabel("TX2", widget), 0, 2);

	gLayout1->addWidget(new QLabel("Frontend Gain", widget), 1, 0);
	gLayout1->addWidget(new QLabel("LO Common Mode", widget), 2, 0);

	// adi,txmon-1-front-end-gain
	IIOWidget *tx1FrontendGain = IIOWidgetBuilder(widget)
					     .device(m_device)
					     .attribute("adi,txmon-1-front-end-gain")
					     .title(" ")
					     .buildSingle();
	gLayout1->addWidget(tx1FrontendGain, 1, 1);

	// adi,txmon-2-front-end-gain
	IIOWidget *tx2FrontendGain = IIOWidgetBuilder(widget)
					     .device(m_device)
					     .attribute("adi,txmon-2-front-end-gain")
					     .title(" ")
					     .buildSingle();
	gLayout1->addWidget(tx2FrontendGain, 1, 2);

	// adi,txmon-1-lo-cm

	IIOWidget *tx1LoCommonMode = IIOWidgetBuilder(widget)
					     .device(m_device)
					     .attribute("adi,txmon-1-lo-cm")
					     .uiStrategy(IIOWidgetBuilder::RangeUi)
					     .title(" ")
					     .buildSingle();
	gLayout1->addWidget(tx1LoCommonMode, 2, 1);

	// adi,txmon-2-lo-cm

	IIOWidget *tx2LoCommonMode = IIOWidgetBuilder(widget)
					     .device(m_device)
					     .attribute("adi,txmon-2-lo-cm")
					     .uiStrategy(IIOWidgetBuilder::RangeUi)
					     .title(" ")
					     .buildSingle();
	gLayout1->addWidget(tx2LoCommonMode, 2, 2);

	layout->addLayout(gLayout1);

	// adi,txmon-low-high-thresh

	IIOWidget *lowHighThresh = IIOWidgetBuilder(widget)
					   .device(m_device)
					   .attribute("adi,txmon-low-high-thresh")
					   .uiStrategy(IIOWidgetBuilder::RangeUi)
					   .title("Low/High Gain Threshold (mdB)")
					   .buildSingle();
	layout->addWidget(lowHighThresh);

	// adi,txmon-low-gain

	IIOWidget *lowGain = IIOWidgetBuilder(widget)
				     .device(m_device)
				     .attribute("adi,txmon-low-gain")
				     .uiStrategy(IIOWidgetBuilder::RangeUi)
				     .title("Low Gain (dB)")
				     .buildSingle();
	layout->addWidget(lowGain);

	// adi,txmon-high-gain

	IIOWidget *highGain = IIOWidgetBuilder(widget)
				      .device(m_device)
				      .attribute("adi,txmon-high-gain")
				      .uiStrategy(IIOWidgetBuilder::RangeUi)
				      .title("High Gain (dB)")
				      .buildSingle();
	layout->addWidget(highGain);

	// adi,txmon-delay

	IIOWidget *delay = IIOWidgetBuilder(widget)
				   .device(m_device)
				   .attribute("adi,txmon-delay")
				   .uiStrategy(IIOWidgetBuilder::RangeUi)
				   .title("Delay (RX samples)")
				   .buildSingle();
	layout->addWidget(delay);

	// adi,txmon-duration

	IIOWidget *duration = IIOWidgetBuilder(widget)
				      .device(m_device)
				      .attribute("adi,txmon-duration")
				      .uiStrategy(IIOWidgetBuilder::RangeUi)
				      .title("Duration (RX Samples)")
				      .buildSingle();
	layout->addWidget(duration);

	// adi,txmon-dc-tracking-enable
	IIOWidget *dcTrackingEnabled = IIOWidgetBuilder(widget)
					       .device(m_device)
					       .attribute("adi,txmon-dc-tracking-enable")
					       .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
					       .title("Enable DC Tracking")
					       .buildSingle();
	layout->addWidget(dcTrackingEnabled);

	// adi,txmon-one-shot-mode-enable
	IIOWidget *oneShotModeEnabled = IIOWidgetBuilder(widget)
						.device(m_device)
						.attribute("adi,txmon-one-shot-mode-enable")
						.uiStrategy(IIOWidgetBuilder::CheckBoxUi)
						.title("Enable One Shot Mode")
						.buildSingle();
	layout->addWidget(oneShotModeEnabled);

	m_layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	connect(this, &TxMonitorWidget::readRequested, this, [=, this]() {
		tx1FrontendGain->readAsync();
		tx2FrontendGain->readAsync();
		tx1LoCommonMode->readAsync();
		tx2LoCommonMode->readAsync();
		lowHighThresh->readAsync();
		lowGain->readAsync();
		highGain->readAsync();
		delay->readAsync();
		duration->readAsync();
		dcTrackingEnabled->readAsync();
		oneShotModeEnabled->readAsync();
	});
}

TxMonitorWidget::~TxMonitorWidget() { ConnectionProvider::close(m_uri); }
