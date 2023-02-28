#include "logging_categories.h"
#include "plugin/ichannelplugin.h"

#include <FftDisplayPlot.h>
#include <newinstrument.hpp>

using namespace adiscope;
using namespace gui;
class RefChannelPlugin : public IChannelPlugin
{
private:
	ChannelWidget* channel;
	QVector<double> xData;
	QVector<double> yData;
	NewInstrument *instrument;
	std::vector<DisplayPlot*> *plotList;
	QString name;

public:
	RefChannelPlugin(QWidget *parent, ToolView* toolView, ChannelManager *chManager, QVector<double> xData, QVector<double> yData, bool dockable) : IChannelPlugin(parent, toolView, chManager, dockable),
		xData(xData), yData(yData)
	{
		init();
	}

	~RefChannelPlugin()
	{

	}

	void init() override
	{
		qDebug(CAT_REFCHANNEL_PLUGIN) << "init()";

		instrument = dynamic_cast<NewInstrument *>(parent);
		plotList = instrument->getPlotList();

		menu = new GenericMenu(new QWidget());
		menu->initInteractiveMenu();
		menu->setMenuHeader("Ref CH", new QColor('gray'), true);

		name = "Ref CH " + QString::number(chManager->getChannelsCount());

		for (auto plot: *plotList) {
			auto fft_plot = dynamic_cast<FftDisplayPlot*>(plot);
			if (fft_plot != nullptr) {
				fft_plot->registerReferenceWaveform(name, xData, yData);
			}
		}

		channel = toolView->buildNewChannel(chManager, menu , dockable, -1, true, false, QColor(), "Reference channel " + QString::number(chManager->getChannelsCount()), name);

		for (auto plot: *plotList) {
			auto fft_plot = dynamic_cast<FftDisplayPlot*>(plot);
			if (fft_plot != nullptr) {
				channel->setIsPhysicalChannel(true);
				channel->setColor(fft_plot->getLineColor(chManager->getChannelID(channel)));
			}
		}

		Q_EMIT channel->selected(true);
	}
};
