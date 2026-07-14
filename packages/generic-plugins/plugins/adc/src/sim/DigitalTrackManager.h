#pragma once

#include <QColor>
#include <QList>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QVector>

#include <core/acq_engine/DataKey.h>

namespace scopy {

class PlotAxis;
class PlotAxisHandle;
class PlotWidget;

namespace acq {
class DataStore;
} // namespace acq

namespace adc {

class DigitalCurveItem;

// Owns the "digital" area of the mixed-signal plot: a dedicated non-
// autoscaled y-axis on the right side of the plot, plus a stack of digital
// items (raw 0/1 curves and decoder annotation bands). Each item has its own
// draggable PlotAxisHandle so the user can move items vertically at will.
// Items render at a fixed canvas-pixel height matching AnnotationCurve rows,
// independent of the y-axis scale.
//
// Threading: main-thread only. updateRawCurves() is called from
// SimInstrumentController::onCycleComplete which is already queued to GUI.
class DigitalTrackManager : public QObject
{
	Q_OBJECT
public:
	explicit DigitalTrackManager(scopy::acq::DataStore *store,
	                             QObject *parent = nullptr);
	~DigitalTrackManager() override;

	// Must be called once before adding items.
	void setPlot(scopy::PlotWidget *plot);

	// The dedicated digital y-axis. Callers building AnnotationCurve items
	// (e.g. DecoderOverlay) use this so annotation bands render in the
	// same isolated area as raw digital curves.
	scopy::PlotAxis *yAxis() const { return m_yAxis; }

	// Register a raw 0/1 DataStore key as a digital curve. Returns a
	// unique id used by removeItem(); empty on failure.
	QString addRawChannel(const scopy::acq::DataKey &key,
	                      const QString &label);

	// Allocate a handle at the next stacking slot for an externally-
	// managed item (e.g. DecoderOverlay creating an AnnotationCurve).
	// The manager tracks the handle so it can be laid out and removed
	// via unregisterAnnotationBand() with the same handle pointer.
	scopy::PlotAxisHandle *registerAnnotationBand(const QString &id);
	void unregisterAnnotationBand(scopy::PlotAxisHandle *handle);

	void removeItem(const QString &id);

	// Called each acquisition cycle. Reads current windows for every raw
	// item from the store and pushes them to the corresponding
	// DigitalCurveItem. `plotSize` is the x-axis extent in samples.
	void updateRawCurves(int plotSize);

public Q_SLOTS:
	// Show/hide the raw digital track registered for a given
	// (sourceId, channelName) pair. Matching is done by rebuilding the
	// raw DataKey and comparing to Item::srcKey — no direct coupling to
	// any SourceBlock type. No-op if no matching item exists.
	void setChannelVisible(const QString &sourceId,
	                       const QString &channelName, bool visible);

private:
	struct Item
	{
		enum class Kind { RawCurve, AnnotationBand };
		Kind                                  kind;
		QString                               id;
		QPointer<scopy::PlotAxisHandle>       handle;

		// RawCurve only:
		DigitalCurveItem                     *curve{nullptr};
		scopy::acq::DataKey                   srcKey;
	};

	// If `color` is valid, the handle's marker is tinted with it;
	// otherwise a neutral theme color is used (annotation-band handles).
	// `westSide=true` places the handle on the west edge (left, for raw
	// digital tracks); `false` places it on the east edge (right, for
	// decoder annotation bands).
	scopy::PlotAxisHandle *makeHandle(double initialPos,
	                                  const QColor &color = {},
	                                  bool westSide = true);
	double                 nextHandlePos();

	scopy::acq::DataStore              *m_store{nullptr};
	QPointer<scopy::PlotWidget>         m_plot;
	scopy::PlotAxis                    *m_yAxis{nullptr};

	QList<Item>                         m_items;
	int                                 m_uidCounter{0};

	// Rotating index into Style::getChannelColor for raw digital tracks.
	// Annotation-band handles use a neutral color and don't advance this.
	int                                 m_colorCursor{0};

	// Stacking cursor in y-axis scale coordinates. Walks downward in
	// steps of ~26 px (24 row + 2 gap) converted to scale space.
	double                              m_nextPosScale{0.0};
	bool                                m_cursorInit{false};
};

} // namespace adc
} // namespace scopy
