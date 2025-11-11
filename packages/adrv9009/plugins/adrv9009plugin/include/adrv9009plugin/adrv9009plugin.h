#ifndef ADRV9009PLUGIN_H
#define ADRV9009PLUGIN_H

#define SCOPY_PLUGIN_NAME Adrv9009Plugin

#include "scopy-adrv9009plugin_export.h"
#include <QObject>
#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>

namespace scopy::adrv9009 {
class SCOPY_ADRV9009PLUGIN_EXPORT Adrv9009Plugin : public QObject, public PluginBase
{
	Q_OBJECT
	SCOPY_PLUGIN;

public:
	bool compatible(QString m_param, QString category) override;
	bool loadPage() override;
	bool loadIcon() override;
	void loadToolList() override;
	void unload() override;
	void initMetadata() override;
	QString description() override;

public Q_SLOTS:
	bool onConnect() override;
	bool onDisconnect() override;
};
} // namespace scopy::adrv9009
#endif // ADRV9009PLUGIN_H
