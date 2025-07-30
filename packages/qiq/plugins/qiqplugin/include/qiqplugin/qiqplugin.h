#ifndef QIQPLUGIN_H
#define QIQPLUGIN_H

#define SCOPY_PLUGIN_NAME QIQPlugin

#include "scopy-qiqplugin_export.h"
#include <QObject>
#include <iiomanager.h>
#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>
#include <qiqcontroller/qiqcontroller.h>

namespace scopy::qiqplugin {
class SCOPY_QIQPLUGIN_EXPORT QIQPlugin : public QObject, public PluginBase
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

private:
	QIQController *m_qiqController;
	IIOManager *m_iioManager;
};
} // namespace scopy::qiqplugin
#endif // QIQPLUGIN_H
