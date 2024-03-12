#ifndef SWIOTREFACTORPLUGIN_H
#define SWIOTREFACTORPLUGIN_H

#define SCOPY_PLUGIN_NAME SWIOTREFACTORPlugin

#include "scopy-swiotrefactor_export.h"
#include <QObject>
#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>

namespace scopy::swiotrefactor {
class SCOPY_SWIOTREFACTOR_EXPORT SWIOTREFACTORPlugin : public QObject, public PluginBase
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
} // namespace scopy::swiotrefactor
#endif // SWIOTREFACTORPLUGIN_H
