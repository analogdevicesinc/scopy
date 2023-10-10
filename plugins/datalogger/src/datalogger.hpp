#ifndef DATALOGGER_H
#define DATALOGGER_H

#include <QElapsedTimer>
#include <QFileDialog>
#include <QJSEngine>
#include <QLineEdit>
#include <QPair>
#include <QRadioButton>
#include <QTimer>
#include <QWidget>

#include <flexgridlayout.hpp>
#include <gui/tool_view.hpp>
#include <spinbox_a.hpp>

/* libm2k includes */
#include "dataloggerreaderthread.hpp"

#include <libm2k/analog/dmm.hpp>
#include <libm2k/analog/m2kanalogin.hpp>
#include <libm2k/m2k.hpp>
#include <libm2k/m2kexceptions.hpp>

namespace libm2k::context {
class ContextBuilder;
}

namespace scopy {
namespace gui {
class GenericMenu;
class ChannelManager;
class ChannelWidget;
} // namespace gui

class CustomSwitch;

namespace datalogger {
namespace gui {
class ChannelMonitorComponent;
class DataLoggerGenericMenu;
} // namespace gui

class DataLogger_API;
class DataLoggerController;

class DataLogger : public QWidget
{
	friend class DataLogger_API;

	Q_OBJECT

public:
	explicit DataLogger(libm2k::context::Context *ctx, QWidget *parent = nullptr);
	~DataLogger();

	scopy::gui::ToolView *getToolView();

	CustomSwitch *showAllSWitch;
	int getPrecision();
	void setPrecision(int precision);
	int getValueReadingTimeInterval();
	void setNativeDialogs(bool nativeDialogs);
	QPushButton *getRunButton();

private:
	int VALUE_READING_TIME_INTERVAL = 1000;

	QWidget *parent;
	QTimer *m_timer;
	QElapsedTimer *m_elapsed;

	QLineEdit *precisionValue;
	PositionSpinButton *recording_timer;

	QList<QColor> m_colors;
	QMap<int, QColor> m_color;
	DataLoggerController *dataLoggerController;
	scopy::gui::ToolView *m_toolView;
	QScrollArea *m_scrollArea;
	FlexGridLayout *m_flexGridLayout;
	libm2k::context::Context *m_context;
	std::vector<libm2k::analog::DMM *> m_dmmList;
	scopy::gui::GenericMenu *m_generalSettingsMenu;
	scopy::gui::ChannelManager *m_monitorChannelManager;
	QMap<int, datalogger::gui::ChannelMonitorComponent *> m_activeChannels;
	DataLoggerReaderThread *readerThread;
	QColor generateColor();
	void initMonitorToolView();
	QColor getChannelColor(int chId);
	scopy::gui::GenericMenu *generateMenu(QString title, QColor *color);
	std::vector<libm2k::analog::DMM *> getDmmList(libm2k::context::Context *m2k_context);
	void createConnections(gui::DataLoggerGenericMenu *mainMenu, gui::DataLoggerGenericMenu *menu,
			       gui::ChannelMonitorComponent *monitor);

Q_SIGNALS:
	void precisionChanged(int precision);
	void channelValueUpdated(int chId, double value, QString nameOfUnitOfMeasure, QString symbolOfUnitOfMeasure);
	void recordingIntervalChanged(double recordingInterval);
	void toggleAll(bool showAll);
	void disableActivateChannel(bool disable);
};
} // namespace datalogger
} // namespace scopy
#endif // DATALOGGER_H
