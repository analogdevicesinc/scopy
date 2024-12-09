#ifndef SCRIPTING_H
#define SCRIPTING_H

#define SCOPY_PLUGIN_NAME Scripting

#include "scopy-scripting_export.h"
#include <QObject>
#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>

namespace scopy {
namespace scripting {

class SCOPY_SCRIPTING_EXPORT Scripting : public QObject, public PluginBase
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

	QWidget *getTool();

public Q_SLOTS:
	bool onConnect() override;
	bool onDisconnect() override;

private:
	QWidget *m_scriptingWidget = nullptr;
};
} // namespace scripting
} // namespace scopy
#endif // SCRIPTING_H
