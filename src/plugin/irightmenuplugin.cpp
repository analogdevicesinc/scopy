#include "plugin/irightmenuplugin.h"

using namespace adiscope;
using namespace gui;
IRightMenuPlugin::IRightMenuPlugin(QWidget *parent, ToolView* toolView, bool dockable) : parent(parent), toolView(toolView), menu(nullptr), dockable(dockable)
{

}

IRightMenuPlugin::~IRightMenuPlugin()
{

}

void IRightMenuPlugin::init()
{
	menu = new GenericMenu(parent);
	menu->initInteractiveMenu();
	menu->setMenuHeader("RightMenu", new QColor('gray'), true);

	toolView->buildNewInstrumentMenu(menu, dockable, "RightMenu", false, false);
}
