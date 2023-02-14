#include "logging_categories.h"
#include "plugin/ichannelplugin.h"

#include <FftDisplayPlot.h>
#include <newinstrument.hpp>

using namespace adiscope;
using namespace gui;
class RefChannelPlugin : public IChannelPlugin
{
private:
	std::vector<ChannelWidget*> channelList;
	QVector<double> xData;
	QVector<double> yData;
	NewInstrument *instrument;
	FftDisplayPlot *fft_plot;
	QString name;

public:
	RefChannelPlugin(QWidget *parent, ToolView* toolView, ChannelManager *chManager, QVector<double> xData, QVector<double> yData, bool dockable) : IChannelPlugin(parent, toolView, chManager, dockable), xData(xData), yData(yData) {}

	void init() override
	{
		qDebug(CAT_REFCHANNEL_PLUGIN) << "init()";

		instrument = dynamic_cast<NewInstrument *>(parent);
		fft_plot = instrument->getPlot();

		menu = new GenericMenu(new QWidget());
		menu->initInteractiveMenu();
		menu->setMenuHeader("Ref CH", new QColor('gray'), true);


		name = "Ref CH " + QString::number(fft_plot->getRef_data().size());
		fft_plot->registerReferenceWaveform(name, xData, yData);

		channelList.push_back(toolView->buildNewChannel(chManager, menu , dockable, -1, true, false, QColor(), "reference channel " + QString::number(fft_plot->getRef_data().size()), name));

		for (auto ch: channelList) {
			ch->setIsPhysicalChannel(true);
			ch->setColor(fft_plot->getLineColor(chManager->getChannelID(ch)));
		}
	}
};
