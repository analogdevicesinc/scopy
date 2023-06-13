#ifndef REGMAPPLUGIN_H
#define REGMAPPLUGIN_H

#define SCOPY_PLUGIN_NAME REGMAPPlugin
#define SCOPY_PLUGIN_PRIO 100

#include <QObject>
#include <pluginbase/plugin.h>
#include "pluginbase/pluginbase.h"
#include "scopy-regmap_export.h"
#include <pluginbase/pluginbase.h>
#include <iio.h>

namespace Ui {

}

namespace scopy {
class SCOPY_REGMAP_EXPORT REGMAPPlugin : public QObject, public PluginBase
{
	Q_OBJECT
	SCOPY_PLUGIN;

public:
	bool loadPage() override;
	bool loadIcon() override;
	void loadToolList() override;
	void unload() override;
	bool compatible(QString uri, QString category) override;
    void preload() override;
    void initPreferences() override;
    bool loadPreferencesPage() override;


	void initMetadata() override;
	QString description() override;

    QWidget* getTool();
public Q_SLOTS:
	bool onConnect() override;
	bool onDisconnect() override;

private:
    QWidget *m_registerMapWidget;
    QList<iio_device*> *m_deviceList = nullptr;
    struct iio_device* getIioDevice(iio_context* ctx, const char *dev_name);
    bool isBufferCapable(iio_device *dev);

private Q_SLOTS:
    void handlePreferenceChange(QString, QVariant);



};
}

#endif // REGMAPPLUGIN_H
