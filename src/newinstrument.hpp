#ifndef NEWINSTRUMENT_HPP
#define NEWINSTRUMENT_HPP

#include <filter.hpp>
#include <tool_launcher.hpp>
#include "ui_newinstrument.h"
#include <tool.hpp>
#include <libm2k/m2k.hpp>
#include <libm2k/contextbuilder.hpp>

namespace Ui {
	//class CursorReadouts;
	class CursorsSettings;
}


namespace adiscope {
class NewInstrument : public Tool
{
	Q_OBJECT
	Ui::NewInstrument *m_ui;
	CapturePlot m_plot;
	unsigned int m_channels;
	libm2k::context::M2k *m_m2k;
	libm2k::analog::M2kAnalogIn* m_m2k_analogin;
	std::atomic_bool m_running;
	std::thread m_workerThread;
	MouseWheelWidgetGuard *wheelEventGuard;
	int zoom_level;
	void startStop(bool start);
	void generateData();

	bool hCursorsEnabled;
	bool vCursorsEnabled;
	Ui::CursorsSettings *cr_ui;
	CustomPlotPositionButton *cursorsPositionButton;
	void cursor_panel_init();

	//adiscope::TriggerSettings trigger_settings;
	QList<CustomPushButton *> menuOrder;
	QQueue<QPair<CustomPushButton *, bool>> menuButtonActions;
	void triggerRightMenuToggle(CustomPushButton *, bool checked);
	void toggleRightMenu(CustomPushButton *, bool checked);

Q_SIGNALS:
	void selectedChannelChanged(int);

private Q_SLOTS:
	void on_boxCursors_toggled(bool on);
	void on_btnCursors_toggled(bool);
	//void toggleCursorsMode(bool toggled);


public:
	NewInstrument(struct iio_context *ctx, Filter *filt,
		      ToolMenuItem *toolMenuItem,
		      QJSEngine *engine, ToolLauncher *parent);
	~NewInstrument();
	void initAcquisition();
	void start();
	void stop();

};
}
#endif // NEWINSTRUMENT_HPP
