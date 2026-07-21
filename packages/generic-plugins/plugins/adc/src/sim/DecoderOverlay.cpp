#include "DecoderOverlay.h"

#include "acq_engine/DataKey.h"
#include "acq_engine/DataStore.h"
#include "acq_engine/ExternalDecoderProcessor.h"
#include "acq_engine/SampleBuffer.h"
#include "gui/annotationcurve.h"
#include "gui/plotaxis.h"
#include "gui/plotaxishandle.h"
#include "gui/plotwidget.h"

#include <QwtPlot>

#include <QEvent>
#include <QHelpEvent>
#include <QMouseEvent>
#include <QToolTip>
#include <QWidget>
#include <qwt_scale_map.h>
#include <variant>

namespace scopy {
namespace adc {

DecoderOverlay::DecoderOverlay(PlotWidget *plot, scopy::acq::DataStore *store,
			       QObject *parent)
	: QObject(parent)
	, m_plot(plot)
	, m_store(store)
{
	// Watch the plot canvas and drive per-annotation tooltips from a
	// single-shot hover timer instead of directly from MouseMove. Firing
	// QToolTip::showText from inside a MouseMove handler triggered a
	// tooltip-window ↔ synthetic Leave/MouseMove feedback loop during
	// zoom-magnify that leaked QTipLabel widgets until the process (and
	// often the user session) was OOM-killed. The hover timer is the
	// same pattern Qt itself uses for built-in tooltips.
	m_hoverTimer.setSingleShot(true);
	m_hoverTimer.setInterval(250);
	connect(&m_hoverTimer, &QTimer::timeout, this,
		&DecoderOverlay::showPendingTooltip);

	if(m_plot && m_plot->plot() && m_plot->plot()->canvas()) {
		QWidget *cv = m_plot->plot()->canvas();
		cv->setMouseTracking(true);
		cv->installEventFilter(this);
	}
}

DecoderOverlay::~DecoderOverlay() { m_curves.clear(); }

void DecoderOverlay::registerDecoder(scopy::acq::ExternalDecoderProcessor *proc,
				     const scopy::acq::DataKey &outKey,
				     PlotAxisHandle *handle,
				     const QString &title)
{
	if(!proc || !m_plot || !handle)
		return;

	if(m_curves.contains(outKey))
		return;

	scopy::PlotAxis *yAxis = m_annYAxis.isNull() ? m_plot->yAxis()
	                                             : m_annYAxis.data();
	auto *curve = new AnnotationCurve(title, m_plot->xAxis(), yAxis, handle);
	// Decoder annotations are always shown once registered. Visibility is
	// no longer gated by the curve Y-combo — the user manages per-decoder
	// presence via the DecoderPanel (Add / Remove) instead.
	curve->setVisible(true);
	curve->attach(m_plot->plot());
	m_curves.insert(outKey, curve);

	connect(proc, &scopy::acq::ExternalDecoderProcessor::cycleProduced,
		this, &DecoderOverlay::onCycleProduced,
		Qt::ConnectionType(Qt::QueuedConnection | Qt::UniqueConnection));
}

void DecoderOverlay::unregisterDecoder(const scopy::acq::DataKey &outKey)
{
	auto it = m_curves.find(outKey);
	if(it == m_curves.end())
		return;
	AnnotationCurve *curve = it.value();
	m_curves.erase(it);
	if(curve) {
		curve->detach();
		delete curve;
	}
	if(m_plot)
		m_plot->replot();
}

void DecoderOverlay::setSampleCount(quint64 n)
{
	if(m_plot.isNull())
		return;
	for(AnnotationCurve *c : std::as_const(m_curves))
		c->setSampleCount(n);
}

void DecoderOverlay::clear()
{
	if(m_plot.isNull()) { m_curves.clear(); return; }
	for(AnnotationCurve *c : std::as_const(m_curves)) {
		c->detach();
		delete c;
	}
	m_curves.clear();
}

void DecoderOverlay::onCycleProduced(scopy::acq::DataKey outKey)
{
	if(!m_store)
		return;
	AnnotationCurve *curve = m_curves.value(outKey, nullptr);
	if(!curve)
		return;

	const scopy::acq::SampleBuffer buf = m_store->read(outKey);
	if(buf.empty()) {
		curve->clear();
		return;
	}

	const scopy::acq::SampleVariant &v = buf.sample(0);
	if(!std::holds_alternative<QVector<scopy::acq::Annotation>>(v)) {
		curve->clear();
		return;
	}

	const auto &anns = std::get<QVector<scopy::acq::Annotation>>(v);

	QVector<AnnotationSpan> spans;
	spans.reserve(anns.size());
	for(const scopy::acq::Annotation &a : anns) {
		AnnotationSpan s;
		s.startSample = a.startSample;
		s.endSample   = a.endSample;
		s.klass       = a.klass;
		s.text        = a.text;
		spans.append(s);
	}

	curve->setAnnotations(spans);
	if(m_plot)
		m_plot->replot();
}

bool DecoderOverlay::eventFilter(QObject *watched, QEvent *ev)
{
	if(!m_plot || !m_plot->plot() || m_plot->plot()->canvas() != watched)
		return QObject::eventFilter(watched, ev);

	switch(ev->type()) {
	case QEvent::MouseMove: {
		auto *me = static_cast<QMouseEvent *>(ev);
		m_pendingCanvasPos = me->position().toPoint();
		m_pendingGlobalPos = me->globalPosition().toPoint();

		// If the cursor moved meaningfully away from where the last
		// tooltip was shown, hide it right away. If the cursor is
		// essentially still (synthetic MouseMove from a tooltip
		// window appearing/disappearing) leave the existing tooltip
		// alone — no allocation happens here.
		const int dx = m_pendingGlobalPos.x() - m_lastTipGlobal.x();
		const int dy = m_pendingGlobalPos.y() - m_lastTipGlobal.y();
		if(!m_lastTip.isEmpty() && (dx * dx + dy * dy) > 4) {
			QToolTip::hideText();
			m_lastTip.clear();
			m_lastTipGlobal = QPoint();
		}

		// (Re)start the hover timer. showText is deferred until the
		// cursor has been still for setInterval() ms.
		m_hoverTimer.start();
		break;
	}
	case QEvent::Leave: {
		m_hoverTimer.stop();
		if(!m_lastTip.isEmpty()) {
			QToolTip::hideText();
			m_lastTip.clear();
			m_lastTipGlobal = QPoint();
		}
		break;
	}
	case QEvent::Wheel: {
		// Wheel drives PlotMagnifier zoom/pan. Suppress any pending
		// tooltip and drop the current one so the tooltip cannot
		// churn while the x-scale is changing rapidly. The event is
		// not consumed — PlotMagnifier still gets it.
		m_hoverTimer.stop();
		if(!m_lastTip.isEmpty()) {
			QToolTip::hideText();
			m_lastTip.clear();
			m_lastTipGlobal = QPoint();
		}
		break;
	}
	default:
		break;
	}

	return QObject::eventFilter(watched, ev);
}

void DecoderOverlay::showPendingTooltip()
{
	if(!m_plot || !m_plot->plot())
		return;
	QWidget *cv = m_plot->plot()->canvas();
	if(!cv)
		return;
	// If the cursor left the canvas since the timer was armed, do
	// nothing — the Leave handler will have hidden any active tooltip.
	if(!cv->underMouse())
		return;

	const QString tip = tooltipForCanvasPos(m_pendingCanvasPos);
	if(tip.isEmpty()) {
		if(!m_lastTip.isEmpty()) {
			QToolTip::hideText();
			m_lastTip.clear();
			m_lastTipGlobal = QPoint();
		}
		return;
	}

	if(tip == m_lastTip)
		return; // already showing this exact tip

	QToolTip::showText(m_pendingGlobalPos, tip, cv);
	m_lastTip       = tip;
	m_lastTipGlobal = m_pendingGlobalPos;
}

QString DecoderOverlay::tooltipForCanvasPos(const QPointF &canvasPos) const
{
	if(!m_plot || !m_plot->plot())
		return {};

	QwtPlot *plot   = m_plot->plot();
	QWidget *canvas = plot->canvas();
	if(!canvas)
		return {};

	const QRectF canvasRect(0, 0, canvas->width(), canvas->height());

	for(auto it = m_curves.constBegin(); it != m_curves.constEnd(); ++it) {
		AnnotationCurve *curve = it.value();
		if(!curve || !curve->isVisible())
			continue;

		const QwtScaleMap xMap = plot->canvasMap(curve->xAxis());
		const QwtScaleMap yMap = plot->canvasMap(curve->yAxis());

		const auto hit =
			curve->hitTest(canvasPos, xMap, yMap, canvasRect);
		if(!hit)
			continue;

		// Rich-text tooltip: bold text, dim metadata line.
		QString tip;
		if(!hit->text.isEmpty())
			tip += QStringLiteral("<b>%1</b>").arg(hit->text.toHtmlEscaped());
		if(!hit->klass.isEmpty()) {
			if(!tip.isEmpty()) tip += QStringLiteral("<br/>");
			tip += QStringLiteral("<i>%1</i>").arg(hit->klass.toHtmlEscaped());
		}
		QString range;
		if(hit->startSample == hit->endSample)
			range = QStringLiteral("sample %1").arg(hit->startSample);
		else
			range = QStringLiteral("samples %1 – %2 (%3)")
				.arg(hit->startSample)
				.arg(hit->endSample)
				.arg(hit->endSample - hit->startSample);
		if(!tip.isEmpty()) tip += QStringLiteral("<br/>");
		tip += QStringLiteral("<span style=\"color:gray;\">%1 · %2</span>")
			.arg(it.key().toString().toHtmlEscaped(), range);
		return tip;
	}
	return {};
}

} // namespace adc
} // namespace scopy
