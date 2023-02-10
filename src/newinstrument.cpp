#include "newinstrument.hpp"
#include <gui/tool_view_builder.hpp>
#include "gui/dynamicWidget.hpp"
#include "plugin/ichannelplugin.h"
#include "plugin/irightmenuplugin.h"
#include "plugin/physical_channelplugin.cpp"
#include <plugin/add_channelplugin.cpp>
#include <plugin/cursors_rightmenuplugin.cpp>


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

	initPlot();
	readPreferences();

	// add plot
	if (dockable_menus) {
		m_toolView->addDockableTabbedWidget(fft_plot->getPlotwithElements(), "FFT Plot");
	}
	else {
		m_toolView->addFixedCentralWidget(fft_plot->getPlotwithElements(), 0, 0, 1, 1);
	}

	initGeneralSettings();
	addPlugins();

	connectSignals();
}

NewInstrument::~NewInstrument()
{
	for (auto ch : m_channelPluginList) {
		delete ch;
	}
	for (auto rm : m_rightMenuPluginList) {
		delete rm;
	}
	m_channelPluginList.clear();
	m_rightMenuPluginList.clear();

	if (m_toolView->getRunBtn()->isChecked()) {
			m_toolView->getRunBtn()->setChecked(false);
	}

	if (m_toolView) {
		delete m_toolView;
		run_button = nullptr;
		m_toolView = nullptr;
	}
}

void NewInstrument::initPlot()
{
	double start = 0, stop = 50000000;

	m_toolView->getCentralWidget()->setStyleSheet("background-color: black;");

	fft_plot = new FftDisplayPlot(m_channelPluginList.size(), this);
//	fft_plot->disableLegend();
	fft_plot->setXaxisMouseGesturesEnabled(false);

	fft_plot->setZoomerEnabled();
	fft_plot->setAxisVisible(QwtAxis::XBottom, false);
	fft_plot->setAxisVisible(QwtAxis::YLeft, false);
	fft_plot->setUsingLeftAxisScales(false);
	fft_plot->enableXaxisLabels();
	fft_plot->enableYaxisLabels();
//	setYAxisUnit(ui->cmb_units->currentText());
	fft_plot->setBtmHorAxisUnit("Hz");

//	fft_plot->setStartStop(start, stop);
	fft_plot->setAxisScale(QwtAxis::XBottom, start, stop);
	fft_plot->replot();
	fft_plot->bottomHandlesArea()->repaint();
}

FftDisplayPlot *NewInstrument::getPlot()
{
	return fft_plot;
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

	connect(this, SIGNAL(selectedChannelChanged(int)),
		fft_plot, SLOT(setSelectedChannel(int)));

	connect(m_channelManager, &ChannelManager::selectedChannel, this, &NewInstrument::onChannelSelected);
	connect(m_channelManager, &ChannelManager::enabledChannel, this, &NewInstrument::onChannelEnabled);
	connect(m_channelManager, &ChannelManager::deletedChannel, this, &NewInstrument::onChannelDeleted);
}

void NewInstrument::onChannelSelected(int id)
{
	fft_plot->bringCurveToFront(id);
	fft_plot->setSelectedChannel(id);
	fft_plot->replot();
}

void NewInstrument::onChannelEnabled(int id, bool enabled)
{
	fft_plot->Curve(id)->setVisible(enabled);
	fft_plot->replot();
}

void NewInstrument::onChannelDeleted(QString name)
{
	fft_plot->unregisterReferenceWaveform(name);
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

//	setDynamicProperty(run_button, "running", pressed);
}

void NewInstrument::addChannelPlugin(IChannelPlugin *plugin)
{
	if (m_recipe.hasChannels) {
		m_channelPluginList.push_back(plugin);

		plugin->init();
	}
}

void NewInstrument::addPlugins()
{
	if (m_recipe.hasChannels) {
		m_channelPluginList.push_back(new PhysicalChannelPlugin(this, m_toolView, m_channelManager, dockable_menus));
		m_channelPluginList.push_back(new PhysicalChannelPlugin(this, m_toolView, m_channelManager, dockable_menus));
//		m_channelPluginList.push_back(new IChannelPlugin(this, m_toolView, m_channelManager, dockable_menus));
//		m_channelPluginList.push_back(new AddChannelPlugin(this, m_toolView, m_channelManager, dockable_menus));

		m_rightMenuPluginList.push_back(new IRightMenuPlugin(this, m_toolView, dockable_menus));
		m_rightMenuPluginList.push_back(new CursorsRightMenuPlugin(this, m_toolView, dockable_menus, true, true, m_channelManager, getPlot()));

		// init plugins
		for (int i=0; i<m_channelPluginList.size(); i++) {
			m_channelPluginList[i]->init();
		}

		for (auto ch: m_rightMenuPluginList) {
			ch->init();
		}
	}
}

void NewInstrument::readPreferences() {
	bool showFps = prefPanel->getShow_plot_fps();
	dockable_menus = prefPanel->getCurrent_docking_enabled();
	fft_plot->setVisibleFpsLabel(showFps);
}
