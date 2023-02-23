#ifndef FFTPLOTPLUGIN_H
#define FFTPLOTPLUGIN_H


#include <FftDisplayPlot.h>
#include <QObject>
#include <QWidget>

#include <gui/channel_manager.hpp>

#include <libm2k/analog/genericanalogin.hpp>

#include "gui/tool_view.hpp"
#include "baseplugin.h"

namespace adiscope {
namespace gui {
class FftPlotPlugin : public BasePlugin
{
	Q_OBJECT
private:
	int sample_rate_divider;
	double m_max_sample_rate;
	double sample_rate;
	libm2k::analog::GenericAnalogIn* m_generic_analogin;
	ChannelManager* chManager;

	void setSampleRate(double sr);
protected:
	gui::ToolView* toolView;
	FftDisplayPlot* fft_plot;

public:
	explicit FftPlotPlugin(QWidget *parent = nullptr, gui::ToolView* toolView = nullptr, ChannelManager* chManager = nullptr, bool dockable = false);
	~FftPlotPlugin();

	void init() override;
	void connectSignals();
	FftDisplayPlot *getPlot();
};
}
}

#endif // FFTPLOTPLUGIN_H
