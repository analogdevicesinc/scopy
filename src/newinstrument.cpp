#include "newinstrument.hpp"
#include <gui/tool_view_builder.hpp>
#include "plugin/irightmenuplugin.h"
#include "plugin/physical_channelplugin.cpp"
#include <plugin/add_channelplugin.cpp>
#include <plugin/cursors_rightmenuplugin.cpp>
#include <plugin/fftplotplugin.h>
#include <plugin/marker_rightmenuplugin.hpp>


using namespace adiscope;
using namespace gui;

NewInstrument::NewInstrument(struct iio_context *ctx, Filter *filt,
			     ToolMenuItem *toolMenuItem,
			     QJSEngine *engine, ToolLauncher *parent):
	Tool(ctx, toolMenuItem, nullptr, "NewInstrument",
	     parent),
	ui(new Ui::NewInstrument)
{
	m_parent = parent;
	run_button = nullptr;
	ui->setupUi(this);

	m_recipe.helpBtnUrl = "";
	m_recipe.hasRunBtn = true;
	m_recipe.hasSingleBtn = true;
	m_recipe.hasPairSettingsBtn = true;
	m_recipe.hasPrintBtn = true;
	m_recipe.hasChannels = true;
	m_recipe.hasRightMenu = true;
	m_recipe.channelsPosition = adiscope::gui::ChannelsPositionEnum::VERTICAL;

	// managers
	m_channelManager = new adiscope::gui::ChannelManager(m_recipe.channelsPosition);
	m_channelManager->enableSwitchButton(true);
	m_channelManager->setChannelIdVisible(false);

	// tool view
	m_toolView = ToolViewBuilder(m_recipe, m_channelManager, m_parent).build();
	this->setLayout(new QVBoxLayout(this));
	this->layout()->addWidget(m_toolView);

	readPreferences();

	initGeneralSettings();
	addPlugins();

	connectSignals();
}

NewInstrument::~NewInstrument()
{
	for (auto plugin : m_pluginList) {
		delete plugin;
	}
	m_pluginList.clear();

	if (m_toolView->getRunBtn()->isChecked()) {
		m_toolView->getRunBtn()->setChecked(false);
	}

	if (m_toolView) {
		delete m_toolView;
		run_button = nullptr;
		m_toolView = nullptr;
	}
}

std::vector<DisplayPlot*> *NewInstrument::getPlotList()
{
	return &m_plotList;
}

void NewInstrument::initGeneralSettings()
{
	m_generalSettingsMenu = new GenericMenu(this);
	m_generalSettingsMenu->initInteractiveMenu();
	m_generalSettingsMenu->setMenuHeader("General Settings", new QColor("grey"), false);
	m_toolView->setGeneralSettingsMenu(m_generalSettingsMenu, dockable_menus);
}

void NewInstrument::connectSignals()
{
	connect(m_toolView->getRunBtn(), &QPushButton::toggled, this, [=](bool toggled){
		toolMenuItem->getToolStopBtn()->setChecked(toggled);
		startStop(toggled);
	});

	connect(m_toolView->getSingleBtn(), &QPushButton::toggled, this, [=](bool toggled){
		toolMenuItem->getToolStopBtn()->setChecked(toggled);
		startStop(toggled);
	});

	connect(toolMenuItem->getToolStopBtn(), &QPushButton::toggled, this, [=](bool toggled){
		m_toolView->getRunBtn()->setChecked(toggled);
		startStop(toggled);
	});

	connect(m_channelManager, &ChannelManager::selectedChannel, this, &NewInstrument::onChannelSelected);
	connect(m_channelManager, &ChannelManager::enabledChannel, this, &NewInstrument::onChannelEnabled);
	connect(m_channelManager, &ChannelManager::deletedChannel, this, &NewInstrument::onChannelDeleted);
}

void NewInstrument::onChannelSelected(int id)
{
	for (auto plot: m_plotList) {
		plot->bringCurveToFront(id);
		plot->setSelectedChannel(id);
		plot->replot();
	}
}

void NewInstrument::onChannelEnabled(int id, bool enabled)
{
	for (auto plot: m_plotList) {
		plot->Curve(id)->setVisible(enabled);
		plot->replot();
	}
}

void NewInstrument::onChannelDeleted(QString name)
{
	for (auto plot: m_plotList) {
		auto fft_plot = dynamic_cast<FftDisplayPlot*>(plot);
		if (fft_plot != nullptr) {
			fft_plot->unregisterReferenceWaveform(name);
		}
	}
}

void NewInstrument::start()
{
}

void NewInstrument::stop()
{
}

void NewInstrument::startStop(bool pressed)
{
	if (pressed) {
		start();
	} else {
		stop();
	}
}

void NewInstrument::addPlugin(BasePlugin *plugin)
{
	m_pluginList.push_back(plugin);
}

void NewInstrument::addPlot(DisplayPlot *plot)
{
	m_plotList.push_back(plot);
}

void NewInstrument::addPlugins()
{
//	m_pluginList.push_back(new FftPlotPlugin(this, m_toolView, m_channelManager, dockable_menus));
	auto fftPlugin = new FftPlotPlugin(this, m_toolView, m_channelManager, dockable_menus);
	m_pluginList.push_back(fftPlugin);

	m_pluginList.push_back(new PhysicalChannelPlugin(this, m_toolView, m_channelManager, dockable_menus));
	m_pluginList.push_back(new PhysicalChannelPlugin(this, m_toolView, m_channelManager, dockable_menus));
	//		m_pluginList.push_back(new IChannelPlugin(this, m_toolView, m_channelManager, dockable_menus));
	m_pluginList.push_back(new AddChannelPlugin(this, m_toolView, m_channelManager, dockable_menus));

	m_pluginList.push_back(new IRightMenuPlugin(this, m_toolView, dockable_menus));
	m_pluginList.push_back(new CursorsRightMenuPlugin(this, m_toolView, dockable_menus, true, true));
	m_pluginList.push_back(new MarkerRightMenuPlugin(this, m_toolView, dockable_menus, m_channelManager, fftPlugin->getPlot()));
}

void NewInstrument::readPreferences() {
	bool showFps = prefPanel->getShow_plot_fps();
	dockable_menus = prefPanel->getCurrent_docking_enabled();

	for (auto plot: m_plotList) {
		plot->setVisibleFpsLabel(showFps);
	}
}
