#include <FftDisplayPlot.h>
#include "plugin/ichannelplugin.h"
#include <newinstrument.hpp>

using namespace adiscope;
using namespace gui;
class PhysicalChannelPlugin : public IChannelPlugin
{
private:
	NewInstrument *instrument;
	FftDisplayPlot *plot;

public:
	PhysicalChannelPlugin(QWidget *parent, ToolView* toolView, ChannelManager *chManager) : IChannelPlugin(parent, toolView, chManager),
		instrument(dynamic_cast<NewInstrument *>(parent)),
		plot(instrument->getPlot()) {
	}

	void init() override
	{
		menu = new GenericMenu(parent);
		menu->initInteractiveMenu();
		menu->setMenuHeader("Phys CH", new QColor('gray'), true);

		channelList.push_back(toolView->buildNewChannel(chManager, menu , false, -1, false, false, QColor(), "channel", "Phys CH"));

		for (auto ch: channelList) {
			ch->setIsPhysicalChannel(true);

			plot->addChannel();
			ch->setColor(plot->getLineColor(chManager->getChannelID(ch)));
		}
	}
};
