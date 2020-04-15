#ifndef LOGIC_ANALYZER_H
#define LOGIC_ANALYZER_H

#include <thread>
#include <mutex>
#include <condition_variable>

#include <QList>
#include <QQueue>
#include <QScrollBar>
#include <QTimer>

#include "tool.hpp"
#include "oscilloscope_plot.hpp"
#include "buffer_previewer.hpp"
#include "spinbox_a.hpp"
#include "scroll_filter.hpp"

#include "genericlogicplotcurve.h"

#include <libm2k/m2k.hpp>
#include <libm2k/contextbuilder.hpp>
#include <libm2k/digital/m2kdigital.hpp>
#include <libm2k/enums.hpp>

// TODO
using namespace libm2k;
using namespace libm2k::digital;
using namespace libm2k::contexts;

namespace Ui {
class LogicAnalyzer;
class CursorsSettings;
} // namespace Ui

namespace adiscope {

class Filter;
class BaseMenu;
class LogicAnalyzer_API;

namespace logic {

class LogicAnalyzer : public Tool {
	Q_OBJECT

	friend class LogicAnalyzer_API;
public:
	explicit LogicAnalyzer(struct iio_context *ctx, Filter *filt,
			 ToolMenuItem *toolMenuItem, QJSEngine *engine,
			 ToolLauncher *parent, bool offline_mode_ = 0);
	~LogicAnalyzer();

public:
	uint16_t * getData();

Q_SIGNALS:
	void dataAvailable(uint64_t, uint64_t);

private Q_SLOTS:

	void on_btnChannelSettings_toggled(bool);
	void on_btnCursors_toggled(bool);
	void on_btnTrigger_toggled(bool);

	void on_cursorsBox_toggled(bool on);

	void on_btnSettings_clicked(bool checked);
	void on_btnGeneralSettings_toggled(bool);
	void rightMenuFinished(bool opened);

	void onTimeTriggerValueChanged(double value);

	void onSampleRateValueChanged(double value);
	void onBufferSizeChanged(double value);

	void on_btnStreamOneShot_toggled(bool toggled);

	void on_btnGroupChannels_toggled(bool checked);
	void channelSelectedChanged(int chIdx, bool selected);

	void saveTriggerState();
	void restoreTriggerState();

private:
	void setupUi();
	void connectSignalsAndSlots();
	void triggerRightMenuToggle(CustomPushButton *, bool checked);
	void toggleRightMenu(CustomPushButton *, bool checked);
	void settingsPanelUpdate(int id);
	void updateBufferPreviewer();
	void initBufferScrolling();

	void startStop(bool start);

	void setupDecoders();
	void updateStackDecoderButton();
	void updateChannelGroupWidget(bool visible);

	void setupTriggerMenu();

private:
	// TODO: consisten naming (m_ui, m_crUi)
	Ui::LogicAnalyzer *ui;
	Ui::CursorsSettings *cr_ui;

	QList<CustomPushButton *> m_menuOrder;
	QQueue<QPair<CustomPushButton *, bool>> m_menuButtonActions;

	CapturePlot m_plot;
	BufferPreviewer *m_bufferPreviewer;
	QScrollBar *m_plotScrollBar;

	ScaleSpinButton *m_sampleRateButton;
	ScaleSpinButton *m_bufferSizeButton;
	ScaleSpinButton *m_timePositionButton;
	double m_sampleRate;
	uint64_t m_bufferSize;
	uint64_t m_lastCapturedSample;

	M2k *m_m2kContext;
	M2kDigital *m_m2kDigital;
	int m_nbChannels;

	uint16_t *m_buffer;
	QVector<GenericLogicPlotCurve *> m_plotCurves;

	double m_horizOffset;
	double m_timeTriggerOffset;
	bool m_resetHorizAxisOffset;

	// capture
	std::thread *m_captureThread;
	std::atomic<bool> m_stopRequested;
	// prob not needed
	std::mutex m_captureMutex;
	std::condition_variable m_captureCv;

	bool m_started;

	int m_selectedChannel;

	MouseWheelWidgetGuard *m_wheelEventGuard;
	QWidget *m_decoderMenu;

	QVector<int> m_currentGroup;
	BaseMenu *m_currentGroupMenu;

	bool m_autoMode;
	QTimer *m_timer;
	double m_timerTimeout;
	QVector<M2K_TRIGGER_CONDITION_DIGITAL> m_triggerState;

};
} // namespace logic
} // namespace adiscope

#endif // LOGIC_ANALYZER_H
