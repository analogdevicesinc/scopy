#ifndef SWIOTAD_HPP
#define SWIOTAD_HPP

#include "customcolqgridlayout.hpp"
#include "src/tool/tool_view.hpp"
#include "qwidget.h"
#include "swiotadlogic.hpp"
#include "swiotadreaderthread.hpp"
#include "swiotcontroller.hpp"
//#include "tool_launcher.hpp"
#include <QVector>


extern "C"{
	struct iio_device;
	struct iio_channel;
	struct iio_buffer;
}

namespace adiscope {

namespace gui {
class GenericMenu;
class ChannelManager;
}

class SwiotAd: public QWidget
{
	Q_OBJECT
public:
	explicit SwiotAd(QWidget* parent=nullptr, ToolLauncher* toolLauncher = nullptr,
			 struct iio_device* iioDev = nullptr, QVector<QString> chnlsFunc = {});
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
Q_SIGNALS:
	void chnlsStatusChanged();
private:
	struct iio_device* m_iioDev;
	int m_enabledChnlsNo = 0;

	adiscope::gui::ChannelManager* m_monitorChannelManager;
	adiscope::gui::ToolView* m_toolView;
	SwiotAdLogic* m_swiotAdLogic;
	QWidget* m_widget;

	CapturePlot* m_plot;
	QWidget* m_plotWidget;

	QVector<SwiotController*> m_controllers;
	QVector<QString> m_chnlsFunction;
	std::vector<ChannelWidget*> m_channelWidgetList;
	std::vector<bool> m_enabledChannels;

	QGridLayout* m_gridPlot;



	SwiotAdReaderThread* m_readerThread;

	bool m_testToggle;
private:
	void createMonitorChannelMenu();
	void connectChnlsWidgesToPlot(std::vector<ChannelWidget*> channelList);
	void drawCurves(std::vector<double*> dataPoints);
};
}
#endif // SWIOTAD_HPP
