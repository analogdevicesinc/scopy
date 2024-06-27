#ifndef DACPLUGIN_H
#define DACPLUGIN_H

#define SCOPY_PLUGIN_NAME DACPlugin

#include "scopy-dac_export.h"
#include <QObject>
#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>

#include <iio.h>

namespace scopy {
namespace dac {
class SCOPY_DAC_EXPORT DACPlugin : public QObject, public PluginBase
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
	QString about() override;
	QString version() override;
	QString description() override;

public Q_SLOTS:
	bool onConnect() override;
	bool onDisconnect() override;

private:
	struct iio_context *m_ctx;
	QWidget *dac;
};
} // namespace dac
} // namespace scopy
#endif // DACPLUGIN_H
