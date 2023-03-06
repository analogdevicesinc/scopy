#ifndef TOOLMANAGER_H
#define TOOLMANAGER_H

#include <QObject>
#include <QMap>
#include "toolmenu.h"
#include "pluginbase/toolmenuentry.h"
#include "toolstack.h"
#include "scopycore_export.h"

namespace adiscope {
class SCOPYCORE_EXPORT ToolManager : public QObject
{
	Q_OBJECT


public:
	ToolManager(ToolMenu* tm, ToolStack *ts, QObject *parent = nullptr);
	~ToolManager();

public Q_SLOTS:
	void addToolList(QString, QList<ToolMenuEntry*>);
	void removeToolList(QString);	
	void changeToolListContents(QString, QList<ToolMenuEntry*>);
	void showToolList(QString);
	void hideToolList(QString);
	void lockToolList(QString);
	void unlockToolList(QString);

	void updateToolEntry(ToolMenuEntry *tme, QString s);
	void updateToolEntry();
	void updateTool();
	void showTool(QString id);

private:
	typedef struct {
		QString id;
		QList<ToolMenuEntry*> tools;
		bool lock;

	} st;
	QMap<QString, st> map;
	QString currentKey;
	QStringList lockedToolLists;
	ToolMenu *tm;
	ToolStack *ts;
};
}

#endif // TOOLMANAGER_H
