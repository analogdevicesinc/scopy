#include "DigitalTrackManager.h"

#include "DigitalCurveItem.h"

#include <core/acq_engine/DataStore.h>
#include <core/acq_engine/SampleBuffer.h>

#include <gui/plotaxis.h>
#include <gui/plotaxishandle.h>
#include <gui/plotwidget.h>
#include <gui/style.h>
#include <gui/style_attributes.h>

#include <axishandle.h>
#include <qwt_axis.h>
#include <qwt_plot.h>
#include <qwt_plot_canvas.h>

#include <QPen>

#include <variant>

namespace scopy {
namespace adc {

namespace {
// Vertical stacking pitch in canvas pixels: 24 px row + 2 px gap. Kept in
// sync with DigitalCurveItem/AnnotationCurve visuals.
constexpr double kSlotPitchPx = 26.0;
} // namespace

DigitalTrackManager::DigitalTrackManager(scopy::acq::DataStore *store,
                                         QObject *parent)
	: QObject(parent)
	, m_store(store)
{}

DigitalTrackManager::~DigitalTrackManager()
{
	// If the PlotWidget has already been destroyed (common during app
	// shutdown, since m_ui/m_plot isn't parented to us), the QwtPlot is
	// gone too: Qwt auto-detaches items and the canvas has already
	// destroyed its child AxisHandle widgets. Touching them here would be
	// a use-after-free (crash: QwtPlotDict::removeItem via detach()).
	if(m_plot.isNull()) {
		m_items.clear();
		return;
	}

	// Normal teardown while the plot is still alive.
	for(Item &it : m_items) {
		if(it.curve) {
			it.curve->detach();
			delete it.curve;
			it.curve = nullptr;
		}
		if(!it.handle.isNull()) {
			m_plot->removePlotAxisHandle(it.handle);
			it.handle->deleteLater();
		}
	}
	m_items.clear();
}

void DigitalTrackManager::setPlot(scopy::PlotWidget *plot)
{
	if(m_plot == plot) return;
	m_plot = plot;
	if(!m_plot) return;

	// Create the digital y-axis on the LEFT side (as a stacked additional
	// YLeft axis) so handles render on the left of the canvas next to the
	// main analog y-axis. Fixed interval [0,1]; ticks/labels are hidden —
	// items self-place using pixel offsets from their handle, so the axis
	// exists only to host handles/items.
	QPen pen(Style::getColor(json::theme::content_silent));
	m_yAxis = new scopy::PlotAxis(QwtAxis::YLeft, m_plot.data(), pen, this);
	m_yAxis->setInterval(0.0, 1.0);
	m_yAxis->setVisible(false);
}

scopy::PlotAxisHandle *DigitalTrackManager::makeHandle(double initialPos,
                                                       const QColor &color,
                                                       bool westSide)
{
	if(!m_plot || !m_yAxis) return nullptr;

	auto *h = new scopy::PlotAxisHandle(m_plot.data(), m_yAxis);
	h->handle()->setBarVisibility(scopy::BarVisibility::ON_HOVER);
	const QColor handleColor = color.isValid()
		? color
		: Style::getColor(json::theme::content_silent);
	h->handle()->setColor(handleColor);
	// For a YLeft axis: NORTH_OR_WEST => left edge (raw digital tracks),
	// SOUTH_OR_EAST => right edge (decoder annotation bands).
	h->handle()->setHandlePos(westSide
		? scopy::HandlePos::NORTH_OR_WEST
		: scopy::HandlePos::SOUTH_OR_EAST);

	m_plot->addPlotAxisHandle(h);

	// AxisHandle is created parented to the canvas (see gui/src/axishandle.cpp
	// line 32) and installs an event filter on the canvas so it auto-resizes
	// via setFixedSize on canvas resize. We must NOT re-parent, resize, or
	// raise the handle here — doing so breaks that mechanism (handles then
	// get clipped/hidden on horizontal plot resize). We only need to show
	// it explicitly, mirroring PlotCursors which calls setVisible(true) on
	// its handles (gui/src/plotcursors.cpp).
	if(auto *ah = h->handle())
		ah->setVisible(true);
	h->setPosition(initialPos);

	// Any handle drag re-triggers a replot so items follow the new pos.
	connect(h, &scopy::PlotAxisHandle::scalePosChanged, this, [this](double) {
		if(m_plot) m_plot->plot()->replot();
	});
	return h;
}

double DigitalTrackManager::nextHandlePos()
{
	if(!m_plot || !m_yAxis) return 1.0;

	if(!m_cursorInit) {
		// Start a bit below the top so there's headroom above the
		// first item (both raw DIO curves and decoder annotation
		// bands share this same starting point).
		const double span = m_yAxis->max() - m_yAxis->min();
		m_nextPosScale = m_yAxis->max() - 0.10 * span;
		m_cursorInit   = true;
	}

	const double pos = m_nextPosScale;

	// Convert pitch pixels to scale delta on this axis. We use a
	// temporary handle to leverage its pixelToScale helper (which knows
	// the axis' current pixel size). If no handle exists yet the plot
	// may not have laid out, in which case fall back to a small nominal
	// scale delta.
	double delta = 0.02; // fallback: ~2% of [0,1]
	if(!m_items.isEmpty() && !m_items.first().handle.isNull()) {
		auto *h = m_items.first().handle.data();
		const double atTop  = h->pixelToScale(0);
		const double atNext = h->pixelToScale(static_cast<int>(kSlotPitchPx));
		delta = std::abs(atTop - atNext);
	} else {
		// First allocation: try to derive from y-axis pixel span
		// once the plot has a canvas.
		if(m_plot && m_plot->plot() && m_plot->plot()->canvas()) {
			const int h = m_plot->plot()->canvas()->height();
			if(h > 0)
				delta = kSlotPitchPx / static_cast<double>(h);
		}
	}
	m_nextPosScale = pos - delta;
	return pos;
}

QString DigitalTrackManager::addRawChannel(const scopy::acq::DataKey &key,
                                           const QString &label)
{
	if(!m_plot || !m_yAxis) return {};

	const QString id = QStringLiteral("raw:%1#%2").arg(key.toString()).arg(m_uidCounter++);

	// Pick a distinct color per raw digital track from the shared theme
	// palette (wraps around if there are more channels than colors).
	const QList<QColor> palette = Style::getChannelColorList();
	const int paletteLen = palette.isEmpty() ? 1 : palette.length();
	const QColor color = Style::getChannelColor(m_colorCursor++ % paletteLen);

	auto *handle = makeHandle(nextHandlePos(), color, /*westSide=*/true);
	if(!handle) return {};

	auto *item = new DigitalCurveItem(label, m_plot->xAxis(), m_yAxis, handle);
	item->setColor(color);
	item->attach(m_plot->plot());

	Item entry;
	entry.kind   = Item::Kind::RawCurve;
	entry.id     = id;
	entry.handle = handle;
	entry.curve  = item;
	entry.srcKey = key;
	m_items.append(entry);
	return id;
}

scopy::PlotAxisHandle *
DigitalTrackManager::registerAnnotationBand(const QString &id)
{
	if(!m_plot || !m_yAxis) return nullptr;

	// Neutral color, west-side handle so decoder annotation handles sit
	// on the left edge together with the raw digital tracks' handles.
	auto *handle = makeHandle(nextHandlePos(), /*color=*/{},
	                          /*westSide=*/true);
	if(!handle) return nullptr;

	Item entry;
	entry.kind   = Item::Kind::AnnotationBand;
	entry.id     = id.isEmpty()
		? QStringLiteral("ann:#%1").arg(m_uidCounter++)
		: id;
	entry.handle = handle;
	m_items.append(entry);
	return handle;
}

void DigitalTrackManager::unregisterAnnotationBand(scopy::PlotAxisHandle *handle)
{
	if(!handle) return;
	for(int i = 0; i < m_items.size(); ++i) {
		if(m_items[i].handle.data() != handle) continue;
		if(m_plot) m_plot->removePlotAxisHandle(handle);
		handle->deleteLater();
		m_items.removeAt(i);
		return;
	}
}

void DigitalTrackManager::removeItem(const QString &id)
{
	for(int i = 0; i < m_items.size(); ++i) {
		if(m_items[i].id != id) continue;
		Item it = m_items.takeAt(i);
		if(it.curve) {
			it.curve->detach();
			delete it.curve;
		}
		if(!it.handle.isNull()) {
			if(m_plot) m_plot->removePlotAxisHandle(it.handle);
			it.handle->deleteLater();
		}
		return;
	}
}

void DigitalTrackManager::setChannelVisible(const QString &sourceId,
                                            const QString &channelName,
                                            bool visible)
{
	// Match by rebuilding the raw key: keeps the manager decoupled from
	// any DataKey parsing quirks (e.g. underscore-in-sourceId).
	const scopy::acq::DataKey target =
		scopy::acq::DataKey::raw(sourceId, channelName);

	bool changed = false;
	for(Item &it : m_items) {
		if(it.kind != Item::Kind::RawCurve) continue;
		if(it.srcKey != target)             continue;
		if(it.curve)  it.curve->setVisible(visible);
		if(!it.handle.isNull() && it.handle->handle())
			it.handle->handle()->setVisible(visible);
		changed = true;
		break;
	}
	if(changed && m_plot)
		m_plot->plot()->replot();
}

void DigitalTrackManager::updateRawCurves(int plotSize)
{
	if(!m_store) return;
	for(Item &it : m_items) {
		if(it.kind != Item::Kind::RawCurve || !it.curve) continue;

		const scopy::acq::SampleVariant v =
			m_store->readWindowNative(it.srcKey, plotSize);

		QVector<quint8> bits;
		std::visit([&](const auto &vec) {
			using VecT = std::decay_t<decltype(vec)>;
			if constexpr(std::is_same_v<VecT, QVector<scopy::acq::Annotation>>) {
				// Not applicable; leave bits empty.
			} else {
				bits.resize(vec.size());
				for(int i = 0; i < vec.size(); ++i)
					bits[i] = (vec[i] != 0) ? quint8(1) : quint8(0);
			}
		}, v);

		it.curve->setSampleCount(static_cast<quint64>(plotSize));
		it.curve->setSamples(bits);
	}
}

} // namespace adc
} // namespace scopy
