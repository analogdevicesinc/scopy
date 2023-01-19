#include "plugin/ichannelplugin.h"

using namespace adiscope;
using namespace gui;
class AddChannelPlugin : public IChannelPlugin
{
private:
	std::vector<ChannelWidget*> channelList;

public:
	AddChannelPlugin(QWidget *parent, ToolView* toolView, ChannelManager *chManager) : IChannelPlugin(parent, toolView, chManager) {}

	void init() override
	{
		menu = new GenericMenu(parent);
		menu->initInteractiveMenu();
		menu->setMenuHeader("Add CH", new QColor('gray'), true);

		chManager->insertAddBtn(menu, false);

	}
};
