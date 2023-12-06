% if 'class_name' in config:
#ifndef ${config['class_name'].upper()}_H
#define ${config['class_name'].upper()}_H

#define SCOPY_PLUGIN_NAME ${config['class_name']}

% if "plugin_name" in config:
#include "scopy-${config["plugin_name"]}_export.h"
% endif
#include <QObject>
#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>

% if 'namespace' in config:
namespace ${config['namespace']} {
class ${plugin_export_macro} ${config['class_name']} : public QObject, public PluginBase
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
} // namespace ${config['namespace']}
% endif
#endif // ${config['class_name'].upper()}_H
% endif