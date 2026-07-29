#pragma once

#include <cfloat>

#include "siminstrument.h"
#include <core/acq_engine/AcquisitionEngine.h>
#include <core/acq_engine/DataStore.h>
#include <core/acq_engine/GenalyzerFFTProcessor.h>
#include <core/acq_engine/MathProcessor.h>
#include <core/acq_engine/MathSource.h>
#include "PlutoIIOSource.h"
#include "M2kLogicSource.h"
#include <core/acq_engine/ScaleOffsetProcessor.h>
#include <core/acq_engine/TriggerBinder.h>
#include <core/acq_engine/TriggerProcessor.h>
#include <core/acq_engine/TriggerProcessorWidget.h>
#include "SimulatedSource.h"

struct iio_context;
namespace libm2k { namespace digital { class M2kDigital; } }
namespace scopy { class GenalyzerPanel; }

#include <QElapsedTimer>
#include <QLabel>
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QPointer>
#include <QTimer>
#include <QVector>

#include <memory>

#include <gui/cursorcontroller.h>
#include <gui/plotautoscaler.h>
#include <gui/plotaxishandle.h>
#include <gui/plotchannel.h>
#include <pluginbase/toolmenuentry.h>

namespace scopy {

namespace decoder {
class IDecoderCatalog;
class IDecoderBackendFactory;
class DecoderLogger;
} // namespace decoder

namespace adc {

class DecoderOverlay;
class DecoderManager;
class DecoderPanel;
class DigitalTrackManager;

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
	// Optionally pass an M2kDigital* to also acquire M2K logic channels.
	void init(iio_context *ctx = nullptr,
		  libm2k::digital::M2kDigital *digital = nullptr);

	// Stop the engine if running.
	void stop();

	SimInstrument *ui() const;

private Q_SLOTS:
	// Called on the main thread after every acquisition cycle.
	// Reads selected axis keys from DataStore → updates the single PlotChannel → replots.
	void onCycleComplete();

	// Called (queued) when the TriggerProcessor fires. Reads the exact
	// fire-cycle chunk out of the snapshot payload (captured on the worker
	// thread before the next cycle) instead of the live store.
	void onTriggerFired(quint32 sampleIndex,
			    QMap<QString, scopy::acq::SampleVariant> snapshot);

private:
	void refreshPlotAxis();
	// Swap which signal drives the plot update path. Idempotent.
	void setTriggerReplotEnabled(bool en);

	// Reset all GUI-side data caches so a new run doesn't inherit any
	// pointer aliasing to the previous run's buffers. Also detaches Qwt
	// from any stale raw-sample pointer that might still be held.
	// Called from requestRun / requestSingle right after m_store->clear().
	void resetLiveBuffers();

	// Enable/disable a PlotChannel and add/remove it from the autoscalers
	// atomically. Idempotent — no-op if the driven state hasn't changed.
	void setCurveDriven(PlotChannel *ch, bool driven);

	ToolMenuEntry *m_tme;

	scopy::acq::DataStore              *m_store{nullptr};
	scopy::acq::AcquisitionEngine      *m_engine{nullptr};
	scopy::adc::sim::SimulatedSource        *m_src{nullptr};
	scopy::adc::sim::PlutoIIOSource         *m_plutoSrc{nullptr};
	scopy::adc::sim::M2kLogicSource         *m_logicSrc{nullptr};
	scopy::acq::ScaleOffsetProcessor   *m_scaleProc{nullptr};
	scopy::acq::GenalyzerFFTProcessor  *m_fftProc{nullptr};
	scopy::acq::MathSource             *m_mathSrc{nullptr};
	scopy::acq::MathProcessor          *m_mathProc{nullptr};
	scopy::acq::TriggerProcessor       *m_trigProc{nullptr};
	scopy::acq::TriggerProcessorWidget *m_trigWidget{nullptr};
	scopy::acq::TriggerBinder          *m_trigBinder{nullptr};
	scopy::adc::DecoderManager           *m_decoderMgr{nullptr};
	scopy::adc::DecoderPanel             *m_decoderPanel{nullptr};
	scopy::adc::DecoderOverlay           *m_decoderOverlay{nullptr};
	scopy::adc::DigitalTrackManager      *m_digitalMgr{nullptr};
	scopy::GenalyzerPanel                *m_genalyzerPanel{nullptr};

	// Composition-root-owned decoder plumbing. Swap the concrete
	// implementations here (e.g. sigrok-cli → libsigrok → …) without
	// touching DecoderManager/DecoderPanel. Non-QObject types, hence
	// unique_ptr rather than Qt parenting.
	std::unique_ptr<scopy::decoder::IDecoderCatalog>        m_decoderCatalog;
	std::unique_ptr<scopy::decoder::IDecoderBackendFactory> m_decoderBackendFactory;
	scopy::decoder::DecoderLogger                          *m_decoderLogger{nullptr};

	QPointer<SimInstrument> m_ui;

	// Two configurable plot channels (Y1 = cyan, Y2 = orange). Both share the same X key.
	PlotChannel *m_curve{nullptr};
	PlotChannel *m_curve2{nullptr};

	// Shared index buffer (0..plotSize-1) used when X, Y, or Y2 is set to "Sample Index".
	// Rebuilt whenever plot size changes.
	QVector<float> m_indexBuf;

	scopy::acq::SampleVariant m_liveX,  m_liveY,  m_liveX2,  m_liveY2;
	QVector<float>            m_scratchX, m_scratchY, m_scratchX2, m_scratchY2;

	// Fire-cycle snapshot delivered inside the trigger's fired() signal.
	// While non-empty, onCycleComplete reads windows from this map instead
	// of the live DataStore, so single-shot / trigger-replot modes plot
	// exactly the cycle that fired. Cleared after each fire is rendered.
	QMap<QString, scopy::acq::SampleVariant> m_firedSnapshot;

	int m_plotSize{1024};

	scopy::gui::PlotAutoscaler *m_autoscalerY{nullptr};
	scopy::gui::PlotAutoscaler *m_autoscalerX{nullptr};

	scopy::CursorController *m_cursorCtrl{nullptr};

	QLabel        *m_fpsLabel{nullptr};
	QElapsedTimer  m_fpsTimer;
	int            m_cycleCount{0};

	QTimer *m_displayTimer{nullptr};
	bool    m_dataDirty{false};

	// When trigger-driven replot is active we save the engine's original
	// maxFPS + Mode so we can restore them on disable. During trigger-replot
	// the engine's maxFPS is forced to 0 (loop free-runs) and Mode is forced
	// to Continuous (no per-cycle sleep on the Triggered branch, no need to
	// gate cycleComplete emission).
	bool                                m_triggerReplotEnabled{false};
	unsigned int                        m_savedMaxFPS{0};
	scopy::acq::AcquisitionEngine::Mode m_savedMode{
		scopy::acq::AcquisitionEngine::Mode::Continuous};
	QMetaObject::Connection m_cycleConn;

	// Trigger sample-position indicator. Draggable when the trigger is in
	// sample-specific mode; read-only/hidden otherwise. Position is in the
	// plot's current X-axis unit (index / time / freq); the controller
	// maps it back to a chunk sample index for the processor via the
	// latest X-array snapshot of the selected X key.
	scopy::PlotAxisHandle *m_triggerHandle{nullptr};
	// Latest X-axis snapshot used for coord mapping: matches the current
	// X-key selection. When empty, sample-index semantics apply.
	QVector<float> m_lastPlotX;

	// Scan-mode X-axis anchoring. The handle's *fractional* position on the
	// canvas (0 = left edge, 1 = right edge) is the single source of truth
	// for where the fired sample must appear. On every fire and on every
	// user drag we recompute the X-interval so that
	//     axisPosForSample(m_lastFiredSample) == xMin + m_handleFraction * W
	// where W = xMax - xMin (owned by the autoscaler / X-key). External
	// axis changes (autoscaler, refreshPlotAxis) trigger axisScaleUpdated
	// which we hook to re-assert the invariant.
	double  m_handleFraction{0.5};
	quint32 m_lastFiredSample{0};
	bool    m_haveLastFired{false};

	// Recursion guard: align() calls setInterval() which fires
	// axisScaleUpdated → onAxisScaleUpdated() which would call align()
	// again. Set while we're the ones changing the interval.
	bool m_aligning{false};

	// True when the trigger processor is enabled AND in scan mode
	// (non-sample-specific). Used to gate axis-anchoring behaviour.
	bool scanActive() const;

	// Map a chunk sample index → current X-axis position, using the last
	// captured X-key snapshot (m_lastPlotX). Falls back to identity when
	// the snapshot is empty or the index is out of range.
	double axisPosForSample(quint32 s) const;

	// Single source of truth for scan-mode alignment. Shifts the X
	// interval so the fired sample lands at m_handleFraction of the
	// canvas, then reprojects the handle onto that axis-value. No-op
	// unless scanActive() && m_haveLastFired.
	void align();

	// Handler for xAxis::axisScaleUpdated. When something *else* (the
	// autoscaler, refreshPlotAxis, user pan/zoom) changed the interval,
	// re-align so the fired sample stays under the handle.
	void onAxisScaleUpdated();

	// Waterfall: currently active data key (set dynamically from curveYKey(2)).
	scopy::acq::DataKey m_fftWaterfallKey;
	int          m_currentWaterfallRows{200};
	static constexpr int WATERFALL_ROWS = 200;

	// TODO: temporary waterfall intensity autoscaling accumulators — should be reworked and removed.
	double m_wfAutoMin{DBL_MAX};
	double m_wfAutoMax{-DBL_MAX};
};

} // namespace adc
} // namespace scopy
