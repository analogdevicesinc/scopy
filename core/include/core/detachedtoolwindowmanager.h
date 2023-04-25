#ifndef DETACHEDTOOLMANAGER_H
#define DETACHEDTOOLMANAGER_H

#include <QWidget>
#include <QMap>
#include <pluginbase/toolmenuentry.h>
#include "scopy-core_export.h"
#include "detachedtoolwindow.h"

namespace scopy {

class SCOPY_CORE_EXPORT DetachedToolWindowManager : public QObject {
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
	QMap<QString, DetachedToolWindow*> map;
};
}

#endif // DETACHEDTOOLMANAGER_H
