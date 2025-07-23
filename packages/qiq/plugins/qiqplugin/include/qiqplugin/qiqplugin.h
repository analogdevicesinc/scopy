#ifndef QIQPLUGIN_H
#define QIQPLUGIN_H

#define SCOPY_PLUGIN_NAME QIQPlugin

#include "scopy-qiqplugin_export.h"
#include <QObject>
#include <iiomanager.h>
#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>

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
	IIOManager *m_iioManager;

	// test
	QVariantMap pskMap = {
		{"symbols_type",
		 QVariantMap{{"type", "string"},
			     {"required", true},
			     {"description", "Type of symbols to demodulate"},
			     {"available_values", QStringList{"BPSK", "QPSK", "8-PSK"}}}},
		{"filter_type",
		 QVariantMap{{"type", "string"},
			     {"required", false},
			     {"default", "None"},
			     {"description", "Type of filter to apply"},
			     {"available_values", QStringList{"None", "Raised Cosine", "Root Raised Cosine"}}}},
		{"symbol_rate",
		 QVariantMap{{"type", "int"},
			     {"required", true},
			     {"default", 1000000},
			     {"description", "Symbol rate"},
			     {"min", 1000},
			     {"max", 1000000000},
			     {"step", 1000}}},
		{"alpha",
		 QVariantMap{{"type", "double"},
			     {"required", false},
			     {"default", 0.3},
			     {"description", "Alpha parameter for filter"},
			     {"min", 0.0},
			     {"max", 1.0},
			     {"step", 0.1}}}};
};
} // namespace scopy::qiqplugin
#endif // QIQPLUGIN_H
