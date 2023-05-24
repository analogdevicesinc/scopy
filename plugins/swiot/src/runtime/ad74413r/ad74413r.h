#ifndef AD74413R_H
#define AD74413R_H

#include <gui/tool_view.hpp>
#include <qwidget.h>
#include "bufferlogic.h"
#include "pluginbase/toolmenuentry.h"
#include "src/runtime/readerthread.h"
#include "buffermenucontroller.h"
#include <gui/channel_manager.hpp>
#include <gui/osc_export_settings.h>
#include <QVector>
#include "bufferplothandler.h"
#include <iio.h>

#define MAX_CURVES_NUMBER 8
#define AD_NAME "ad74413r"

extern "C"{
struct iio_device;
struct iio_channel;
struct iio_buffer;
}

namespace scopy {
namespace gui {
class GenericMenu;
}

namespace swiot {
class Ad74413r : public QWidget {
	Q_OBJECT
public:
	explicit Ad74413r(iio_context *ctx = nullptr, ToolMenuEntry *tme = 0,
			  QVector<QString> chnlsFunc = {}, QWidget *parent = nullptr);

	~Ad74413r();

	void initMonitorToolView();

	void initExportSettings(QWidget *parent);

	void verifyChnlsChanges();

	scopy::gui::GenericMenu *createSettingsMenu(QString title, QColor *color);
public Q_SLOTS:

	void onChannelWidgetEnabled(bool en);
	void onChannelWidgetSelected(bool checked);
	void onOffsetHdlSelected(int hdlIdx, bool selected);

	void onRunBtnPressed();
	void onSingleBtnPressed();

	void onReaderThreadFinished();

	void onSingleCaptureFinished();

Q_SIGNALS:

	void channelWidgetEnabled(int curveId, std::vector<bool> enabledPlots);

	void exportBtnClicked(QMap<int, bool> exportConfig);

	void activateExportButton();

	void backBtnPressed();
private:
	void setupToolView();

	void createMonitorChannelMenu();

	void setupConnections();
	void connectChnlsWidgesToPlot();

	QPushButton* createBackBtn();

private:
	struct iio_device *m_iioDev;
	int m_enabledChnlsNo = 0;

	scopy::gui::ChannelManager *m_monitorChannelManager;
	scopy::gui::ToolView *m_toolView;
	QWidget *m_widget;
	QPushButton *m_backBtn;
	ToolMenuEntry *m_tme;

	QVector<BufferMenuController *> m_controllers;
	QVector<QString> m_chnlsFunction;

	std::vector<bool> m_enabledChannels;
	std::vector<bool> m_enabledPlots;
	std::vector<ChannelWidget*> m_channelWidgetList;

	BufferLogic *m_swiotAdLogic;
	ReaderThread *m_readerThread;
	BufferPlotHandler *m_plotHandler;

	//--------GENERAL SETTINGS
	QComboBox *m_samplingFreqOptions;
	PositionSpinButton *m_timespanSpin;
	ExportSettings *m_exportSettings;
};
}
}
#endif // AD74413R_H
