#pragma once

#include <QList>
#include <QObject>
#include <QPointer>
#include <QString>

#include <core/acq_engine/DataKey.h>
#include <core/decoder/IDecoderBackend.h>

namespace scopy {

class PlotAxis;
class PlotWidget;

namespace decoder {
class IDecoderBackendFactory;
} // namespace decoder

namespace acq {
class AcquisitionEngine;
class DataStore;
class ExternalDecoderProcessor;
} // namespace acq

namespace adc {

class DecoderOverlay;

// Runtime handle for one active decoder. Owned by DecoderManager; the
// AcquisitionEngine owns the processor lifetime.
struct DecoderInstance
{
	QString                                uid;             // unique per manager: "<id>#<n>"
	QString                                decoderId;       // e.g. "uart"
	scopy::decoder::DecoderConfig          cfg;             // last applied
	QList<scopy::acq::DataKey>             orderedRawKeys;  // bitIndex → DataKey
	scopy::acq::ExternalDecoderProcessor  *proc{nullptr};
	scopy::acq::DataKey                    outKey;          // annotation output
	QPointer<scopy::PlotAxis>              axis;            // annotation band
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

	// Create a new decoder instance. Returns its uid or an empty string
	// on failure (e.g. plot/overlay not set yet).
	QString addDecoder(const QString &decoderId);

	// Detach + delete a decoder instance and its axis. Safe to call while
	// engine is running (the engine::removeProcessor handles queueing).
	void removeDecoder(const QString &uid);

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
	void bandAllocated(scopy::PlotAxis *axis);

private:
	scopy::PlotAxis *allocateBand(); // stack downward under waveform axis

	scopy::acq::AcquisitionEngine          *m_engine{nullptr};
	scopy::acq::DataStore                  *m_store{nullptr};
	scopy::decoder::IDecoderBackendFactory *m_backendFactory{nullptr};
	QPointer<scopy::PlotWidget>             m_plot;
	QPointer<DecoderOverlay>                m_overlay;

	QList<DecoderInstance>          m_decoders;

	// Monotonically increasing suffix so uids remain unique across
	// add/remove cycles.
	int                             m_uidCounter{0};

	// Vertical stacking cursor for annotation bands. Bands start at
	// -0.2 and grow downward in ~1.0-tall steps.
	double                          m_nextBandTop{-0.2};

	// Window size (samples) applied to every ExternalDecoderProcessor.
	// 0 = legacy single-chunk mode.
	int                             m_windowSize{0};
};

} // namespace adc
} // namespace scopy
