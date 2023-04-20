#ifndef SWIOTAD_HPP
#define SWIOTAD_HPP

#include <gui/tool_view.hpp>
#include <qwidget.h>
#include "bufferlogic.hpp"
#include "src/runtime/readerthread.h"
#include "buffermenucontroller.hpp"
#include <gui/channel_manager.hpp>
#include <gui/osc_export_settings.h>
#include <QVector>
#include "bufferplothandler.hpp"
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
	explicit Ad74413r(iio_context *ctx = nullptr, QVector<QString> chnlsFunc = {},
			  QWidget *parent = nullptr);

	~Ad74413r();

	void initMonitorToolView();

	void initExportSettings(QWidget *parent);

	void verifyChnlsChanges();

	scopy::gui::GenericMenu *createSettingsMenu(QString title, QColor *color);
public Q_SLOTS:

	void onChannelWidgetEnabled(bool en);

	void onChannelWidgetSelected(bool checked);

	void onRunBtnPressed();

	void onReaderThreadFinished();

Q_SIGNALS:

	void plotChnlsChanges(std::vector<bool> m_enabledPlots);

	void exportBtnClicked(QMap<int, bool> exportConfig);

	void activateExportButton();

	void backBtnPressed();
private:
	void setupToolView();

	void createMonitorChannelMenu();

	void setupConnections();
	void connectChnlsWidgesToPlot(std::vector<ChannelWidget *> channelList);

	QPushButton* createBackBtn();

private:
	struct iio_device *m_iioDev;
	int m_enabledChnlsNo = 0;

	scopy::gui::ChannelManager *m_monitorChannelManager;
	scopy::gui::ToolView *m_toolView;
	QWidget *m_widget;
	QPushButton *m_backBtn;

	QVector<BufferMenuController *> m_controllers;
	QVector<QString> m_chnlsFunction;

	std::vector<bool> m_enabledChannels;
	std::vector<bool> m_enabledPlots;

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
#endif // SWIOTAD_HPP
