#ifndef M2KPLUGIN_H
#define M2KPLUGIN_H

#define SCOPY_PLUGIN_NAME M2kPlugin

#include <QObject>
//#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>
#include "scopym2kplugin_export.h"
#include <QLineEdit>

namespace adiscope {

class SCOPYM2KPLUGIN_EXPORT M2kPlugin : public QObject, public PluginBase
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


#endif // M2KPLUGIN_H
