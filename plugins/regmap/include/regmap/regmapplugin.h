#ifndef REGMAPPLUGIN_H
#define REGMAPPLUGIN_H

#define SCOPY_PLUGIN_NAME REGMAPPlugin
#define SCOPY_PLUGIN_PRIO 100

#include <QObject>
#include <pluginbase/plugin.h>
#include "pluginbase/pluginbase.h"
#include "scopyregmap_export.h"
#include <pluginbase/pluginbase.h>
#include <iio.h>

namespace Ui {

}

namespace adiscope {
class SCOPYREGMAP_EXPORT REGMAPPlugin : public QObject, public PluginBase
{
	Q_OBJECT
	SCOPY_PLUGIN;

public:
	bool loadPage() override;
	bool loadIcon() override;
	void loadToolList() override;
	void unload() override;
	bool compatible(QString uri) override;
    void preload() override;


	void initMetadata() override;

    QWidget* getTool();
public Q_SLOTS:
	bool onConnect() override;
	bool onDisconnect() override;

private:
    QWidget *m_registerMapWidget;
    QList<iio_device*> *m_deviceList = nullptr;
    struct iio_device* getIioDevice(iio_context* ctx, const char *dev_name);


};
}

#endif // REGMAPPLUGIN_H
