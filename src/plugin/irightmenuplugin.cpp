#include "plugin/irightmenuplugin.h"
#include "logging_categories.h"

using namespace adiscope;
using namespace gui;
IRightMenuPlugin::IRightMenuPlugin(QWidget *parent, ToolView* toolView, bool dockable) : BasePlugin(parent, dockable), toolView(toolView), menu(nullptr)
{
}

IRightMenuPlugin::~IRightMenuPlugin()
{

}

void IRightMenuPlugin::init()
{
	qDebug(CAT_IRIGHTMENU_PLUGIN) << "init()";

	menu = new GenericMenu(parent);
	menu->initInteractiveMenu();
	menu->setMenuHeader("RightMenu", new QColor('gray'), true);

	toolView->buildNewInstrumentMenu(menu, dockable, "RightMenu", false, false);
}
