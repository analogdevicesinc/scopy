#pragma once

#include <cfloat>

#include "siminstrument.h"
#include "AcquisitionEngine.h"
#include "DataStore.h"
#include "GenalyzerFFTProcessor.h"
#include "MathProcessor.h"
#include "MathSource.h"
#include "PlutoIIOSource.h"
#include "ScaleOffsetProcessor.h"
#include "SimulatedSource.h"

struct iio_context;

#include <QElapsedTimer>
#include <QLabel>
#include <QObject>
#include <QPointer>
#include <QTimer>
#include <QVector>

#include <gui/cursorcontroller.h>
#include <gui/plotautoscaler.h>
#include <gui/plotchannel.h>
#include <pluginbase/toolmenuentry.h>

namespace scopy {
namespace adc {

// Self-contained controller for the simulated ADC tool.
// Does NOT depend on GRTopBlock, AcqTreeNode, or any GNU Radio component.
class SimInstrumentController : public QObject
{
	Q_OBJECT
public:
	explicit SimInstrumentController(ToolMenuEntry *tme, QObject *parent = nullptr);
	~SimInstrumentController();

	// Build the engine, sources, UI and connect all signals. Call once.
	// Pass a live iio_context* to use PlutoIIOSource (real hardware);
	// pass nullptr (default) to use SimulatedSource (offline/testing).
	void init(iio_context *ctx = nullptr);

	// Stop the engine if running.
	void stop();

	SimInstrument *ui() const;

private Q_SLOTS:
	// Called on the main thread after every acquisition cycle.
	// Reads selected axis keys from DataStore → updates the single PlotChannel → replots.
	void onCycleComplete();

private:
	ToolMenuEntry *m_tme;

	sim::DataStore              *m_store{nullptr};
	sim::AcquisitionEngine      *m_engine{nullptr};
	sim::SimulatedSource        *m_src{nullptr};
	sim::PlutoIIOSource         *m_plutoSrc{nullptr};
	sim::ScaleOffsetProcessor   *m_scaleProc{nullptr};
	sim::GenalyzerFFTProcessor  *m_fftProc{nullptr};
	sim::MathSource             *m_mathSrc{nullptr};
	sim::MathProcessor          *m_mathProc{nullptr};

	QPointer<SimInstrument> m_ui;

	// Two configurable plot channels (Y1 = cyan, Y2 = orange). Both share the same X key.
	PlotChannel *m_curve{nullptr};
	PlotChannel *m_curve2{nullptr};

	// Shared index buffer (0..n-1) used when X, Y, or Y2 is set to "Sample Index".
	// Rebuilt whenever the acquisition buffer size changes.
	QVector<float> m_indexBuf;

	// Tracks the last seen key set so onCycleComplete() can refresh combos only on change.
	QList<sim::DataKey> m_lastKeys;

	scopy::gui::PlotAutoscaler *m_autoscalerY{nullptr};
	scopy::gui::PlotAutoscaler *m_autoscalerX{nullptr};

	scopy::CursorController *m_cursorCtrl{nullptr};

	QLabel        *m_fpsLabel{nullptr};
	QElapsedTimer  m_fpsTimer;
	int            m_cycleCount{0};

	QTimer *m_displayTimer{nullptr};
	bool    m_dataDirty{false};

	// Waterfall: currently active data key (set dynamically from curveYKey(2)).
	sim::DataKey m_fftWaterfallKey;
	int          m_currentWaterfallRows{200};
	static constexpr int WATERFALL_ROWS = 200;

	// TODO: temporary waterfall intensity autoscaling accumulators — should be reworked and removed.
	double m_wfAutoMin{DBL_MAX};
	double m_wfAutoMax{-DBL_MAX};
};

} // namespace adc
} // namespace scopy
