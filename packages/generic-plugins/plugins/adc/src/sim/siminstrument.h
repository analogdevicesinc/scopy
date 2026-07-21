#pragma once

#include <core/acq_engine/AcquisitionError.h>
#include <core/acq_engine/AcquisitionEngine.h>
#include <core/acq_engine/DataKey.h>
#include <core/acq_engine/DataStore.h>
#include <core/acq_engine/ProcessorBlock.h>

#include <QColor>
#include <QComboBox>
#include <QList>
#include <QPushButton>
#include <QTextEdit>
#include <QTreeWidget>
#include <QWidget>

#include <gui/tooltemplate.h>
#include <gui/plotwidget.h>
#include <gui/waterfallplotwidget.h>
#include <gui/widgets/toolbuttons.h>

namespace scopy {
namespace adc {

class SimInstrument : public QWidget
{
	friend class SimInstrumentController;
	Q_OBJECT
public:
	// Describes one plot curve and the processor blocks whose settings belong to it.
	// The controller fills in a list of these and passes it to buildControlPanel().
	struct CurveDescriptor
	{
		QString                    name;       // e.g. "Curve 1"
		QColor                     color;
		QList<scopy::acq::ProcessorBlock *> processors; // may be empty
	};

	explicit SimInstrument(QWidget *parent = nullptr);
	~SimInstrument();

	PlotWidget *plot() const { return m_plot; }
	WaterfallPlotWidget *waterfall() const { return m_waterfall; }

	// Called once after init() — builds the settings panel from engine + curve descriptors.
	// Must be called before the widget is shown.
	void buildControlPanel(scopy::acq::AcquisitionEngine *engine,
			       const QList<CurveDescriptor> &curves);

	// Returns the currently selected X/Y DataStore key for curve i (0-based).
	// Returns an empty string when "Sample Index" is selected.
	QString curveXKey(int i) const;
	QString curveYKey(int i) const;

	// Refresh all axis key combo-boxes from the current store key list.
	void updateCurveKeyCombos(const QList<scopy::acq::DataKey> &keys);

	// Refresh the DataStore inspector panel with a live snapshot of the store.
	// Safe to call from the main thread at any time (e.g. on every cycleComplete).
	void refreshDatastoreView(scopy::acq::DataStore *store);

	// Install the decoder panel widget in the right stack and wire the
	// "Decoders" toggle button into the mutual-exclusion group set up by
	// buildControlPanel(). Must be called after buildControlPanel().
	void registerDecoderPanel(QWidget *panel);

public Q_SLOTS:
	void onStarted();
	void onStopped();
	void onForceStopped();
	void appendLog(int severity, const QString &id, const QString &message);
	// Append a decoder-only log line (called from DecoderLogger::messageLogged).
	// `level` matches scopy::decoder::LogLevel (Info=0, Warning=1, Critical=2).
	void appendDecoderLog(int level, const QString &id, const QString &message);

Q_SIGNALS:
	void requestRun();
	void requestStop();
	void requestSingle();
	void sampleSizeChanged(int size);
	void plotSizeChanged(int size);
	void maxFpsChanged(int fps);
	void acqModeChanged(int index); // 0 = Continuous, 1 = Triggered
	void waterfallRowsChanged(int rows);

private:
	void setupUi();
	// Wire one panel toggle button so that activating it deactivates all
	// others currently in m_panelBtns. The set is consulted at click-time
	// (not at wire-time), so panels registered later still participate.
	void wirePanelButton(QPushButton *btn, const QString &menuId);

	// Per-curve selectors created by buildControlPanel()
	struct CurveSelectors
	{
		QComboBox *xKey{nullptr};
		QComboBox *yKey{nullptr};
	};

	ToolTemplate        *m_tool;
	PlotWidget          *m_plot;
	WaterfallPlotWidget *m_waterfall;
	RunBtn              *m_runBtn;
	SingleShotBtn *m_singleBtn;
	GearBtn       *m_settingsBtn;
	QPushButton   *m_cursorBtn;
	QPushButton   *m_logBtn;
	QPushButton   *m_datastoreBtn;
	QPushButton   *m_decoderBtn;
	QPushButton   *m_decoderLogBtn;
	QTextEdit     *m_logView;
	QTextEdit     *m_decoderLogView;
	QTreeWidget   *m_datastoreTable;

	// Populated by buildControlPanel()
	QList<CurveSelectors> m_curveSelectors;

	// Engine controls kept so onStarted/onStopped can enable/disable them
	QComboBox *m_modeCombo{nullptr};
	QWidget   *m_sampleSizeWidget{nullptr}; // the QSpinBox for buffer size
	QWidget   *m_plotSizeWidget{nullptr};   // the QSpinBox for plot size

	// Mutual-exclusion set for right-panel toggle buttons. Populated in
	// buildControlPanel() and mutated by registerDecoderPanel() so panels
	// added after the control panel is built still share the group.
	QList<QPushButton *> m_panelBtns;
};

} // namespace adc
} // namespace scopy
