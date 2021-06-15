#include "menu_anim.hpp"

#include <QLayout>
#include <QDebug>

#include <scopy/gui/FftDisplayPlot.h>
#include <scopy/gui/HistogramDisplayPlot.h>
#include <scopy/gui/channel_settings.hpp>
#include <scopy/gui/cursors_settings.hpp>
#include <scopy/gui/logic_analyzer_channel_menu.hpp>
#include <scopy/gui/logic_analyzer_general_menu.hpp>
#include <scopy/gui/logic_pattern_trigger_menu.hpp>
#include <scopy/gui/measure_settings.hpp>
#include <scopy/gui/network_analyzer_channel_menu.hpp>
#include <scopy/gui/network_analyzer_general_menu.hpp>
#include <scopy/gui/oscilloscope_plot.hpp>
#include <scopy/gui/pattern_generator_channel_menu.hpp>
#include <scopy/gui/pattern_generator_general_menu.hpp>
#include <scopy/gui/power_supply_menu.hpp>
#include <scopy/gui/signal_generator_menu.hpp>
#include <scopy/gui/spectrum_analyzer_add_reference_menu.hpp>
#include <scopy/gui/spectrum_analyzer_channel_menu.hpp>
#include <scopy/gui/spectrum_analyzer_general_menu.hpp>
#include <scopy/gui/spectrum_analyzer_markers_menu.hpp>
#include <scopy/gui/spectrum_analyzer_sweep_menu.hpp>
#include <scopy/gui/test_menu.hpp>
#include <scopy/gui/test_tool.hpp>
#include <scopy/gui/voltmeter_menu.hpp>

using namespace scopy::gui;

TestTool::TestTool()
{
	// Spectrum Analyzer wannabe

	ToolViewRecipe recipe;
	recipe.helpBtnUrl = "https://google.com";
	recipe.hasRunBtn = true;
	recipe.hasSingleBtn = true;
	recipe.hasPrintBtn = true;

	recipe.hasPairSettingsBtn = true;

	recipe.hasChannels = true;
	//	recipe.channelsPosition = ChannelsPositionEnum::VERTICAL;

	ChannelManager* channelManager = new ChannelManager(recipe.channelsPosition);

	m_toolView = ToolViewBuilder(recipe, channelManager).build();

	m_toolView->setGeneralSettingsMenu(new SpectrumAnalyzerGeneralMenu, true);

	m_toolView->buildNewInstrumentMenu(new SpectrumAnalyzerSweepMenu, true, "Sweep");
	m_toolView->buildNewInstrumentMenu(new SpectrumAnalyzerMarkersMenu, true, "Markers");
	m_toolView->buildNewInstrumentMenu(new TestMenu, true, "Interactive");

	//	MeasureSettings* measureMenu = new MeasureSettings;
	//	measureMenu->setChannelName("Channel 1");
	//	measureMenu->setChannelUnderlineColor(new QColor("#FF7200"));

	//	m_toolView->buildNewInstrumentMenu(new CursorsSettings, true, "Cursors", true, true);
	//	m_toolView->buildNewInstrumentMenu(measureMenu, true, "Measure", true, true);
	//	m_toolView->buildNewInstrumentMenu(new TriggerMenu, true, "Trigger");


	SpectrumAnalyzerChannelMenu* ch1Menu = new SpectrumAnalyzerChannelMenu("Channel 1", new QColor("#FF7200"));
	SpectrumAnalyzerChannelMenu* ch2Menu = new SpectrumAnalyzerChannelMenu("Channel 2", new QColor("#9013FE"));
	SpectrumAnalyzerChannelMenu* ch3Menu = new SpectrumAnalyzerChannelMenu("Math 1", new QColor("green"));

	m_toolView->buildNewChannel(channelManager, ch1Menu, true, 0, false, false, QColor("#FF7200"), "Channel", "CH");
	m_toolView->buildNewChannel(channelManager, ch2Menu, true, 1, false, false, QColor("#9013FE"), "Channel", "CH");
	m_toolView->buildNewChannel(channelManager, ch3Menu, true, 0, true, false, QColor("green"), "Math", "M");

	channelManager->insertAddBtn(new SpectrumAnalyzerAddReferenceMenu, true);


	CapturePlot* plot = new CapturePlot(this, false, 10, 10, new MetricPrefixFormatter, new MetricPrefixFormatter);
	plot->disableLegend();

	unsigned int noPoints = 1024;
	unsigned int sampleRate = 1024;

	QVector<double> data;
	unsigned int maxValue = 99;
	for(unsigned int i = 0; i < noPoints; i++) {
		data.push_back(i % 100);
	}

	QVector<double> indexes(1024);
	std::iota(std::begin(indexes), std::end(indexes), 0);

	plot->registerReferenceWaveform("", indexes, data);

	plot->Curve(0)->setAxes(
			QwtAxisId(QwtPlot::xBottom, 0),
			QwtAxisId(QwtPlot::yLeft, 0));
	plot->addZoomer(0);
	plot->Curve(0)->setTitle("CH " + QString::number(1));

	/* Default plot settings */
	plot->setZoomerEnabled(true);

	plot->setSampleRate(sampleRate, 1, "");

	plot->setHorizUnitsPerDiv(noPoints/10.0);
	plot->setVertUnitsPerDiv(maxValue/10.0);

	//	plot->setHorizOffset(noPoints/2.0);
	//	plot->setVertOffset(maxValue/2.0);

	plot->setYaxis(0, 99);
	plot->setXaxis(0, 1024);

	plot->setXaxisUnit("Spl");
	plot->setYaxisUnit("Val");

	m_toolView->addCentralWidget(plot, false, "");


	// dockable plots
	//	m_toolView->addCentralWidget(new CapturePlot(this, false, 10, 10), true, "1");
	//	m_toolView->addCentralWidget(new CapturePlot(this, false, 10, 10), true, "2");
	//	m_toolView->addCentralWidget(new CapturePlot(this, false, 10, 10), true, "3");
	//	m_toolView->addCentralWidget(new CapturePlot(this, false, 10, 10), true, "4");
	//	m_toolView->addCentralWidget(new CapturePlot(this, false, 10, 10), true, "5");
	//	m_toolView->addCentralWidget(new CapturePlot(this, false, 10, 10), true, "6");
	//	m_toolView->addCentralWidget(new CapturePlot(this, false, 10, 10), true, "7");

	// fixed plots
	//	m_toolView->addCentralWidget(new CapturePlot(this, false, 10, 10), false, "");
	//	m_toolView->addCentralWidget(new CapturePlot(this, false, 10, 10), false, "", 0, 1);
	//	m_toolView->addCentralWidget(new CapturePlot(this, false, 10, 10), false, "", 1, 0, 2, 0);

	//	FftDisplayPlot* fft_plot = new FftDisplayPlot(3, centralWidget);

	//	fft_plot->disableLegend();

	//	// Disable mouse interactions with the axes until they are in a working state
	//	fft_plot->setXaxisMouseGesturesEnabled(false);

	//	for (uint i = 0; i < 3; i++) {
	//		fft_plot->setYaxisMouseGesturesEnabled(i, false);
	//	}

	//	centralWidget->layout()->addWidget(fft_plot->getPlotwithElements());

	//	fft_plot->enableXaxisLabels();
	//	fft_plot->enableYaxisLabels();

	//	m_toolView->setInstrumentNotesVisible(true);
}

ToolView* TestTool::getToolView() { return m_toolView; }
