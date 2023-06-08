#include "adcinstrument.h"
#include <gui/tool_view.hpp>
#include <gui/tool_view_builder.hpp>

using namespace scopy;
using namespace scopy::grutil;
AdcInstrument::AdcInstrument(PlotProxy* proxy, QWidget *parent) : QWidget(parent), proxy(proxy)
{

	// TODO LIST
	//
	scopy::gui::ToolViewRecipe recipe;
	recipe.helpBtnUrl = "";
	recipe.hasRunBtn = true;
	recipe.hasSingleBtn = true;
	recipe.hasPairSettingsBtn = true;
	recipe.hasPrintBtn = false;
	recipe.hasChannels = true;
	recipe.channelsPosition = scopy::gui::ChannelsPositionEnum::VERTICAL;

	auto m_monitorChannelManager = new scopy::gui::ChannelManager(recipe.channelsPosition);
	m_monitorChannelManager->setChannelIdVisible(false);
	m_monitorChannelManager->setToolStatus(" Channels");


	GRTimePlotAddon* plotAddon = dynamic_cast<GRTimePlotAddon*>(proxy->getPlotAddon());

	auto m_toolView = scopy::gui::ToolViewBuilder(recipe, m_monitorChannelManager, this).build();

	for(auto d: proxy->getDeviceAddons()) {
		GRDeviceAddon *dev = dynamic_cast<GRDeviceAddon*>(d);
		if(!dev)
			return;


		std::vector<ChannelWidget*> chGroup;
		auto dev_ch = m_toolView->buildNewChannel(m_monitorChannelManager,
						       new gui::GenericMenu(),
						       false,
						       -1,
						       false,
						       false,
						       QColor("black"),
						       "LongName - Test1",
						       d->getName());
		chGroup.push_back(dev_ch);

		for(auto c: dev->getRegisteredChannels()) {
			auto ch = m_toolView->buildNewChannel(m_monitorChannelManager,
							      new gui::GenericMenu(),
							      false,
							      -1,
							      false,
							      false,
							      QColor("black"),
							      "LongName - Test1",
							      c->getName());
			plotAddon->onChannelAdded(c);
			auto plot = plotAddon->plot();
			auto curveId = plot->getAnalogChannels() - 1;
			auto color = plot->getLineColor(curveId);
			ch->setColor(color);
			plot->Curve(curveId)->setAxes(
			    QwtAxisId(QwtAxis::XBottom, 0),
			    QwtAxisId(QwtAxis::YLeft, curveId));

//			plot->Curve(curveId)->
			plot->DetachCurve(curveId);
			plot->AttachCurve(curveId);
//			plot->addZoomer(curveId);

			ch->setMenuButtonVisibility(false);
			chGroup.push_back(ch);
		}

		m_toolView->buildChannelGroup(m_monitorChannelManager,
					      chGroup[0],
					      chGroup
					      );



	}


	m_toolView->addFixedCentralWidget(proxy->getPlotAddon()->getWidget());
	m_toolView->setGeneralSettingsMenu(proxy->getPlotSettings()->getWidget(),false);
	connect(m_toolView->getRunBtn(), SIGNAL(toggled(bool)), this, SLOT(run(bool)));


//	gui::GenericMenu *settingsMenu = createSettingsMenu("General settings", new QColor("Red"));
//	m_toolView->setGeneralSettingsMenu(settingsMenu, true);

}

void AdcInstrument::run(bool b) {
	qInfo()<<b;

	for(auto ch : proxy->getChannelAddons()) {
		if(b)
			ch->onStart();
		else
			ch->onStop();
	}
	for(auto dev : proxy->getDeviceAddons()) {
		if(b)
			dev->onStart();
		else
			dev->onStop();
	}

	if(b)
		proxy->getPlotAddon()->onStart();
	else
		proxy->getPlotAddon()->onStop();



}





