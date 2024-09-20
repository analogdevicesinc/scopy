#ifndef INSTRUMENTMANAGER_H
#define INSTRUMENTMANAGER_H

#include "../detachedtoolwindowmanager.h"
#include "instrumentmenu.h"
#include "instrumentwidget.h"

#include <QMap>
#include <QObject>
#include <menusectionwidget.h>
#include "../toolstack.h"
#include <pluginbase/toolmenuentry.h>
#include "../scopy-core_export.h"

namespace scopy {
class SCOPY_CORE_EXPORT InstrumentManager : public QObject
{
	Q_OBJECT
public:
	typedef struct
	{
		QString name;
		QString uri;
	} DeviceInfo;

	InstrumentManager(ToolStack *ts, DetachedToolWindowManager *dtm, InstrumentMenu *instrumentMenu,
			  QObject *parent = nullptr);
	~InstrumentManager();

	void addMenuItem(QString deviceId, DeviceInfo devInfo, QList<ToolMenuEntry *> tools, int itemIndex = -1);
	void removeMenuItem(QString deviceId);
	void changeToolListContents(QString deviceId, QList<ToolMenuEntry *> tools);

	void showMenuItem(QString id);
	void hideMenuItem(QString id);

public Q_SLOTS:
	void deviceConnected(QString id);
	void deviceDisconnected(QString id);

Q_SIGNALS:
	void requestToolSelect(QString id);

private Q_SLOTS:
	void updateTool(QWidget *old);
	void updateToolAttached(bool oldAttach, InstrumentWidget *instrWidget);

private:
	void loadToolAttachedState(ToolMenuEntry *tme);
	void saveToolAttachedState(ToolMenuEntry *tme);
	void detachSuccesful(InstrumentWidget *instrWidget);
	void attachSuccesful(InstrumentWidget *instrWidget);
	void showTool(InstrumentWidget *instrWidget);
	void selectInstrument(InstrumentWidget *instrWidget, bool on);
	void setTmeAttached(ToolMenuEntry *tme);
	void createMenuSectionLabel(MenuSectionCollapseWidget *section, QString uri);
	InstrumentWidget *createInstrWidget(ToolMenuEntry *tme, QWidget *parent = nullptr);

	QString m_prevItem;
	QStringList m_connectedDev;
	ToolStack *m_ts;
	DetachedToolWindowManager *m_dtm;
	InstrumentMenu *m_instrumentMenu;
	QMap<QString, MenuSectionCollapseWidget *> m_itemMap;
	QMap<QString, DeviceInfo> m_devInfoMap;
};
} // namespace scopy

#endif // INSTRUMENTMANAGER_H
