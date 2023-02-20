#ifndef TESTPLUGINIP_H
#define TESTPLUGINIP_H

#include <QObject>
#include <plugin.h>

namespace adiscope {
class TestPluginIp : public Plugin
{
	Q_OBJECT
public:
	TestPluginIp(QObject *parent = nullptr);
	~TestPluginIp();


	// Plugin interface
public:
	bool compatible(QString uri) override;
	bool load(QString uri) override;
	void unload() override;

	bool connectDev() override;
	bool disconnectDev() override;

private:
	QWidget *m_tool;

};
}
#endif // TESTPLUGINIP_H
