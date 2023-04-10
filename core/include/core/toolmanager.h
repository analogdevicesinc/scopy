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

	void updateToolEntry(ToolMenuEntry *tme);
	void updateToolEntry();
	void updateToolAttached();

	bool eventFilter(QObject *object, QEvent *event) override;


	void updateTool();
	void showTool(QString id);
	void toggleAttach(QString id);

private:

	void saveToolAttachedState(ToolMenuEntry *tme);
	void loadToolAttachedState(ToolMenuEntry *tme);

	void saveToolGeometry(ToolMenuEntry *tme, QWidget *w);
	void loadToolGeometry(ToolMenuEntry *tme, QWidget *w);

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
