#ifndef SWIOTAD_HPP
#define SWIOTAD_HPP

#include "src/refactoring/maincore/customcolqgridlayout.hpp"
#include "src/refactoring/tool/tool_view.hpp"
#include "qwidget.h"
#include "swiotadlogic.hpp"
#include "swiotadreaderthread.hpp"
#include "swiotcontroller.hpp"
#include "src/refactoring/maingui/channel_manager.hpp"
#include "captureplot/osc_export_settings.h"
#include <QVector>
#include "src/refactoring/tool/tool_view_builder.hpp"
#include "swiotplothandler.hpp"

#define MAX_CURVES_NUMBER 8

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
	void initExportSettings(QWidget *parent);
	void setChannelsFunction(QVector<QString> chnlsFunction);
	void verifyChnlsChanges();
	adiscope::gui::GenericMenu* createSettingsMenu(QString title, QColor* color);
	adiscope::gui::ToolView* getToolView();

public Q_SLOTS:
	void onChannelWidgetEnabled(bool en);
	void onChannelWidgetSelected(bool checked);
	void onRunBtnPressed();
	void onReaderThreadFinished();
Q_SIGNALS:
	void plotChnlsChanges(std::vector<bool> m_enabledPlots);
	void exportBtnClicked(QMap<int, bool> exportConfig);
	void activateExportButton();
private:
	void createMonitorChannelMenu();
	void connectChnlsWidgesToPlot(std::vector<ChannelWidget*> channelList);
private:
	struct iio_device* m_iioDev;
	int m_enabledChnlsNo = 0;

	adiscope::gui::ChannelManager* m_monitorChannelManager;
	adiscope::gui::ToolView* m_toolView;
	QWidget* m_widget;

	QVector<SwiotController*> m_controllers;
	QVector<QString> m_chnlsFunction;

	std::vector<bool> m_enabledChannels;
	std::vector<bool> m_enabledPlots;

	SwiotAdLogic* m_swiotAdLogic;
	SwiotAdReaderThread *m_readerThread;
	SwiotPlotHandler *m_plotHandler;

	//--------GENERAL SETTINGS
	QComboBox *m_samplingFreqOptions;
	PositionSpinButton *m_timespanSpin;
	ExportSettings *m_exportSettings;
};
}
#endif // SWIOTAD_HPP
