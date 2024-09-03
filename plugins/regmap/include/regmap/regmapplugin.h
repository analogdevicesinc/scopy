#ifndef REGMAPPLUGIN_H
#define REGMAPPLUGIN_H

#define SCOPY_PLUGIN_NAME RegmapPlugin
#define SCOPY_PLUGIN_PRIO 100

#include "pluginbase/pluginbase.h"
#include "scopy-regmap_export.h"

#include <iio.h>

#include <QObject>

#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>

#include <readwrite/iregisterreadstrategy.hpp>
#include <readwrite/iregisterwritestrategy.hpp>

namespace Ui {}

namespace scopy {
namespace regmap {

class RegisterMapTool;
class JsonFormatedElement;
class RegMap_API;

class SCOPY_REGMAP_EXPORT RegmapPlugin : public QObject, public PluginBase
{
	friend class RegMap_API;
	Q_OBJECT
	SCOPY_PLUGIN;

public:
	bool loadPage() override;
	bool loadIcon() override;
	void loadToolList() override;
	void unload() override;
	bool compatible(QString uri, QString category) override;
	void initPreferences() override;
	bool loadPreferencesPage() override;

	void initMetadata() override;
	QString description() override;
	QString displayName() override;

	QWidget *getTool();

	void generateDevice(QString xmlPath, struct iio_device *dev, QString devName,
			    IRegisterReadStrategy *readStrategy, IRegisterWriteStrategy *writeStrategy,
			    int bitsPerRow = 8);

public Q_SLOTS:
	bool onConnect() override;
	bool onDisconnect() override;

private:
	QWidget *m_registerMapWidget = nullptr;
	QList<iio_device *> *m_deviceList = nullptr;
	struct iio_device *getIioDevice(iio_context *ctx, const char *dev_name);
	bool isBufferCapable(iio_device *dev);
	RegisterMapTool *registerMapTool;
	void InitApi();
	RegMap_API *api;
};
} // namespace regmap
} // namespace scopy

#endif // REGMAPPLUGIN_H
