#ifndef MAX14906_H
#define MAX14906_H

#include "diocontroller.h"
#include "diodigitalchannelcontroller.h"
#include "diosettingstab.h"
#include "scopy-swiotrefactor_export.h"
#include <QPushButton>
#include <QWidget>
#include <readerthread.h>
#include <gui/tooltemplate.h>
#include <gui/widgets/toolbuttons.h>
#include <iioutil/commandqueue.h>
#include <iioutil/connection.h>
#include <pluginbase/toolmenuentry.h>

#define MAX_NAME "max14906"
#define MAX14906_POLLING_TIME 1000

namespace scopy::swiotrefactor {
class SCOPY_SWIOTREFACTOR_EXPORT Max14906 : public QWidget
{
	Q_OBJECT
public:
	Max14906(QString uri, ToolMenuEntry *tme, QWidget *parent = nullptr);
	~Max14906();

Q_SIGNALS:
	void configBtnPressed();

public Q_SLOTS:
	void handleConnectionDestroyed();

private Q_SLOTS:
	void runButtonToggled();
	void timerChanged(double value);
	void onConfigBtnPressed();

private:
	ToolTemplate *m_tool;
	QWidget *m_gridWidget;
	QPushButton *m_configBtn;
	RunBtn *m_runBtn;
	GearBtn *m_gearBtn;

	int m_nbDioChannels;
	DioController *max14906ToolController;
	DioSettingsTab *m_max14906SettingsTab;

	QTimer *m_qTimer;

	CommandQueue *m_cmdQueue;
	ReaderThread *m_readerThread;
	Connection *m_conn;
	struct iio_context *m_ctx;
	QString m_uri;
	QMap<int, DioDigitalChannelController *> m_channelControls;

	ToolMenuEntry *m_tme;

	void initChannels();
	void setupDynamicUi(QWidget *parent);
	void initMonitorToolView();
	void connectSignalsAndSlots();

	static QFrame *createVLine(QWidget *parent);
	static QFrame *createHLine(QWidget *parent);
	static QMainWindow *createDockableMainWindow(const QString &title, DioDigitalChannel *digitalChannel,
						     QWidget *parent);
	QPushButton *createConfigBtn(QWidget *parent = nullptr);
};
} // namespace scopy::swiotrefactor
#endif // MAX14906_H
