#include "plugin/ichannelplugin.h"

using namespace adiscope;
using namespace gui;
class PhysicalChannelPlugin : public IChannelPlugin
{
private:
	std::vector<ChannelWidget*> channelList;

public:
	PhysicalChannelPlugin(QWidget *parent, ToolView* toolView, ChannelManager *chManager) : IChannelPlugin(parent, toolView, chManager) {}

	void init() override
	{
		menu = new GenericMenu(parent);
		menu->initInteractiveMenu();
		menu->setMenuHeader("Phys CH", new QColor('gray'), true);

		channelList.push_back(toolView->buildNewChannel(chManager, menu , false, -1, false, false, QColor("green"), "channel", "Phys CH"));

		for (auto ch: channelList) {
			ch->setIsPhysicalChannel(true);
		}
	}
};
