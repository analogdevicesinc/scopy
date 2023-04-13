#ifndef TESTPLUGIN_H
#define TESTPLUGIN_H

#define SCOPY_PLUGIN_NAME BareMinimum

#include <QObject>
#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>
#include "scopybareminimumplugin_export.h"
#include <QLineEdit>

namespace scopy {

class SCOPYBAREMINIMUMPLUGIN_EXPORT BareMinimum : public QObject, public PluginBase
{
	Q_OBJECT
	SCOPY_PLUGIN;
public:

	void initMetadata() override;
	bool compatible(QString m_param) override;
	void loadToolList() override;

	bool onConnect() override;
	bool onDisconnect() override;

};

}


#endif // TESTPLUGIN_H
