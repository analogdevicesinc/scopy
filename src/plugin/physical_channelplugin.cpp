#include <FftDisplayPlot.h>
#include "logging_categories.h"
#include "plugin/ichannelplugin.h"
#include <newinstrument.hpp>

using namespace adiscope;
using namespace gui;
class PhysicalChannelPlugin : public IChannelPlugin
{
private:
	NewInstrument *instrument;
	std::vector<DisplayPlot*> *plotList;

public:
	PhysicalChannelPlugin(QWidget *parent, ToolView* toolView, ChannelManager *chManager, bool dockable) : IChannelPlugin(parent, toolView, chManager, dockable),
		instrument(dynamic_cast<NewInstrument *>(parent)),
		plotList(instrument->getPlotList())
	{
		init();
	}

	~PhysicalChannelPlugin()
	{

	}

	void init() override
	{
		qDebug(CAT_PHYSICALCHANNEL_PLUGIN) << "init()";

		menu = new GenericMenu(parent);
		menu->initInteractiveMenu();
		menu->setMenuHeader("Phys CH", new QColor('gray'), true);

		channelList.push_back(toolView->buildNewChannel(chManager, menu , dockable, -1, false, false, QColor(), "Physical channel", "Phys CH"));

		for (auto ch: channelList) {
			ch->setIsPhysicalChannel(true);

			for (auto plot: *plotList) {
				auto fft_plot = dynamic_cast<FftDisplayPlot*>(plot);
				if (fft_plot != nullptr) {
					fft_plot->addChannel();
					ch->setColor(fft_plot->getLineColor(chManager->getChannelID(ch)));

					Q_EMIT ch->selected(true);
				}
			}
		}
	}
};
