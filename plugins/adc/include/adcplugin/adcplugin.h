#ifndef TESTPLUGIN_H
#define TESTPLUGIN_H

#define SCOPY_PLUGIN_NAME ADCPlugin

#include "src/adcacquisitionmanager.h"
#include "scopy-adcplugin_export.h"

#include <iio.h>

#include <QLabel>
#include <QLineEdit>
#include <QObject>

#include <gr-util/griiodevicesource.h>
#include <gr-util/griiofloatchannelsrc.h>
#include <gr-util/grproxyblock.h>
#include <gr-util/grscaleoffsetproc.h>
#include <gr-util/grtopblock.h>
#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>
#include <gui/stylehelper.h>

namespace scopy {
namespace adc {
using namespace grutil;

class SCOPY_ADCPLUGIN_EXPORT ChannelIdProvider : public QObject
{
	Q_OBJECT
public:
	ChannelIdProvider(QObject *parent)
		: QObject(parent)
	{
		idx = 0;
	}
	virtual ~ChannelIdProvider() {}

	int next() { return idx++; }
	QPen pen(int idx) { return QPen(StyleHelper::getColor("CH" + QString::number(idx))); }

	int idx;
};

class SCOPY_ADCPLUGIN_EXPORT ADCPlugin : public QObject, public PluginBase
{
	Q_OBJECT
	SCOPY_PLUGIN;

	// Plugin interface
public:
	void initPreferences() override;
	void initMetadata() override;
	bool compatible(QString m_param, QString category) override;
	bool loadPreferencesPage() override;
	bool loadPage() override;
	bool loadIcon() override;
	void loadToolList() override;
	QString about() override;
	QString version() override;
	bool onConnect() override;
	bool onDisconnect() override;
	void saveSettings(QSettings &) override;
	void loadSettings(QSettings &) override;

private:
	iio_context *m_ctx;
	QWidget *time;
	QLineEdit *edit;

	void createGRIIOTreeNode(GRTopBlockNode *node, iio_context *ctx);

};
} // namespace adc
} // namespace scopy

#endif // TESTPLUGIN_H
