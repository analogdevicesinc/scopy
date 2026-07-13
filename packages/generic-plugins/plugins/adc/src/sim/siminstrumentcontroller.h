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
#include "SimulatedSource.h"

struct iio_context;
namespace libm2k { namespace digital { class M2kDigital; } }
namespace scopy { class GenalyzerPanel; }

#include <QElapsedTimer>
#include <QLabel>
#include <QObject>
#include <QPointer>
#include <QTimer>
#include <QVector>

#include <memory>

#include <gui/cursorcontroller.h>
#include <gui/plotautoscaler.h>
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

private:
	void refreshPlotAxis();

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
	scopy::adc::DecoderManager           *m_decoderMgr{nullptr};
	scopy::adc::DecoderPanel             *m_decoderPanel{nullptr};
	scopy::adc::DecoderOverlay           *m_decoderOverlay{nullptr};
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

	int m_plotSize{1024};

	// Tracks the last seen key set so onCycleComplete() can refresh combos only on change.
	QList<scopy::acq::DataKey> m_lastKeys;

	scopy::gui::PlotAutoscaler *m_autoscalerY{nullptr};
	scopy::gui::PlotAutoscaler *m_autoscalerX{nullptr};

	scopy::CursorController *m_cursorCtrl{nullptr};

	QLabel        *m_fpsLabel{nullptr};
	QElapsedTimer  m_fpsTimer;
	int            m_cycleCount{0};

	QTimer *m_displayTimer{nullptr};
	bool    m_dataDirty{false};

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
