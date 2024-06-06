#ifndef ADMTPLUGIN_H
#define ADMTPLUGIN_H

#define SCOPY_PLUGIN_NAME ADMTPlugin

#include "scopy-admt_export.h"
#include <QObject>
#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>

namespace scopy::admt {
class SCOPY_ADMT_EXPORT ADMTPlugin : public QObject, public PluginBase
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

public Q_SLOTS:
	bool onConnect() override;
	bool onDisconnect() override;
};
} // namespace scopy::admt
#endif // ADMTPLUGIN_H
