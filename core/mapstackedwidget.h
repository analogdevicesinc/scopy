#ifndef MAPSTACKEDWIDGET_H
#define MAPSTACKEDWIDGET_H

#include <QStackedWidget>
#include <QMap>
#include <scopycore_export.h>

namespace adiscope {
class SCOPYCORE_EXPORT MapStackedWidget : public QStackedWidget
{
	Q_OBJECT
public:
	explicit MapStackedWidget(QWidget *parent = nullptr);
	~MapStackedWidget();

	virtual void add(QString key, QWidget *w);
	virtual bool remove(QString key);
	virtual QString getKey(QWidget *w);
	virtual bool contains(QString key);
public Q_SLOTS:
	virtual bool show(QString key);
protected:
	QMap<QString, QWidget*> map;
};
}
#endif // MAPSTACKEDWIDGET_H


