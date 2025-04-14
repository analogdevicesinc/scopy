#include "miscwidget.h"

#include <style.h>
#include <iioutil/connectionprovider.h>
#include <iiowidgetbuilder.h>

using namespace scopy;
using namespace pluto;

MiscWidget::MiscWidget(QString uri, QWidget *parent)
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

	QWidget *dcOffsetTracking = new QWidget(this);
	QGridLayout *dcOffsetTrackingLayout = new QGridLayout(dcOffsetTracking);
	dcOffsetTracking->setLayout(dcOffsetTrackingLayout);

	dcOffsetTrackingLayout->addWidget(new QLabel("RX Freq", dcOffsetTracking), 0, 0);
	dcOffsetTrackingLayout->addWidget(new QLabel("> 4 GHz", dcOffsetTracking), 0, 1);
	dcOffsetTrackingLayout->addWidget(new QLabel("< 4 GHz", dcOffsetTracking), 0, 2);

	// attenuation

	dcOffsetTrackingLayout->addWidget(new QLabel("Attenuation", dcOffsetTracking), 1, 0);

	// adi,dc-offset-attenuation-low-range
	IIOWidget *attenuationLowRange = IIOWidgetBuilder(dcOffsetTracking)
						 .device(m_device)
						 .attribute("adi,dc-offset-attenuation-low-range")
						 .uiStrategy(IIOWidgetBuilder::RangeUi)
						 .buildSingle();
	dcOffsetTrackingLayout->addWidget(attenuationLowRange, 1, 1);

	// adi,dc-offset-attenuation-high-range
	IIOWidget *attenuationHighRange = IIOWidgetBuilder(dcOffsetTracking)
						  .device(m_device)
						  .attribute("adi,dc-offset-attenuation-high-range")
						  .uiStrategy(IIOWidgetBuilder::RangeUi)
						  .buildSingle();
	dcOffsetTrackingLayout->addWidget(attenuationHighRange, 1, 2);

	// count
	dcOffsetTrackingLayout->addWidget(new QLabel("Count", dcOffsetTracking), 2, 0);

	// adi,dc-offset-count-low-range
	IIOWidget *countLowRange = IIOWidgetBuilder(dcOffsetTracking)
					   .device(m_device)
					   .attribute("adi,dc-offset-count-low-range")
					   .uiStrategy(IIOWidgetBuilder::RangeUi)
					   .buildSingle();
	dcOffsetTrackingLayout->addWidget(countLowRange, 2, 1);

	// adi,dc-offset-count-high-range
	IIOWidget *countHighRange = IIOWidgetBuilder(dcOffsetTracking)
					    .device(m_device)
					    .attribute("adi,dc-offset-count-high-range")
					    .uiStrategy(IIOWidgetBuilder::RangeUi)
					    .buildSingle();
	dcOffsetTrackingLayout->addWidget(countHighRange, 2, 2);

	// adi,dc-offset-tracking-update-event-mask
	dcOffsetTrackingLayout->addWidget(new QLabel("Update Event Mask", dcOffsetTracking), 3, 0);
	IIOWidget *updateEventMask = IIOWidgetBuilder(dcOffsetTracking)
					     .device(m_device)
					     .attribute("adi,dc-offset-tracking-update-event-mask")
					     .uiStrategy(IIOWidgetBuilder::RangeUi)
					     .buildSingle();
	dcOffsetTrackingLayout->addWidget(updateEventMask, 3, 1);

	m_layout->addWidget(dcOffsetTracking);

	// adi,qec-tracking-slow-mode-enable
	QWidget *qecTracking = new QWidget(this);
	QVBoxLayout *qecTrackingLayout = new QVBoxLayout(qecTracking);
	qecTracking->setLayout(qecTrackingLayout);

	qecTrackingLayout->addWidget(new QLabel("QEC Tracking", qecTracking));

	IIOWidget *qecTrackingIio = IIOWidgetBuilder(qecTracking)
					    .device(m_device)
					    .attribute("adi,qec-tracking-slow-mode-enable")
					    // TODO MAKE IT SWITCH
					    .buildSingle();
	qecTrackingLayout->addWidget(qecTrackingIio);

	m_layout->addWidget(qecTracking);
	m_layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));
}

MiscWidget::~MiscWidget() { ConnectionProvider::close(m_uri); }
