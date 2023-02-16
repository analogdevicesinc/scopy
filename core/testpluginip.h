#ifndef TESTPLUGINIP_H
#define TESTPLUGINIP_H

#include <QObject>
#include <plugin.h>

namespace adiscope {
class TestPluginIp : public QObject, public Plugin
{
	Q_OBJECT
public:
	TestPluginIp(QObject *parent = nullptr);
	~TestPluginIp();


	// Plugin interface
public:
	bool compatible(QString uri) override;
	bool load(QString uri) override;

	bool connectDev() override;
	bool disconnectDev() override;



};
}
#endif // TESTPLUGINIP_H
