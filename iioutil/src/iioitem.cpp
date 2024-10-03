#include "iioitem.h"

using namespace scopy;

IIOItem::IIOItem(QString name, QString id, IIOTypeCtx type, iio_context *ctx, IIOItem *parent)
	: QStandardItem((id.isEmpty()) ? name : id + ((name.isEmpty()) ? " " : ": " + name))
	, m_name(name)
	, m_id(id)
	, m_type(type)
	, m_ctx(ctx)
	, m_parent(parent)
{}

IIOItem::IIOItem(QString name, QString id, IIOTypeDev type, iio_device *dev, IIOItem *parent)
	: QStandardItem((id.isEmpty()) ? name : id + ((name.isEmpty()) ? " " : ": " + name))
	, m_name(name)
	, m_id(id)
	, m_type(type)
	, m_dev(dev)
	, m_parent(parent)
{}

IIOItem::IIOItem(QString name, QString id, IIOTypeChnl type, iio_channel *chnl, IIOItem *parent)
	: QStandardItem((id.isEmpty()) ? name : id + ((name.isEmpty()) ? " " : ": " + name))
	, m_name(name)
	, m_id(id)
	, m_type(type)
	, m_chnl(chnl)
	, m_parent(parent)
{}

IIOItem::~IIOItem() {}

int IIOItem::type() const { return m_type; }

int IIOItem::childCount() const { return m_children.size(); }

void IIOItem::addChild(IIOItem *item)
{
	item->setParent(this);
	appendRow(item);
	m_children.append(item); // Maintain out own list for easy access
}

void IIOItem::removeChild(IIOItem *item)
{
	int row = m_children.indexOf(item);
	if(row >= 0) {
		removeRow(row);
		m_children.removeAt(row);
		delete item;
	}
}

IIOItem *IIOItem::child(int index) const
{
	if(index >= 0 && index < m_children.size()) {
		return m_children.at(index);
	}

	return nullptr;
}

IIOItem *IIOItem::child(QString name) const
{
	for(int i = 0; i < m_children.size(); ++i) {
		if(m_children.at(i)->name() == name) {
			return m_children.at(i);
		}
	}

	return nullptr;
}

IIOItem *IIOItem::parent() const { return m_parent; }

void IIOItem::setParent(IIOItem *parent) { m_parent = parent; }

QString IIOItem::name() const { return m_name; }

QString IIOItem::id() const { return m_id; }

iio_context *IIOItem::ctx() const { return m_ctx; }

void IIOItem::setCtx(iio_context *newCtx) { m_ctx = newCtx; }

iio_device *IIOItem::dev() const { return m_dev; }

void IIOItem::setDev(iio_device *newDev) { m_dev = newDev; }

iio_channel *IIOItem::chnl() const { return m_chnl; }

void IIOItem::setChnl(iio_channel *newChnl) { m_chnl = newChnl; }
