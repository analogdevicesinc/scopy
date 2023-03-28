#ifndef SWIOTAD_HPP
#define SWIOTAD_HPP

#include "src/refactoring/maincore/customcolqgridlayout.hpp"
#include "src/refactoring/tool/tool_view.hpp"
#include "qwidget.h"
#include "swiotadlogic.hpp"
#include "swiotadreaderthread.hpp"
#include "swiotcontroller.hpp"
#include "src/refactoring/maingui/channel_manager.hpp"
#include "captureplot/oscilloscope_plot.hpp"
#include <QVector>
#include "src/refactoring/tool/tool_view_builder.hpp"

extern "C"{
	struct iio_device;
	struct iio_channel;
	struct iio_buffer;
}

namespace adiscope {

namespace gui {
class GenericMenu;
}

class SwiotAd: public QWidget
{
	Q_OBJECT
public:
	explicit SwiotAd(QWidget* parent=nullptr, struct iio_device* iioDev = nullptr,
			 QVector<QString> chnlsFunc = {});
	~SwiotAd();

	void initMonitorToolView();
	void initPlot();
	void setChannelsFunction(QVector<QString> chnlsFunction);
	void verifyChnlsChanges();
	adiscope::gui::ToolView* getToolView();

public Q_SLOTS:
	void onChannelWidgetEnabled(bool en);
	void onChannelWidgetSelected(bool checked);
	void onRunBtnPressed();
	void onBufferRefilled(QVector<QVector<double>>);
	void onReaderThreadFinished();
private:
	void createMonitorChannelMenu();
	void connectChnlsWidgesToPlot(std::vector<ChannelWidget*> channelList);
	void drawCurves(std::vector<double*> dataPoints, int numberOfPoints);
	void resetPlot();
private:
	struct iio_device* m_iioDev;
	int m_enabledChnlsNo = 0;
	int m_sampleRate = 4800;
	double m_timespan = 1;

	adiscope::gui::ChannelManager* m_monitorChannelManager;
	adiscope::gui::ToolView* m_toolView;
	SwiotAdLogic* m_swiotAdLogic;
	QWidget* m_widget;

	CapturePlot* m_plot;
	QWidget* m_plotWidget;

	QVector<SwiotController*> m_controllers;
	QVector<QString> m_chnlsFunction;

	std::vector<bool> m_enabledChannels;
	std::vector<bool> m_enabledPlots;

	QGridLayout* m_gridPlot;

	SwiotAdReaderThread *m_readerThread;
};
}
#endif // SWIOTAD_HPP
