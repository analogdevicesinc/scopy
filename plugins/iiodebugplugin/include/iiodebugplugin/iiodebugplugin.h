#ifndef IIODEBUGPLUGIN_H
#define IIODEBUGPLUGIN_H

#define SCOPY_PLUGIN_NAME IIODebugPlugin

#include "scopy-iiodebugplugin_export.h"
#include <QObject>
#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>
#include "iiodebuginstrument.h"

namespace scopy::iiodebugplugin {
class SCOPY_IIODEBUGPLUGIN_EXPORT IIODebugPlugin : public QObject, public PluginBase
{
	Q_OBJECT
	SCOPY_PLUGIN;

public:
	friend class IIODebugPlugin_API;
	bool compatible(QString m_param, QString category) override;
	bool loadPage() override;
	bool loadIcon() override;
	void loadToolList() override;
	void unload() override;
	void initMetadata() override;
	QString description() override;
	QString version() override;
	void saveSettings(QSettings &) override;
	void loadSettings(QSettings &) override;

public Q_SLOTS:
	bool onConnect() override;
	bool onDisconnect() override;

private:
	ApiObject *m_pluginApi;
	IIODebugInstrument *m_iioDebugger;
};

class SCOPY_IIODEBUGPLUGIN_EXPORT IIODebugPlugin_API : public ApiObject
{
	Q_OBJECT
public:
	explicit IIODebugPlugin_API(IIODebugPlugin *p)
		: ApiObject(p)
		, p(p)
	{}
	~IIODebugPlugin_API(){};
	IIODebugPlugin *p;

	Q_PROPERTY(QString debugTest READ debugTest WRITE setDebugTest NOTIFY debugTestChanged FINAL)
	QString debugTest() const;
	void setDebugTest(const QString &newDebugTest);

Q_SIGNALS:
	void debugTestChanged();

private:
	QString m_debugTest;
};

} // namespace scopy::iiodebugplugin
#endif // IIODEBUGPLUGIN_H
