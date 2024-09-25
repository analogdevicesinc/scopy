#ifndef TOOLMENUMANAGER_H
#define TOOLMENUMANAGER_H

#include "detachedtoolwindowmanager.h"
#include "toolmenu.h"
#include "toolmenuitem.h"

#include <QMap>
#include <QObject>
#include <menusectionwidget.h>
#include "toolstack.h"
#include <pluginbase/toolmenuentry.h>
#include "scopy-core_export.h"

namespace scopy {
class SCOPY_CORE_EXPORT ToolMenuManager : public QObject
{
	Q_OBJECT
public:
	typedef struct
	{
		QString name;
		QString uri;
	} DeviceInfo;

	ToolMenuManager(ToolStack *ts, DetachedToolWindowManager *dtm, ToolMenu *toolMenu, QObject *parent = nullptr);
	~ToolMenuManager();

	void addMenuItem(QString deviceId, DeviceInfo devInfo, QList<ToolMenuEntry *> tools, int itemIndex = -1);
	void removeMenuItem(QString deviceId);
	void changeToolListContents(QString deviceId, QList<ToolMenuEntry *> tools);

	void showMenuItem(QString id);
	void hideMenuItem(QString id);

public Q_SLOTS:
	void deviceConnected(QString id);
	void deviceDisconnected(QString id);
	void onDisplayNameChanged(QString id, QString devName);

Q_SIGNALS:
	void requestToolSelect(QString id);

private Q_SLOTS:
	void updateTool(QWidget *old);
	void updateToolAttached(bool oldAttach, ToolMenuItem *toolMenuItem);

private:
	void loadToolAttachedState(ToolMenuEntry *tme);
	void saveToolAttachedState(ToolMenuEntry *tme);
	void detachSuccesful(ToolMenuItem *toolMenuItem);
	void attachSuccesful(ToolMenuItem *toolMenuItem);
	void showTool(ToolMenuItem *toolMenuItem);
	void selectTool(ToolMenuItem *toolMenuItem, bool on);
	void setTmeAttached(ToolMenuEntry *tme);
	void createMenuSectionLabel(MenuSectionCollapseWidget *section, QString uri);
	ToolMenuItem *createToolMenuItem(ToolMenuEntry *tme, QWidget *parent = nullptr);

	QString m_prevItem;
	QStringList m_connectedDev;
	ToolStack *m_ts;
	DetachedToolWindowManager *m_dtm;
	ToolMenu *m_toolMenu;
	QMap<QString, MenuSectionCollapseWidget *> m_itemMap;
	QMap<QString, DeviceInfo> m_devInfoMap;
};
} // namespace scopy

#endif // TOOLMENUMANAGER_H
