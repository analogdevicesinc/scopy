#ifndef WATCHLISTENTRY_H
#define WATCHLISTENTRY_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QTableWidgetItem>

#include "iiostandarditem.h"

namespace scopy::iiodebugplugin {
class WatchListEntry : public QObject
{
	Q_OBJECT
public:
	explicit WatchListEntry(IIOStandardItem *item, QObject *parent = nullptr);
	~WatchListEntry();

	QTableWidgetItem *name();
	void setName(QString name);

	QTableWidgetItem *value();
	void setValue(QString value);

	QTableWidgetItem *path();
	void setPath(QString path);

	IIOStandardItem *item();

private Q_SLOTS:
	void setNewData(QString data, QString optionalData);

private:
	void setupUi();

	IIOStandardItem *m_item;
	QTableWidgetItem *m_name;
	QTableWidgetItem *m_value;
	QTableWidgetItem *m_path;
};
} // namespace scopy::iiodebugplugin

#endif // WATCHLISTENTRY_H
