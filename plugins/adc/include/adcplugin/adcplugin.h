#ifndef TESTPLUGIN_H
#define TESTPLUGIN_H

#define SCOPY_PLUGIN_NAME ADCPlugin

#include <QLabel>
#include <QObject>
#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>
#include "scopy-adcplugin_export.h"
#include <QLineEdit>
#include <iio.h>
#include "oscilloscope_plot.hpp"


#include <gr-util/grtopblock.h>
#include <gr-util/grproxyblock.h>
#include <gr-util/griiodevicesource.h>
#include <gr-util/griiofloatchannelsrc.h>
#include <gr-util/grscaleoffsetproc.h>
#include <gr-util/tooladdon.h>

#include <gr-util/grtimeplotaddon.h>
#include <gr-util/grtimeplotaddonsettings.h>
#include <gr-util/grdeviceaddon.h>
#include <gr-util/grtimechanneladdon.h>


namespace scopy {
using namespace grutil;
// timedomainplot addon
// x-y plot addon
// histogram plot addon

// timedomainchannel addon
// timedomainmathchannel addon
// referencechannel addon
// softtrigger menu addon
// cursor addon
// measurements addon

// spectrumchanneladdon
// fft sweep addon
// waterfall plot addon



class SCOPY_ADCPLUGIN_EXPORT PlotProxy {
public:
	virtual ToolAddon* getPlotAddon() = 0;
	virtual ToolAddon* getPlotSettings() = 0;
	virtual QList<ToolAddon*> getDeviceAddons() = 0;
	virtual QList<ToolAddon*> getChannelAddons() = 0;

};

class SCOPY_ADCPLUGIN_EXPORT GRTimePlotProxy : public QObject, public PlotProxy {
	Q_OBJECT
public:
	GRTimePlotProxy(QObject *parent = nullptr) : QObject(parent) { }
	~GRTimePlotProxy() {}

//	void addDeviceAddon(GRDeviceAddon* d);
//	void addChannelAddon(GRTimeChannelAddon *t);
	void setPlotAddon(GRTimePlotAddon *p, GRTimePlotAddonSettings *s) {
		this->plotAddon = p;
		this->plotSettingsAddon = s;
	}

	void addDeviceAddon(ToolAddon* d) {
		deviceAddons.append(d);
	}

	void removeDeviceAddon(ToolAddon* d) {
		deviceAddons.removeAll(d);
	}

	void addChannelAddon(ToolAddon *c) {
		channelAddons.append(c);
	}

	void removeChannelAddon(ToolAddon *c) {
		channelAddons.removeAll(c);
	}

	ToolAddon* getPlotAddon() override {
		return plotAddon;
	}

	ToolAddon* getPlotSettings() override {
		return plotSettingsAddon;
	}

	QList<ToolAddon*> getDeviceAddons() override {
		return deviceAddons;
	}

	QList<ToolAddon*> getChannelAddons() override {
		return channelAddons;
	}


	QString getPrefix() { return prefix; }
	void setPrefix(QString p) { prefix = p;}
private:

	GRTimePlotAddon *plotAddon;
	GRTimePlotAddonSettings *plotSettingsAddon;
	QList<ToolAddon*> deviceAddons;
	QList<ToolAddon*> channelAddons;

	QString prefix;
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
	PlotProxy* createRecipe(iio_context *ctx);
	GRTimePlotProxy *recipe;
};

}

#endif // TESTPLUGIN_H
