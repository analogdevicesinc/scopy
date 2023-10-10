#ifndef MAPSTACKEDWIDGET_H
#define MAPSTACKEDWIDGET_H

#include "scopy-gui_export.h"

#include <QMap>
#include <QStackedWidget>

namespace scopy {
class SCOPY_GUI_EXPORT MapStackedWidget : public QStackedWidget
{
	Q_OBJECT
public:
	explicit MapStackedWidget(QWidget *parent = nullptr);
	~MapStackedWidget();

	virtual void add(QString key, QWidget *w);
	virtual bool remove(QString key);
	virtual QString getKey(QWidget *w);
	virtual bool contains(QString key);
	virtual QWidget *get(QString key);
public Q_SLOTS:
	virtual bool show(QString key);

protected:
	QMap<QString, QWidget *> map;
};
} // namespace scopy
#endif // MAPSTACKEDWIDGET_H
