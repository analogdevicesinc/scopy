#pragma once

#include "scopy-core_export.h"

#include "TriggerProcessor.h"

#include <QStringList>
#include <QWidget>

class QCheckBox;
class QComboBox;
class QLabel;
class QSpinBox;
class QVBoxLayout;

namespace scopy {
namespace acq {

// GUI panel for TriggerProcessor. Renders a condition list (one row per
// condition) plus a combine-rule combo and coincidence-window spinbox.
// Editing UI is intentionally minimal — conditions can be edited inline via a
// small child widget per row (kind/edge/level/threshold/key).
class SCOPY_CORE_EXPORT TriggerProcessorWidget : public QWidget
{
	Q_OBJECT
public:
	explicit TriggerProcessorWidget(TriggerProcessor *proc, QWidget *parent = nullptr);

public Q_SLOTS:
	// Update the list of DataStore keys shown in each condition row's key
	// combo. Preserves the current selection per row (adds it as a stale
	// entry if it is not in the new list so unknown keys don't silently
	// change under the user).
	void setAvailableKeys(const QStringList &keys);

	// Report acquisition running state. Semantics:
	//   • running=true  → arm: clear "fired-since-arm", label goes to
	//     "waiting" (if enabled) / "idle" (if disabled).
	//   • running=false → stop: if a fire happened during this run and
	//     the processor is still enabled, label becomes "triggered".
	//     Otherwise "waiting" (enabled) / "idle" (disabled).
	// The widget never shows "triggered" while the acquisition is
	// running — that state is exclusively post-stop-and-fired.
	void setAcquisitionRunning(bool running);

	// Configure the maximum valid sample index for the target-sample
	// spinbox — usually the plot/chunk size minus one. Preserves the
	// current selection when clamping.
	void setMaxTargetSample(int maxIdx);

private:
	void rebuildRows();
	// Sticky trigger state indicator: "idle" while disabled, "waiting"
	// after enable, "triggered" on first fire until next enable toggle.
	void setStatusIdle();
	void setStatusWaiting();
	void setStatusTriggered();

	TriggerProcessor *m_proc;
	QVBoxLayout      *m_rowsLay{nullptr};
	QComboBox        *m_opCombo{nullptr};
	QSpinBox         *m_windowSpin{nullptr};
	QLabel           *m_statusLabel{nullptr};
	QStringList       m_availableKeys;
	QList<QComboBox *> m_keyCombos; // parallel to condition rows, cleared on rebuild
	// True once TriggerProcessor::fired has arrived since the last
	// setAcquisitionRunning(true) / enable transition. Consumed by
	// setAcquisitionRunning(false) to decide "triggered" vs "waiting".
	bool              m_firedSinceArm{false};
	// Latest known acquisition running state. When false, we suppress
	// the live "triggered" label (only stop-time may set it).
	bool              m_acqRunning{false};
	QCheckBox        *m_sampleSpecificCb{nullptr};
	QSpinBox         *m_sampleSpin{nullptr};
	QSpinBox         *m_toleranceSpin{nullptr};
};

} // namespace acq
} // namespace scopy
