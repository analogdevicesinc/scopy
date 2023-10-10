#ifndef DETACHEDTOOLMANAGER_H
#define DETACHEDTOOLMANAGER_H

#include "detachedtoolwindow.h"
#include "scopy-core_export.h"

#include <QMap>
#include <QWidget>

#include <pluginbase/toolmenuentry.h>

namespace scopy {

class SCOPY_CORE_EXPORT DetachedToolWindowManager : public QObject
{
	Q_OBJECT
public:
	DetachedToolWindowManager(QObject *parent = nullptr);
	~DetachedToolWindowManager();

	void add(QString id, ToolMenuEntry *tme);
	bool remove(QString id);
	bool contains(QString key);
	QWidget *getWidget(QString key);

public Q_SLOTS:
	void show(QString id);

private:
	QMap<QString, DetachedToolWindow *> map;
};
} // namespace scopy

#endif // DETACHEDTOOLMANAGER_H
