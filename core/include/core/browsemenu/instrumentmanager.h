#ifndef INSTRUMENTMANAGER_H
#define INSTRUMENTMANAGER_H

#include "instrumentmenu.h"

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
	InstrumentManager(ToolStack *ts, InstrumentMenu *instrumentMenu, QObject *parent = nullptr);
	~InstrumentManager();

	void addMenuItem(QString deviceId, QString deviceName, QList<ToolMenuEntry *> tools);
	void removeMenuItem(QString deviceId);
	void changeToolListContents(QString deviceId, QList<ToolMenuEntry *> tools);

	void showMenuItem(QString id);
	void hideMenuItem(QString id);

public Q_SLOTS:
	void deviceConnected(QString id);
	void deviceDisconnected(QString id);

Q_SIGNALS:
	void requestToolSelect(QString);

private Q_SLOTS:
	void updateTool(QWidget *old);

private:
	QString m_prevItem;
	QStringList m_connectedDev;
	ToolStack *m_ts;
	InstrumentMenu *m_instrumentMenu;
	QMap<QString, MenuSectionCollapseWidget *> m_itemMap;
};
} // namespace scopy

#endif // INSTRUMENTMANAGER_H
