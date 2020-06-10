#include "handlesareaextension.h"

#include <QObject>

#include <qwt_plot.h>

#include "handles_area.hpp"
#include "oscilloscope_plot.hpp"
#include "paintersaverestore.h"

using namespace adiscope;

HandlesAreaExtension::HandlesAreaExtension(QwtPlot* plot)
	: m_plot(plot) {}

XBottomRuller::XBottomRuller(QwtPlot *plot)
	: HandlesAreaExtension(plot) {}

bool XBottomRuller::draw(QPainter *painter, QWidget *owner)
{
	HorizHandlesArea *area = qobject_cast<HorizHandlesArea*>(owner);
	if (!area) {
		return false;
	}

	const CapturePlot *plot = qobject_cast<CapturePlot*>(m_plot);
	if (!plot) {
		return false;
	}

	// Some extensions might alter the state of the painter
	// such as its brush or pen. Consider saving its state
	// then restore it when done using it so other extensions
	// won't be affected

	// PainterSaveRestore psr(painter);

	const QwtInterval interval = plot->axisInterval(QwtPlot::xBottom);

	const double leftP = area->leftPadding();
	const double rightP = area->rightPadding();

	const int labels = plot->xAxisNumDiv() + 1;

	const double totalWidth = owner->width() - (leftP + rightP);
	const double distBetween2Labels = totalWidth / (labels - 1);
	const double timeBetween2Labels = (interval.maxValue() - interval.minValue()) / (labels - 1);

	// compute rectangles of labels and
	// corresponding text
	QVector<QRectF> labelRectangles;
	QStringList labelTexts;
	double midPoint = leftP;
	double currentTime = interval.minValue();
	for (int i = 0; i < labels; ++i) {
		const QString text = plot->timeScaleValueFormat(currentTime, 2);

		const QSizeF textSize = QwtText(text).textSize(painter->font());
		QRectF textRect(QPointF(0.0, 0.0), textSize);

		textRect.moveCenter(QPointF(midPoint, textSize.height() / 2.0));

		labelRectangles.push_back(textRect);
		labelTexts.push_back(text);

		midPoint += distBetween2Labels;
		currentTime += timeBetween2Labels;
	}

	// adjust labels to fit visible area of the handle area
	// mainly the first and last label
	if (labelRectangles.first().topLeft().x() < owner->mask().boundingRect().bottomLeft().x()) {
		int offset = owner->mask().boundingRect().bottomLeft().x() - labelRectangles.first().topLeft().x();
		labelRectangles.first().adjust(offset, 0, offset, 0);
	}

	if (labelRectangles.last().bottomRight().x() > owner->mask().boundingRect().bottomRight().x()) {
		int offset = labelRectangles.last().bottomRight().x() - owner->mask().boundingRect().bottomRight().x();
		labelRectangles.last().adjust(-offset, 0, -offset, 0);
	}

	// filter out overlaping labels, but always drawing
	// the first and last label
	bool overlaping = false;
	do {
		// consider none overlaping
		overlaping = false;

		// find overlaping
		int i = 0;
		for (; i < labelRectangles.size() - 1; ++i) {
			if (labelRectangles[i].intersects(labelRectangles[i + 1])) {
				overlaping = true;
				break;
			}
		}

		// done
		if (!overlaping) {
			break;
		}

		// remove overlaping
		if (i + 1 == labelRectangles.size() - 1) {
			labelRectangles.removeAt(i);
			labelTexts.removeAt(i);
		} else {
			labelRectangles.removeAt(i + 1);
			labelTexts.removeAt(i + 1);
		}
	} while(overlaping);

	// draw the labels
	for (int i = 0; i < labelRectangles.size(); ++i) {
		painter->drawText(labelRectangles[i], labelTexts[i]);
	}

	return false;
}
