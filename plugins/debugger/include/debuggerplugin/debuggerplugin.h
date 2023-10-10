#ifndef DEBUGGERPLUGIN_H
#define DEBUGGERPLUGIN_H

#define SCOPY_PLUGIN_NAME DebuggerPlugin

#include "QLabel"
#include "scopy-debuggerplugin_export.h"

#include <QObject>

#include <pluginbase/pluginbase.h>

namespace scopy::debugger {
class SCOPY_DEBUGGERPLUGIN_EXPORT DebuggerPlugin : public QObject, public PluginBase
{
	Q_OBJECT
	SCOPY_PLUGIN;

public:
	void initMetadata() override;
	bool compatible(QString param, QString category) override;
	bool loadPage() override;
	void loadToolList() override;
	QString description() override;

public Q_SLOTS:
	bool onConnect() override;
	bool onDisconnect() override;
};
} // namespace scopy::debugger

#endif // DEBUGGERPLUGIN_H
