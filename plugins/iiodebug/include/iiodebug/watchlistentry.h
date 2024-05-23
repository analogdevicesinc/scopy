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

	QTableWidgetItem *path();
	void setPath(QString path);

	IIOStandardItem *item();

	QTableWidgetItem *type() const;
	void setType(QString type);

	// the value entry can only be lineedit or combo box, other options do not have enough space to look good
	QWidget *valueUi() const;
	void setValueUi(QWidget *newValueUi);

private Q_SLOTS:
	void setNewData(QString data, QString optionalData);

private:
	void setupUi();
	void setupWidget(IIOWidget *widget);

	IIOStandardItem *m_item;
	QTableWidgetItem *m_name;
	QWidget *m_valueUi;
	QTableWidgetItem *m_type;
	QTableWidgetItem *m_path;

	QComboBox *m_combo;
	QLineEdit *m_lineedit;
};
} // namespace scopy::iiodebugplugin

#endif // WATCHLISTENTRY_H
