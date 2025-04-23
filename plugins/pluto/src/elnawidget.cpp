#include "elnawidget.h"

#include <style.h>
#include <iioutil/connectionprovider.h>
#include <iiowidgetbuilder.h>

using namespace scopy;
using namespace pluto;

ElnaWidget::ElnaWidget(QString uri, QWidget *parent)
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

	QLabel *title = new QLabel("eLNA", this);
	m_layout->addWidget(title);

	// adi,elna-gain-mdB

	IIOWidget *gain = IIOWidgetBuilder(this)
				  .device(m_device)
				  .attribute("elna-gain-mdB")
				  .uiStrategy(IIOWidgetBuilder::RangeUi)
				  .title("LNA Gain (mdB")
				  .buildSingle();
	m_layout->addWidget(gain);

	// adi,elna-bypass-loss-mdB
	IIOWidget *bypassLoss = IIOWidgetBuilder(this)
					.device(m_device)
					.attribute("bypass-loss-mdB")
					.uiStrategy(IIOWidgetBuilder::RangeUi)
					.title("LNA Bypass Loss (mdB)")
					.buildSingle();
	m_layout->addWidget(bypassLoss);

	// adi,elna-settling-delay-ns
	IIOWidget *settlingDelay = IIOWidgetBuilder(this)
					   .device(m_device)
					   .attribute("settling-delay-ns")
					   .uiStrategy(IIOWidgetBuilder::RangeUi)
					   .title("Setting Delay (ns)")
					   .buildSingle();
	m_layout->addWidget(settlingDelay);

	QHBoxLayout *hlayout = new QHBoxLayout();

	// adi,elna-rx1-gpo0-control-enable
	IIOWidget *rx1GPO0Controll = IIOWidgetBuilder(this)
					     .device(m_device)
					     .attribute("adi,elna-rx1-gpo0-control-enable")
					     .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
					     .title("RX1 GPO0")
					     .buildSingle();
	hlayout->addWidget(rx1GPO0Controll);

	// adi,elna-rx2-gpo1-control-enable
	IIOWidget *rx2GPO0Controll = IIOWidgetBuilder(this)
					     .device(m_device)
					     .attribute("adi,elna-rx2-gpo1-control-enable")
					     .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
					     .title("RX2 GPO0")
					     .buildSingle();
	hlayout->addWidget(rx2GPO0Controll);

	m_layout->addLayout(hlayout);

	// adi,elna-gaintable-all-index-enable
	IIOWidget *gaintableAllIndex = IIOWidgetBuilder(this)
					       .device(m_device)
					       .attribute("adi,elna-gaintable-all-index-enable")
					       .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
					       .title("External LNA enabled for all gain indexes")
					       .buildSingle();
	m_layout->addWidget(gaintableAllIndex);

	m_layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));
}

ElnaWidget::~ElnaWidget() { ConnectionProvider::close(m_uri); }
