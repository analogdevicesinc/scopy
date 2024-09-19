#ifndef DEVICE_H
#define DEVICE_H

#include "pluginbase/toolmenuentry.h"
#include "scopy-core_export.h"

#include <QObject>
#include <QSettings>

namespace scopy {
class SCOPY_CORE_EXPORT Device
{
public:
	virtual ~Device(){};
	virtual QString id() = 0;
	virtual QString category() = 0;
	virtual QString displayName() = 0;
	virtual QString param() = 0;
	virtual QString displayParam() = 0;
	virtual QWidget *icon() = 0;

	virtual QWidget *page() = 0;

	virtual QList<ToolMenuEntry *> toolList() = 0;
	virtual void init() = 0;
	virtual void preload() = 0;
	virtual void loadPlugins() = 0;
	virtual void unloadPlugins() = 0;

public Q_SLOTS:
	virtual void connectDev() = 0;
	virtual void disconnectDev() = 0;
	virtual void showPage() = 0;
	virtual void hidePage() = 0;
	virtual void save(QSettings &) = 0;
	virtual void load(QSettings &) = 0;

Q_SIGNALS:
	virtual void toolListChanged() = 0;
	virtual void connected() = 0;
	virtual void disconnected() = 0;
	virtual void requestedRestart() = 0;
	virtual void requestTool(QString) = 0;
};
} // namespace scopy

#endif // DEVICE_H