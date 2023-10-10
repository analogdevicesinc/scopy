#ifndef TESTPLUGIN_H
#define TESTPLUGIN_H

#define SCOPY_PLUGIN_NAME BareMinimum

#include "scopy-bareminimumplugin_export.h"

#include <QLineEdit>
#include <QObject>

#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>

namespace scopy {

class SCOPY_BAREMINIMUMPLUGIN_EXPORT BareMinimum : public QObject, public PluginBase
{
	Q_OBJECT
	SCOPY_PLUGIN;

public:
	void initMetadata() override;
	bool compatible(QString m_param, QString category) override;
	void loadToolList() override;

	bool onConnect() override;
	bool onDisconnect() override;
};

} // namespace scopy

#endif // TESTPLUGIN_H
