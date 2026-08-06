#pragma once

#include "scopy-core_export.h"

#include <QWidget>

class QComboBox;
class QLabel;
class QLineEdit;
class QSpinBox;

namespace scopy {

class MenuOnOffSwitch;

namespace acq {

class GenalyzerFFTProcessor;

// GUI panel for GenalyzerFFTProcessor's transform parameters — the FFT itself,
// as opposed to the Fourier analysis that GenalyzerSettings covers.
//
// Window, real-mode dBFS reference and sample rate are editable. NFFT is not: it
// is derived from the input length divided by the averaging count, so the block
// reports it and this widget only displays it. Showing it read-only rather than
// hiding it matters because averaging trades it away — turning on 4x averaging
// over a fixed buffer quarters the bin count, and that should be visible where
// the averaging control is.
class SCOPY_CORE_EXPORT GenalyzerTransformSettings : public QWidget
{
	Q_OBJECT
public:
	explicit GenalyzerTransformSettings(GenalyzerFFTProcessor *proc, QWidget *parent = nullptr);

private Q_SLOTS:
	// Pull every value back from the block. Also the slot for its change
	// signals, so a sample rate set from elsewhere shows up here.
	void syncFromProcessor();

private:
	void setupUI();
	void updateDerived();

	GenalyzerFFTProcessor *m_proc{nullptr};

	QComboBox       *m_windowCombo{nullptr};
	QComboBox       *m_scaleCombo{nullptr};
	QWidget         *m_scaleContainer{nullptr};
	QLineEdit       *m_sampleRateEdit{nullptr};
	MenuOnOffSwitch *m_avgSwitch{nullptr};
	QSpinBox        *m_avgSpinbox{nullptr};
	QWidget         *m_avgSizeContainer{nullptr};
	QLabel          *m_nfftValue{nullptr};
	QLabel          *m_rbwValue{nullptr};
};

} // namespace acq
} // namespace scopy
