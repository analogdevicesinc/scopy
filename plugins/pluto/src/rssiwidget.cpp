#include "rssiwidget.h"

#include <style.h>
#include <iioutil/connectionprovider.h>
#include <iiowidgetbuilder.h>

using namespace scopy;
using namespace pluto;

RssiWidget::RssiWidget(QString uri, QWidget *parent)
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

	QLabel *title = new QLabel("RSSI", widget);
	Style::setStyle(title, style::properties::label::menuBig);
	layout->addWidget(title);

	// adi,rssi-duration
	IIOWidget *rssiDuration = IIOWidgetBuilder(widget)
					  .device(m_device)
					  .attribute("rssi-duration")
					  .uiStrategy(IIOWidgetBuilder::RangeUi)
					  .title("Duration (us)")
					  .buildSingle();
	layout->addWidget(rssiDuration);
	rssiDuration->setToolTip("Total RSSI measurement duration");

	// adi,rssi-delay
	IIOWidget *rssiDelay = IIOWidgetBuilder(widget)
				       .device(m_device)
				       .attribute("rssi-delay")
				       .uiStrategy(IIOWidgetBuilder::RangeUi)
				       .title("Delay (us)")
				       .buildSingle();
	layout->addWidget(rssiDelay);
	rssiDelay->setToolTip("When the RSSI algorithm (re)starts, the AD9361 first waits for the Rx signal path to "
			      "settle. This delay is the “RSSI Delay”");

	// wait  adi,rssi-wait
	IIOWidget *rssiWait = IIOWidgetBuilder(widget)
				      .device(m_device)
				      .attribute("rssi-wait")
				      .uiStrategy(IIOWidgetBuilder::RangeUi)
				      .title("Wait (us)")
				      .buildSingle();
	layout->addWidget(rssiWait);
	rssiWait->setToolTip("After the “RSSI Delay” the RSSI algorithm alternates between measuring RSSI and waiting "
			     "“RSSI Wait” to measure RSSI");

	// adi,rssi-restart-mode
	IIOWidget *rssiRestartMode = IIOWidgetBuilder(widget)
					     .device(m_device)
					     .attribute("adi,rssi-restart-mode")
					     .title("Restart Mode")
					     // TODO FIND WHERE THE DROPDOWN SHOULD GET VALUES
					     // .uiStrategy(IIOWidgetBuilder::ComboUi)
					     .buildSingle();
	layout->addWidget(rssiRestartMode);

	m_layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	connect(this, &RssiWidget::readRequested, this, [=, this]() {
		rssiDuration->readAsync();
		rssiDelay->readAsync();
		rssiWait->readAsync();
		rssiRestartMode->readAsync();
	});
}

RssiWidget::~RssiWidget() { ConnectionProvider::close(m_uri); }
