#ifndef TESTPLUGIN_H
#define TESTPLUGIN_H

#define SCOPY_PLUGIN_NAME ADCPlugin

#include "scopy-adcplugin_export.h"

#include <iio.h>

#include <QLabel>
#include <QLineEdit>
#include <QObject>

#include <gr-util/grdeviceaddon.h>
#include <gr-util/griiodevicesource.h>
#include <gr-util/griiofloatchannelsrc.h>
#include <gr-util/grproxyblock.h>
#include <gr-util/grscaleoffsetproc.h>
#include <gr-util/grtimechanneladdon.h>
#include <gr-util/grtimeplotaddon.h>
#include <gr-util/grtimeplotaddonsettings.h>
#include <gr-util/grtopblock.h>
#include <gr-util/tooladdon.h>
#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>

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

class SCOPY_ADCPLUGIN_EXPORT PlotProxy
{
public:
	virtual ToolAddon *getPlotAddon() = 0;
	virtual ToolAddon *getPlotSettings() = 0;
	virtual QList<ToolAddon *> getDeviceAddons() = 0;
	virtual QList<ToolAddon *> getChannelAddons() = 0;
	virtual QList<ToolAddon *> getAddons() = 0;

	virtual void addDeviceAddon(ToolAddon *d) = 0;
	virtual void removeDeviceAddon(ToolAddon *d) = 0;
	virtual void addChannelAddon(ChannelAddon *c) = 0;
	virtual void removeChannelAddon(ChannelAddon *c) = 0;
	virtual void init() = 0;

	virtual ChannelIdProvider *getChannelIdProvider() = 0;
};

class SCOPY_ADCPLUGIN_EXPORT GRTimePlotProxy : public QObject, public PlotProxy
{
	Q_OBJECT
public:
	GRTimePlotProxy(QObject *parent = nullptr)
		: QObject(parent)
	{
		chIdP = new ChannelIdProvider(this);
	}
	~GRTimePlotProxy() {}

	void setPlotAddon(GRTimePlotAddon *p, GRTimePlotAddonSettings *s)
	{
		this->plotAddon = p;
		this->plotSettingsAddon = s;
	}

	void addDeviceAddon(ToolAddon *d) override { deviceAddons.append(d); }

	void removeDeviceAddon(ToolAddon *d) override { deviceAddons.removeAll(d); }

	void addChannelAddon(ChannelAddon *c) override { channelAddons.append(c); }

	void removeChannelAddon(ChannelAddon *c) override { channelAddons.removeAll(c); }

	ToolAddon *getPlotAddon() override { return plotAddon; }

	ToolAddon *getPlotSettings() override { return plotSettingsAddon; }

	QList<ToolAddon *> getDeviceAddons() override { return deviceAddons; }

	QList<ToolAddon *> getChannelAddons() override { return channelAddons; }

	QList<ToolAddon *> getAddons() override
	{
		QList<ToolAddon *> addons;

		addons.append(channelAddons);
		addons.append(deviceAddons);
		addons.append(plotSettingsAddon);
		addons.append(plotAddon);
		return addons;
	}

	void init() override
	{
		for(auto *addon : getAddons()) {
			if(dynamic_cast<GRTopAddon *>(addon)) {
				auto GRAddon = dynamic_cast<GRTopAddon *>(addon);
				connect(topBlock, &GRTopBlock::aboutToStart, this, [=]() { GRAddon->preFlowStart(); });
				connect(topBlock, &GRTopBlock::started, this, [=]() { GRAddon->postFlowStart(); });
				connect(topBlock, &GRTopBlock::aboutToStop, this, [=]() { GRAddon->preFlowStop(); });
				connect(topBlock, &GRTopBlock::stopped, this, [=]() { GRAddon->postFlowStop(); });
				connect(topBlock, &GRTopBlock::aboutToBuild, this, [=]() { GRAddon->preFlowBuild(); });
				connect(topBlock, &GRTopBlock::builtSignalPaths, this,
					[=]() { GRAddon->postFlowBuild(); });
				connect(topBlock, &GRTopBlock::aboutToTeardown, this,
					[=]() { GRAddon->preFlowTeardown(); });
				connect(topBlock, &GRTopBlock::teardownSignalPaths, this,
					[=]() { GRAddon->postFlowTeardown(); });
			}
		}
	}

	ChannelIdProvider *getChannelIdProvider() override { return chIdP; }

	QString getPrefix() { return prefix; }
	void setPrefix(QString p) { prefix = p; }
	GRTopBlock *getTopBlock() const { return topBlock; }
	void setTopBlock(GRTopBlock *newTopBlock) { topBlock = newTopBlock; }

private:
	GRTimePlotAddon *plotAddon;
	GRTimePlotAddonSettings *plotSettingsAddon;
	QList<ToolAddon *> deviceAddons;
	QList<ToolAddon *> channelAddons;
	GRTopBlock *topBlock;
	ChannelIdProvider *chIdP;

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
	PlotProxy *createRecipe(iio_context *ctx);
	GRTimePlotProxy *recipe;
};

} // namespace scopy

#endif // TESTPLUGIN_H
