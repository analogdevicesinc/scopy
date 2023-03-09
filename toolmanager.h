#ifndef TOOLMANAGER_H
#define TOOLMANAGER_H

#include <QObject>
#include <QMap>
#include "toolmenu.h"
#include "toolmenuentry.h"

namespace adiscope {
class ToolManager : public QObject
{
	Q_OBJECT


public:
	ToolManager(ToolMenu* tm, QObject *parent = nullptr);
	~ToolManager();

public Q_SLOTS:
	void addToolList(QString, QList<ToolMenuEntry>);
	void removeToolList(QString);
	void changeToolListContents(QString, QList<ToolMenuEntry>);
	void showToolList(QString);
	void hideToolList(QString);
	void lockToolList(QString);
	void unlockToolList(QString);

private:
	typedef struct {
		QString uri;
		QList<ToolMenuEntry> tools;
		bool lock;

	} st;
	QMap<QString, st> map;
	QString currentKey;
	ToolMenu *tm;
};
}

#endif // TOOLMANAGER_H
