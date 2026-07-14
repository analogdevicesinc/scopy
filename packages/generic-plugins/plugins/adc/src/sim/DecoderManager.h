#pragma once

#include <QList>
#include <QObject>
#include <QPointer>
#include <QString>

#include <core/acq_engine/DataKey.h>
#include <core/decoder/IDecoderBackend.h>

namespace scopy {

class PlotAxis;
class PlotAxisHandle;
class PlotWidget;

namespace decoder {
class IDecoderBackendFactory;
class DecoderLogger;
} // namespace decoder

namespace acq {
class AcquisitionEngine;
class DataStore;
class ExternalDecoderProcessor;
} // namespace acq

namespace adc {

class DecoderOverlay;

// Runtime handle for one active decoder stack. Owned by DecoderManager;
// the AcquisitionEngine owns the processor lifetime. A stack has one entry
// per stage in cfg.stack (index 0 is the root that reads raw logic).
struct DecoderInstance
{
	QString                                uid;             // unique per manager: "<rootId>#<n>"
	QStringList                            stageIds;        // e.g. {"uart","modbus"}
	scopy::decoder::DecoderConfig          cfg;             // last applied
	QList<scopy::acq::DataKey>             orderedRawKeys;  // bitIndex → DataKey (root only)
	scopy::acq::ExternalDecoderProcessor  *proc{nullptr};
	QList<scopy::acq::DataKey>             outKeys;         // one per stage
	// One draggable handle per stage. The handle lives on the plot's
	// main y-axis and its scale-space position defines the top of the
	// annotation band; the AnnotationCurve reads this on each redraw.
	QList<QPointer<scopy::PlotAxisHandle>> handles;
};

// Owns the list of active decoders and creates/destroys their processors
// and annotation-band axes. The overlay and plot are set separately so the
// manager can be constructed before the UI exists.
//
// Threading: all public methods are main-thread only. Config mutation is
// gated to when the engine is not running.
class DecoderManager : public QObject
{
	Q_OBJECT
public:
	// The backend factory is injected (non-owning) so this manager stays
	// decoupled from any specific decoder implementation (sigrok-cli,
	// libsigrok, custom CLI, …). The factory must outlive the manager.
	DecoderManager(scopy::acq::AcquisitionEngine *engine,
	               scopy::acq::DataStore *store,
	               scopy::decoder::IDecoderBackendFactory *backendFactory,
	               QObject *parent = nullptr);
	~DecoderManager() override;

	void setPlot(scopy::PlotWidget *plot);
	void setOverlay(DecoderOverlay *overlay);
	void setLogger(scopy::decoder::DecoderLogger *lg) { m_logger = lg; }

	// Create a new decoder instance. Returns its uid or an empty string
	// on failure (e.g. plot/overlay not set yet).
	QString addDecoder(const QString &decoderId);

	// Detach + delete a decoder instance and its axis. Safe to call while
	// engine is running (the engine::removeProcessor handles queueing).
	void removeDecoder(const QString &uid);

	// Append a stacked stage to an existing decoder instance. Allocates
	// a new annotation band + output DataKey, extends proc->outputKeys,
	// and registers the new (outKey, axis) pair with the overlay.
	// Returns the new stage index (>=1) on success, -1 on failure. Only
	// valid when the engine is stopped.
	int pushStage(const QString &uid, const QString &decoderId);

	// Remove stages at index >= fromIndex (must be >= 1 — the root is
	// never dropped this way). Bands are hidden/detached and their output
	// keys removed from the overlay & store. Only valid when the engine
	// is stopped.
	void popStagesFrom(const QString &uid, int fromIndex);

	// Push a new config to an existing decoder. Only valid when the
	// engine is stopped; caller must check isEngineRunning() first.
	void applyConfig(const QString &uid,
	                 const scopy::decoder::DecoderConfig &cfg,
	                 const QList<scopy::acq::DataKey> &orderedRawKeys);

	// Propagates to every existing and future ExternalDecoderProcessor.
	void setDecoderWindowSize(int n);

	const QList<DecoderInstance> &decoders() const { return m_decoders; }
	DecoderInstance              *find(const QString &uid);
	bool                          isEngineRunning() const;

Q_SIGNALS:
	void decoderAdded(const QString &uid);
	void decoderRemoved(const QString &uid);
	void configApplied(const QString &uid);

private:
	// Compute the initial scale-space position (on the plot's main
	// y-axis) for the next band handle, stacking downward from the
	// previous handle position.
	double nextBandPos();

	// Create a draggable PlotAxisHandle on the plot's main y-axis at the
	// given initial scale-space position, and make it visible.
	scopy::PlotAxisHandle *attachHandle(double initialPos);

	scopy::acq::AcquisitionEngine          *m_engine{nullptr};
	scopy::acq::DataStore                  *m_store{nullptr};
	scopy::decoder::IDecoderBackendFactory *m_backendFactory{nullptr};
	QPointer<scopy::PlotWidget>             m_plot;
	QPointer<DecoderOverlay>                m_overlay;

	QList<DecoderInstance>          m_decoders;

	// Monotonically increasing suffix so uids remain unique across
	// add/remove cycles.
	int                             m_uidCounter{0};

	// Vertical stacking cursor (in main y-axis scale coordinates) for
	// annotation band handle positions. First handle sits just below the
	// current waveform y-range; each subsequent handle stacks downward.
	double                          m_nextBandPos{0.0};
	bool                            m_bandCursorInit{false};

	// Window size (samples) applied to every ExternalDecoderProcessor.
	// 0 = legacy single-chunk mode.
	int                             m_windowSize{0};

	scopy::decoder::DecoderLogger  *m_logger{nullptr};
};

} // namespace adc
} // namespace scopy
