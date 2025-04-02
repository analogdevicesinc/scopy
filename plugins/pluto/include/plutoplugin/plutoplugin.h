#ifndef PLUTOPLUGIN_H
#define PLUTOPLUGIN_H

#define SCOPY_PLUGIN_NAME PlutoPlugin

#include "scopy-plutoplugin_export.h"
#include <QObject>
#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>

namespace scopy::pluto {
class SCOPY_PLUTOPLUGIN_EXPORT PlutoPlugin : public QObject, public PluginBase
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
} // namespace scopy::pluto
#endif // PLUTOPLUGIN_H
