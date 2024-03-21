#include "watchlistentry.h"
#include <iio-widgets/datastrategy/datastrategyinterface.h>
#include <QHBoxLayout>

using namespace scopy::iiodebugplugin;

WatchListEntry::WatchListEntry(IIOStandardItem *item, QObject *parent)
	: QObject(parent)
	, m_item(item)
	, m_name(new QTableWidgetItem())
	, m_value(new QTableWidgetItem())
	, m_path(new QTableWidgetItem())
{
	setupUi();
	m_name->setText(item->name());
	m_path->setText(item->path());

	IIOStandardItem::Type type = item->type();
	if(type == IIOStandardItem::ContextAttribute || type == IIOStandardItem::DeviceAttribute ||
	   type == IIOStandardItem::ChannelAttribute) {
		// it is a leaf node so it has only 1 widget
		QList<IIOWidget *> widgets = item->getIIOWidgets();
		IIOWidget *widget = widgets[0];
		m_value->setText(widget->getDataStrategy()->data());
		QObject::connect(dynamic_cast<QWidget *>(widget->getDataStrategy()), SIGNAL(sendData(QString, QString)),
				 this, SLOT(setNewData(QString, QString)));
	} else {
		m_value->setText("N/A");
	}
}

WatchListEntry::~WatchListEntry()
{
	// delete m_name;
	// delete m_value;
	// delete m_path;
}

QTableWidgetItem *WatchListEntry::name() { return m_name; }

void WatchListEntry::setName(QString name) { m_name->setText(name); }

QTableWidgetItem *WatchListEntry::value() { return m_value; }

void WatchListEntry::setValue(QString value) { m_value->setText(value); }

QTableWidgetItem *WatchListEntry::path() { return m_path; }

void WatchListEntry::setPath(QString path) { m_path->setText(path); }

IIOStandardItem *WatchListEntry::item() { return m_item; }

void WatchListEntry::setNewData(QString data, QString optionalData)
{
	Q_UNUSED(optionalData)
	m_value->setText(data);
}

void WatchListEntry::setupUi()
{
	// implement if needed
	m_name->setFlags(m_name->flags() ^ Qt::ItemIsEditable);
	m_value->setFlags(m_value->flags() ^ Qt::ItemIsEditable);
	m_path->setFlags(m_path->flags() ^ Qt::ItemIsEditable);
}
