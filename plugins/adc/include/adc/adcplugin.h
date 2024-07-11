#ifndef ADCPLUGIN_H
#define ADCPLUGIN_H

#define SCOPY_PLUGIN_NAME ADCPlugin

#include "src/adcacquisitionmanager.h"
#include "scopy-adc_export.h"

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

#include <adcinstrumentcontroller.h>
namespace scopy {
namespace adc {
using namespace grutil;

typedef enum {
	TIME,
	FREQUENCY
} ADCInstrumentType;

class SCOPY_ADC_EXPORT ADCPlugin : public QObject, public PluginBase
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

	void newInstrument(ADCInstrumentType t, AcqTreeNode *root);
	void deleteInstrument(ToolMenuEntry *w);
private:
	iio_context *m_ctx;
	QLineEdit *edit;
	QList<ADCInstrumentController*> m_ctrls;

	void createGRIIOTreeNode(GRTopBlockNode *node, iio_context *ctx);
};
} // namespace adc
} // namespace scopy

#endif // ADCPLUGIN_H
