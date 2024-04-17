#ifndef HOCKEYPUCKTEMPSENSORPLUGIN_H
#define HOCKEYPUCKTEMPSENSORPLUGIN_H

#define SCOPY_PLUGIN_NAME HockeyPuckTempSensorPlugin

#include "scopy-hockeypucktempsensorplugin_export.h"
#include <QObject>
#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>
#include <iioutil/connectionprovider.h>

namespace scopy::HockeyPuckTempSensor {
class SCOPY_HOCKEYPUCKTEMPSENSORPLUGIN_EXPORT HockeyPuckTempSensorPlugin : public QObject, public PluginBase
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
	QString displayName() override;

public Q_SLOTS:
	bool onConnect() override;
	bool onDisconnect() override;

private:
	iio_context *m_ctx;
};
} // namespace scopy::HockeyPuckTempSensor
#endif // HOCKEYPUCKTEMPSENSORPLUGIN_H
