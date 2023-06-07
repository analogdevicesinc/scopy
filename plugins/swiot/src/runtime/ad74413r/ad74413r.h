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
#define SWIOT_DEVICE_NAME "swiot"

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
			  QWidget *parent = nullptr);

	~Ad74413r();

	void initMonitorToolView(gui::GenericMenu *settingsMenu);

	void initExportSettings(QWidget *parent);

	void verifyChnlsChanges();

	void createDevicesMap(iio_context *ctx);

	scopy::gui::GenericMenu *createSettingsMenu(QString title, QColor *color);

public Q_SLOTS:
	void onChannelWidgetEnabled(int chnWidgetId, bool en);
	void onChannelWidgetSelected(int chnWidgetId, bool en);
	void onOffsetHdlSelected(int hdlIdx, bool selected);

	void onRunBtnPressed();
	void onSingleBtnPressed();

	void onReaderThreadFinished();
	void onSingleCaptureFinished();

	void externalPowerSupply(bool ps);

Q_SIGNALS:

	void channelWidgetEnabled(int curveId, bool en);
	void channelWidgetSelected(int curveId);

	void exportBtnClicked(QMap<int, bool> exportConfig);

	void activateExportButton();

	void backBtnPressed();
private:
	void setupToolView(gui::GenericMenu *settingsMenu);

	void createMonitorChannelMenu();

	void setupConnections();

	QPushButton* createBackBtn();

private:
	QMap<QString, iio_device*> m_iioDevicesMap;
	int m_enabledChnlsNo = 0;

	scopy::gui::ChannelManager *m_monitorChannelManager;
	scopy::gui::ToolView *m_toolView;
	QWidget *m_widget;
	QPushButton *m_backBtn;
	QLabel* m_statusLabel;
	QWidget* m_statusContainer;
	ToolMenuEntry *m_tme;

	QVector<BufferMenuController *> m_controllers;

	std::vector<bool> m_enabledChannels;
	std::vector<ChannelWidget*> m_channelWidgetList;

	BufferLogic *m_swiotAdLogic;
	ReaderThread *m_readerThread;
	BufferPlotHandler *m_plotHandler;

	QComboBox *m_samplingFreqOptions;
	PositionSpinButton *m_timespanSpin;
	ExportSettings *m_exportSettings;
};
}
}
#endif // AD74413R_H
