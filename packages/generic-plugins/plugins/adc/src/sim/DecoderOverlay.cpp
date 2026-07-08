#include "DecoderOverlay.h"

#include "acq_engine/DataKey.h"
#include "acq_engine/DataStore.h"
#include "acq_engine/ExternalDecoderProcessor.h"
#include "acq_engine/SampleBuffer.h"
#include "gui/annotationcurve.h"
#include "gui/plotaxis.h"
#include "gui/plotwidget.h"

#include <QwtPlot>

#include <QEvent>
#include <QHelpEvent>
#include <QMouseEvent>
#include <QSet>
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
	// Watch the plot canvas so we can drive per-annotation tooltips from
	// mouse-move events. The canvas already handles Qt::ToolTip events
	// (from the Qwt picker infrastructure) so we listen to MouseMove and
	// drive QToolTip::showText explicitly — this lets the tooltip follow
	// the cursor across the canvas without waiting for a hover timeout.
	if(m_plot && m_plot->plot() && m_plot->plot()->canvas()) {
		QWidget *cv = m_plot->plot()->canvas();
		cv->setMouseTracking(true);
		cv->installEventFilter(this);
	}
}

DecoderOverlay::~DecoderOverlay() { m_curves.clear(); }

void DecoderOverlay::registerDecoder(scopy::acq::ExternalDecoderProcessor *proc,
				     PlotAxis *yAxis)
{
	if(!proc || !m_plot || !yAxis)
		return;

	const scopy::acq::DataKey outKey = proc->outputKey();
	if(m_curves.contains(outKey))
		return;

	auto *curve = new AnnotationCurve(outKey.toString(),
					  m_plot->xAxis(), yAxis);
	curve->setVisible(false); // hidden until setVisibleKeys() enables it
	curve->attach(m_plot->plot());
	m_curves.insert(outKey, curve);

	connect(proc, &scopy::acq::ExternalDecoderProcessor::cycleProduced,
		this, &DecoderOverlay::onCycleProduced,
		Qt::QueuedConnection);
}

void DecoderOverlay::setVisibleKeys(const QList<scopy::acq::DataKey> &keys)
{
	const QSet<scopy::acq::DataKey> selected(keys.begin(), keys.end());
	for(auto it = m_curves.constBegin(); it != m_curves.constEnd(); ++it) {
		const bool visible = selected.contains(it.key());
		it.value()->setVisible(visible);
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

	if(ev->type() == QEvent::MouseMove) {
		auto *me = static_cast<QMouseEvent *>(ev);
		const QString tip = tooltipForCanvasPos(me->position());
		QWidget *cv = m_plot->plot()->canvas();
		if(tip.isEmpty()) {
			QToolTip::hideText();
		} else {
			QToolTip::showText(me->globalPosition().toPoint(),
					   tip, cv);
		}
	} else if(ev->type() == QEvent::Leave) {
		QToolTip::hideText();
	}

	return QObject::eventFilter(watched, ev);
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

		const AnnotationSpan *hit =
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
