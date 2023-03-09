#include "handlesareaextension.h"

#include <QObject>

#include <qwt_plot.h>

#include "handles_area.hpp"
#include "DisplayPlot.h"
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

	const DisplayPlot *plot = qobject_cast<DisplayPlot*>(m_plot);
	if (!plot) {
		return false;
	}

	const double leftP = area->leftPadding();
	const double rightP = area->rightPadding();

	QVector<QRectF> labelRectangles;
	QStringList labelTexts;

	if(plot->isLogaritmicPlot())
	{
		QList<double> majorTicks = plot->getXaxisMajorTicksPos();
		double pointVal, currentValue;

		for (int i = 0; i < majorTicks.size(); ++i) {
			currentValue = majorTicks.at(i);
			pointVal = plot->transform(QwtAxis::XBottom, currentValue) + leftP;

			const QString text = plot->formatXValue(currentValue, 2);

			const QSizeF textSize = QwtText(text).textSize(painter->font());
			QRectF textRect(QPointF(0.0, 0.0), textSize);

			textRect.moveCenter(QPointF(pointVal, textSize.height() / 2.0));

			labelRectangles.push_back(textRect);
			labelTexts.push_back(text);
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
	}
	else
	{
		// Some extensions might alter the state of the painter
		// such as its brush or pen. Consider saving its state
		// then restore it when done using it so other extensions
		// won't be affected

		// PainterSaveRestore psr(painter);

		const QwtInterval interval = plot->axisInterval(QwtAxis::XBottom);

		const int labels = plot->xAxisNumDiv() + 1;

		const double totalWidth = owner->width() - (leftP + rightP);
		const double distBetween2Labels = totalWidth / (labels - 1);
		const double valueBetween2Labels = (interval.maxValue() - interval.minValue()) / (labels - 1);

		double midPoint = leftP;
		double currentValue = interval.minValue();
		for (int i = 0; i < labels; ++i) {

			const QString text = plot->formatXValue(currentValue, 2);

			const QSizeF textSize = QwtText(text).textSize(painter->font());
			QRectF textRect(QPointF(0.0, 0.0), textSize);

			textRect.moveCenter(QPointF(midPoint, textSize.height() / 2.0));

			labelRectangles.push_back(textRect);
			labelTexts.push_back(text);

			midPoint += distBetween2Labels;
			currentValue += valueBetween2Labels;
		}

		bool allLabelsTheSame = true;
		for (int i = 1; i < labelTexts.size(); ++i) {
			if (labelTexts[i] != labelTexts[i - 1]) {
				allLabelsTheSame = false;
				break;
			}
		}

		// get nr of major ticks
		const int nrMajorTicks = plot->axisScaleDiv(QwtAxis::XBottom).ticks(QwtScaleDiv::MajorTick).size();
		const int midLabelTick = nrMajorTicks / 2;

		if (allLabelsTheSame) {
			// draw delta as middle label
			labelRectangles.clear();
			labelTexts.clear();
			midPoint = leftP;
			currentValue = interval.minValue();
			for (int i = 0; i < labels; ++i) {
				QString text;
				if (i == midLabelTick) {
					text = plot->formatXValue(currentValue, 6);
				} else {
					text = plot->formatXValue(currentValue - (interval.minValue() + midLabelTick * valueBetween2Labels), 2);

					if (i > midLabelTick) {
						text = "+" + text;
					}
				}

				const QSizeF textSize = QwtText(text).textSize(painter->font());
				QRectF textRect(QPointF(0.0, 0.0), textSize);

				textRect.moveCenter(QPointF(midPoint, textSize.height() / 2.0));

				labelRectangles.push_back(textRect);
				labelTexts.push_back(text);

				midPoint += distBetween2Labels;
				currentValue += valueBetween2Labels;
			}

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
			if (allLabelsTheSame) {
				int center = midLabelTick;
				for (int i = center - 1; i >= 0; i -= 2) {
					// Remove the tick and make sure to update the center
					// label position
					labelRectangles.removeAt(i);
					labelTexts.removeAt(i);
					--center;
				}
				for (int j = center + 1; j < labelRectangles.size(); j += 1) {
					labelRectangles.removeAt(j);
					labelTexts.removeAt(j);
				}

			} else {
				// remove overlaping
				if (i + 1 == labelRectangles.size() - 1) {
					labelRectangles.removeAt(i);
					labelTexts.removeAt(i);
				} else {
					labelRectangles.removeAt(i + 1);
					labelTexts.removeAt(i + 1);
				}
			}
		} while(overlaping);

	}
	// draw the labels
	for (int i = 0; i < labelRectangles.size(); ++i) {
		painter->drawText(labelRectangles[i], labelTexts[i]);
	}

	return false;
}

XTopRuller::XTopRuller(QwtPlot *plot)
	: HandlesAreaExtension(plot) {}

bool XTopRuller::draw(QPainter *painter, QWidget *owner)
{
	HorizHandlesArea *area = qobject_cast<HorizHandlesArea*>(owner);
	if (!area) {
		return false;
	}

	const DisplayPlot *plot = qobject_cast<DisplayPlot*>(m_plot);
	if (!plot) {
		return false;
	}

	const double leftP = area->leftPadding();
	const double rightP = area->rightPadding();

	QVector<QRectF> labelRectangles;
	QStringList labelTexts;

	if(plot->isLogaritmicPlot())
	{
		QList<double> majorTicks = plot->getXaxisMajorTicksPos();
		double pointVal, currentValue;

		for (int i = 0; i < majorTicks.size(); ++i) {
			currentValue = majorTicks.at(i);
			pointVal = plot->transform(QwtAxis::XTop, currentValue) + leftP;

			const QString text = plot->formatXValue(currentValue, 2);

			const QSizeF textSize = QwtText(text).textSize(painter->font());
			QRectF textRect(QPointF(0.0, 0.0), textSize);

			textRect.moveCenter(QPointF(pointVal + textSize.width() / 2.0, textSize.height() / 2.0));

			labelRectangles.push_back(textRect);
			labelTexts.push_back(text);
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


	}
	else
	{
		// Some extensions might alter the state of the painter
		// such as its brush or pen. Consider saving its state
		// then restore it when done using it so other extensions
		// won't be affected

		// PainterSaveRestore psr(painter);

		const QwtInterval interval = plot->axisInterval(QwtAxis::XTop);

		const int labels = plot->xAxisNumDiv() + 1;

		const double totalWidth = owner->width() - (leftP + rightP);
		const double distBetween2Labels = totalWidth / (labels - 1);
		const double valueBetween2Labels = (interval.maxValue() - interval.minValue()) / (labels - 1);
		double midPoint = leftP;
		double currentValue = interval.minValue();

		for (int i = 0; i < labels; ++i) {
			const QString text = plot->formatXValue(currentValue, 2);

			const QSizeF textSize = QwtText(text).textSize(painter->font());
			QRectF textRect(QPointF(0.0, 0.0), textSize);

			textRect.moveCenter(QPointF(midPoint, textSize.height() / 2.0));

			labelRectangles.push_back(textRect);
			labelTexts.push_back(text);

			midPoint += distBetween2Labels;
			currentValue += valueBetween2Labels;
		}

		bool allLabelsTheSame = true;
		for (int i = 1; i < labelTexts.size(); ++i) {
			if (labelTexts[i] != labelTexts[i - 1]) {
				allLabelsTheSame = false;
				break;
			}
		}

		// get nr of major ticks
		const int nrMajorTicks = plot->axisScaleDiv(QwtAxis::XTop).ticks(QwtScaleDiv::MajorTick).size();
		const int midLabelTick = nrMajorTicks / 2;

		if (allLabelsTheSame) {
			// draw delta as middle label
			labelRectangles.clear();
			labelTexts.clear();
			midPoint = leftP;
			currentValue = interval.minValue();
			for (int i = 0; i < labels; ++i) {
				QString text;
				if (i == midLabelTick) {
					text = plot->formatXValue(currentValue, 2);
				} else {
					text= plot->formatXValue(currentValue - (interval.minValue() + midLabelTick * valueBetween2Labels), 2);
					if (i > midLabelTick) {
						text = "+" + text;
					}
				}

				const QSizeF textSize = QwtText(text).textSize(painter->font());
				QRectF textRect(QPointF(0.0, 0.0), textSize);

				textRect.moveCenter(QPointF(midPoint, textSize.height() / 2.0));

				labelRectangles.push_back(textRect);
				labelTexts.push_back(text);

				midPoint += distBetween2Labels;
				currentValue += valueBetween2Labels;
			}

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
			if (allLabelsTheSame) {
				int center = midLabelTick;
				for (int i = center - 1; i >= 0; i -= 2) {
					// Remove the tick and make sure to update the center
					// label position
					labelRectangles.removeAt(i);
					labelTexts.removeAt(i);
					--center;
				}
				for (int j = center + 1; j < labelRectangles.size(); j += 1) {
					labelRectangles.removeAt(j);
					labelTexts.removeAt(j);
				}

			} else {
				// remove overlaping
				if (i + 1 == labelRectangles.size() - 1) {
					labelRectangles.removeAt(i);
					labelTexts.removeAt(i);
				} else {
					labelRectangles.removeAt(i + 1);
					labelTexts.removeAt(i + 1);
				}
			}
		} while(overlaping);
	}

	// draw the labels
	for (int i = 0; i < labelRectangles.size(); ++i) {
		painter->drawText(labelRectangles[i], labelTexts[i]);
	}

	return false;
}

YLeftRuller::YLeftRuller(QwtPlot *plot)
	: HandlesAreaExtension(plot) {}

bool YLeftRuller::draw(QPainter *painter, QWidget *owner)
{
	VertHandlesArea *area = qobject_cast<VertHandlesArea*>(owner);
	if (!area) {
		return false;
	}

	auto width_text = QFontMetrics(painter->font()).horizontalAdvance("-XXX.XX XX");
	if(area->width() != width_text)
		area->setMinimumWidth(width_text);

	const DisplayPlot *plot = qobject_cast<DisplayPlot*>(m_plot);
	if (!plot) {
		return false;
	}


	const double topP = area->topPadding();
	const double bottomP = area->bottomPadding();

	QVector<QRectF> labelRectangles;
	QStringList labelTexts;

	if(plot->isLogaritmicYPlot())
	{
		QList<double> majorTicks = plot->getYaxisMajorTicksPos();
		double pointVal, currentValue;

		for (int i = 0; i < majorTicks.size(); ++i) {
			currentValue = majorTicks.at(i);
			pointVal = plot->transform(QwtAxis::YLeft, currentValue) + topP;

			const QString text = plot->formatYValue(currentValue, 2);

			const QSizeF textSize = QwtText(text).textSize(painter->font());
			QRectF textRect(QPointF(0.0, 0.0), textSize);

			textRect.moveCenter(QPointF(textSize.width() / 2.0, pointVal));

			labelRectangles.push_back(textRect);
			labelTexts.push_back(text);
		}

		// adjust labels to fit visible area of the handle area
		// mainly the first and last label
		if(!labelRectangles.empty())
		{
			if (labelRectangles.first().topRight().y() < owner->mask().boundingRect().topLeft().y()) {
				int offset = owner->mask().boundingRect().topLeft().y() - labelRectangles.first().topRight().y();
				labelRectangles.first().adjust(0, offset, 0, offset);
			}

			if (labelRectangles.last().bottomRight().y() > owner->mask().boundingRect().bottomLeft().y()) {
				int offset = labelRectangles.last().bottomRight().y() - owner->mask().boundingRect().bottomLeft().y();
				labelRectangles.last().adjust(0, -offset, 0, -offset);
			}
		}
	}
	else
	{

		const QwtInterval interval = plot->axisInterval(QwtAxis::YLeft);

		const int labels = plot->yAxisNumDiv();

		const double totalHeight = owner->height() - (topP + bottomP);
		const double distBetween2Labels = totalHeight / (labels - 1);
		const double valueBetween2Labels = (interval.maxValue() - interval.minValue()) / (labels - 1);

		double midPoint = topP;

		QList<double> majorTicks = plot->axisScaleDiv(QwtAxis::YLeft).ticks(QwtScaleDiv::MajorTick);

		double pointVal, currentValue;
		for (int i = 0; i < majorTicks.size(); ++i) {
			// this is the same way ticks are parsed in logarithmic mode too
			// this whole function may need refactoring soon ...

			currentValue = majorTicks.at(i);
			pointVal = plot->transform(QwtAxis::YLeft, currentValue) + topP;

			const QString text = plot->formatYValue(currentValue, 2);

			const QSizeF textSize = QwtText(text).textSize(painter->font());
			QRectF textRect(QPointF(0.0, 0.0), textSize);

			textRect.moveCenter(QPointF(textSize.width() / 2.0, pointVal));

			labelRectangles.push_back(textRect);
			labelTexts.push_back(text);
		}

		bool allLabelsTheSame = true;
		for (int i = 1; i < labelTexts.size(); ++i) {
			if (labelTexts[i] != labelTexts[i - 1]) {
				allLabelsTheSame = false;
				break;
			}
		}

		// get nr of major ticks
		const int nrMajorTicks = plot->axisScaleDiv(QwtAxis::YLeft).ticks(QwtScaleDiv::MajorTick).size();
		const int midLabelTick = nrMajorTicks / 2;

		if (allLabelsTheSame) {
			// draw delta as middle label
			labelRectangles.clear();
			labelTexts.clear();
			midPoint = topP;
			currentValue = interval.maxValue();
			for (int i = 0; i < labels; ++i) {
				QString text;
				if (i == midLabelTick) {
					text = plot->formatYValue(currentValue, 2);
				} else {
					text = plot->formatYValue(currentValue - (interval.maxValue() + midLabelTick * valueBetween2Labels), 2);
					if (i > midLabelTick) {
						text = "+" + text;
					}
				}

				QSizeF textSize = QwtText(text).textSize(painter->font());
				textSize.setWidth(textSize.width() + 20);
				QRectF textRect(QPointF(0.0, 0.0), textSize);
				textRect.moveCenter(QPointF(textSize.width() / 2.0, midPoint));

				labelRectangles.push_back(textRect);
				labelTexts.push_back(text);

				midPoint += distBetween2Labels;
				currentValue -= valueBetween2Labels;
			}
		}

		// adjust labels to fit visible area of the handle area
		// mainly the first and last label
		if (labelRectangles.first().topRight().y() < owner->mask().boundingRect().topLeft().y()) {
			int offset = owner->mask().boundingRect().topLeft().y() - labelRectangles.first().topRight().y();
			labelRectangles.first().adjust(0, offset, 0, offset);
		}

		if (labelRectangles.last().bottomRight().y() > owner->mask().boundingRect().bottomLeft().y()) {
			int offset = labelRectangles.last().bottomRight().y() - owner->mask().boundingRect().bottomLeft().y();
			labelRectangles.last().adjust(0, -offset, 0, -offset);
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
			if (allLabelsTheSame) {
				int center = midLabelTick;
				for (int i = center - 1; i >= 0; i -= 2) {
					// Remove the tick and make sure to update the center
					// label position
					labelRectangles.removeAt(i);
					labelTexts.removeAt(i);
					--center;
				}
				for (int j = center + 1; j < labelRectangles.size(); j += 1) {
					labelRectangles.removeAt(j);
					labelTexts.removeAt(j);
				}

			}else {
				// remove overlaping
				if (i + 1 == labelRectangles.size() - 1) {
					labelRectangles.removeAt(i);
					labelTexts.removeAt(i);
				} else {
					labelRectangles.removeAt(i + 1);
					labelTexts.removeAt(i + 1);
				}
			}
		}while(overlaping);
	}

	// draw the labels
	for (int i = 0; i < labelRectangles.size(); ++i) {
		painter->drawText(labelRectangles[i], labelTexts[i]);
	}

	return false;
}
