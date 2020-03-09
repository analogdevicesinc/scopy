#ifndef LOGIC_ANALYZER_H
#define LOGIC_ANALYZER_H

#include <QList>
#include <QQueue>

#include "tool.hpp"
#include "oscilloscope_plot.hpp"
#include "buffer_previewer.hpp"
#include "spinbox_a.hpp"

#include <libm2k/m2k.hpp>
#include <libm2k/contextbuilder.hpp>
#include <libm2k/digital/m2kdigital.hpp>

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

namespace logic {

class LogicAnalyzer : public Tool {
	Q_OBJECT
public:
	explicit LogicAnalyzer(struct iio_context *ctx, Filter *filt,
			 ToolMenuItem *toolMenuItem, QJSEngine *engine,
			 ToolLauncher *parent, bool offline_mode_ = 0);
	~LogicAnalyzer();

public:
	uint16_t * getData();

private Q_SLOTS:

	void on_btnChannelSettings_toggled(bool);
	void on_btnCursors_toggled(bool);
	void on_btnTrigger_toggled(bool);

	void on_cursorsBox_toggled(bool on);

	void on_btnSettings_clicked(bool checked);
	void on_btnGeneralSettings_toggled(bool);
	void rightMenuFinished(bool opened);

	void onTimeTriggerValueChanged(double value);

private:
	void setupUi();
	void connectSignalsAndSlots();
	void triggerRightMenuToggle(CustomPushButton *, bool checked);
	void toggleRightMenu(CustomPushButton *, bool checked);
	void settingsPanelUpdate(int id);
	void updateBufferPreviewer();
	void initBufferScrolling();

private:
	// TODO: consisten naming (m_ui, m_crUi)
	Ui::LogicAnalyzer *ui;
	Ui::CursorsSettings *cr_ui;

	QList<CustomPushButton *> m_menuOrder;
	QQueue<QPair<CustomPushButton *, bool>> m_menuButtonActions;

	CapturePlot m_plot;
	BufferPreviewer *m_bufferPreviewer;

	ScaleSpinButton *m_sampleRateButton;
	ScaleSpinButton *m_bufferSize;

	M2k *m_m2kContext;

	uint16_t *m_buffer;

	double m_horizOffset;
	double m_timeTriggerOffset;
	bool m_resetHorizAxisOffset;

};
} // namespace logic
} // namespace adiscope

#endif // LOGIC_ANALYZER_H
