#ifndef TESTPLUGIN_H
#define TESTPLUGIN_H

#include <QObject>
#include <plugin.h>

namespace adiscope {
class TestPlugin : public QObject, public Plugin
{
	Q_OBJECT
public:
	TestPlugin(QObject *parent = nullptr);
	~TestPlugin();


	// Plugin interface
public:
	bool compatible(QString uri) override;	
	bool load(QString uri) override;

	bool connectDev() override;
	bool disconnectDev() override;



};
}
#endif // TESTPLUGIN_H
