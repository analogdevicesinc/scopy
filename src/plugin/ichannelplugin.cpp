#include "plugin/ichannelplugin.h"
#include "qlayout.h"

using namespace adiscope;
using namespace gui;
IChannelPlugin::IChannelPlugin(QWidget *parent, ToolView* toolView, gui::ChannelManager *chManager) : parent(parent), toolView(toolView), chManager(chManager), menu(nullptr), ch(nullptr)
{

}

IChannelPlugin::~IChannelPlugin()
{

}

void IChannelPlugin::init()
{
	menu = new GenericMenu(parent);
	menu->initInteractiveMenu();
	menu->setMenuHeader("CH", new QColor('gray'), true);

	ch = toolView->buildNewChannel(chManager, menu , false, -1, false, false, QColor(), "I channel", "CH");
	ch->setIsPhysicalChannel(true);

	channelList.push_back(ch);
}

void IChannelPlugin::setColor(QColor color)
{
	if (ch != nullptr) {
		ch->setColor(color);
	}
}

std::vector<ChannelWidget*> IChannelPlugin::getChannelList()
{
	return std::ref(channelList);
}
