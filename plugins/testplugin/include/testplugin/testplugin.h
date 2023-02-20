#ifndef TESTPLUGIN_H
#define TESTPLUGIN_H

#include <QObject>
#include <pluginbase/plugin.h>
#include "scopytestplugin_export.h"

namespace adiscope {
class SCOPYTESTPLUGIN_EXPORT TestPlugin : public Plugin
{
	Q_OBJECT
public:
	TestPlugin(QObject *parent = nullptr);
	~TestPlugin();


	// Plugin interface
public:
	bool compatible(QString uri) override;	
	bool load(QString uri) override;
	void unload() override;

	bool connectDev() override;
	bool disconnectDev() override;

};
}
#endif // TESTPLUGIN_H
