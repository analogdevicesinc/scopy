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
	Style::setStyle(this, style::properties::widget::border_interactive);

	m_layout = new QVBoxLayout(this);
	setLayout(m_layout);

	// Get connection to device
	Connection *conn = ConnectionProvider::GetInstance()->open(m_uri);
	// iio:device0: ad9361-phy
	m_device = iio_context_find_device(conn->context(), "ad9361-phy");

	QLabel *title = new QLabel("RSSI", this);
	m_layout->addWidget(title);

	// duration   adi,rssi-duration
	IIOWidget *rssiDuration = IIOWidgetBuilder(this)
					  .device(m_device)
					  .attribute("rssi-duration")
					  .uiStrategy(IIOWidgetBuilder::RangeUi)
					  .title("Duration (us)")
					  .buildSingle();
	m_layout->addWidget(rssiDuration);

	// delay  adi,rssi-delay
	IIOWidget *rssiDelay = IIOWidgetBuilder(this)
				       .device(m_device)
				       .attribute("rssi-delay")
				       .uiStrategy(IIOWidgetBuilder::RangeUi)
				       .title("Delay (us)")
				       .buildSingle();
	m_layout->addWidget(rssiDelay);

	// wait  adi,rssi-wait
	IIOWidget *rssiWait = IIOWidgetBuilder(this)
				      .device(m_device)
				      .attribute("rssi-wait")
				      .uiStrategy(IIOWidgetBuilder::RangeUi)
				      .title("Wait (us)")
				      .buildSingle();
	m_layout->addWidget(rssiWait);

	// restart mode adi,rssi-restart-mode
	IIOWidget *rssiRestartMode = IIOWidgetBuilder(this)
					     .device(m_device)
					     .attribute("adi,rssi-restart-mode")
					     .title("Restart Mode")
					     // TODO FIND WHERE THE DROPDOWN SHOULD GET VALUES
					     // .uiStrategy(IIOWidgetBuilder::ComboUi)
					     .buildSingle();
	m_layout->addWidget(rssiRestartMode);

	m_layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));
}

RssiWidget::~RssiWidget() { ConnectionProvider::close(m_uri); }
