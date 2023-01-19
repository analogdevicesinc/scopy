#include "plugin/ichannelplugin.h"
#include "qlayout.h"

using namespace adiscope;
using namespace gui;
IChannelPlugin::IChannelPlugin(QWidget *parent, ToolView* toolView, gui::ChannelManager *chManager) : parent(parent), toolView(toolView), chManager(chManager), menu(nullptr)
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

	ChannelWidget *ch = toolView->buildNewChannel(chManager, menu , false, -1, false, false, QColor(), "I channel", "CH");
	ch->setIsPhysicalChannel(true);
}
