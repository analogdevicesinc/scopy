#include "handlesareaextension.h"

#include <QObject>

#include <qwt_plot.h>

#include "handles_area.hpp"
#include "oscilloscope_plot.hpp"
#include "dbgraph.hpp"
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
        //const QString text = plot->formatter->format(currentTime,"Hz", 2);

        const QString text = plot->timeScaleValueFormat(currentTime, 2);

        const QSizeF textSize = QwtText(text).textSize(painter->font());
		QRectF textRect(QPointF(0.0, 0.0), textSize);

		textRect.moveCenter(QPointF(midPoint, textSize.height() / 2.0));

		labelRectangles.push_back(textRect);
		labelTexts.push_back(text);

		midPoint += distBetween2Labels;
		currentTime += timeBetween2Labels;
	}

	bool allLabelsTheSame = true;
	for (int i = 1; i < labelTexts.size(); ++i) {
		if (labelTexts[i] != labelTexts[i - 1]) {
			allLabelsTheSame = false;
			break;
		}
	}

	// get nr of major ticks
	const int nrMajorTicks = plot->axisScaleDiv(QwtPlot::xBottom).ticks(QwtScaleDiv::MajorTick).size();
	const int midLabelTick = nrMajorTicks / 2;

	if (allLabelsTheSame) {
		// draw delta as middle label
		labelRectangles.clear();
		labelTexts.clear();
		midPoint = leftP;
		currentTime = interval.minValue();
		for (int i = 0; i < labels; ++i) {
			QString text;
			if (i == midLabelTick) {
                text = plot->timeScaleValueFormat(currentTime, 6);

                //text = plot->formatter->format(currentTime,"Hz", 2);
			} else {   
                text = plot->timeScaleValueFormat(currentTime, 6);

                //text = plot->formatter->format(currentTime - (interval.minValue() + midLabelTick * timeBetween2Labels), "", 2);
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
			currentTime += timeBetween2Labels;
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

    const dBgraph *plot = qobject_cast<dBgraph*>(m_plot);
    if (!plot) {
        return false;
    }

    // Some extensions might alter the state of the painter
    // such as its brush or pen. Consider saving its state
    // then restore it when done using it so other extensions
    // won't be affected

    // PainterSaveRestore psr(painter);

    const QwtInterval interval = plot->axisInterval(QwtPlot::xTop);

    const double leftP = area->leftPadding();
    const double rightP = area->rightPadding();

    const int labels = plot->xAxisNumDiv();

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
        const QString text = plot->formatter->format(currentTime,"Hz", 2);

        const QSizeF textSize = QwtText(text).textSize(painter->font());
        QRectF textRect(QPointF(0.0, 0.0), textSize);

        textRect.moveCenter(QPointF(midPoint, textSize.height() / 2.0));

        labelRectangles.push_back(textRect);
        labelTexts.push_back(text);

        midPoint += distBetween2Labels;
        currentTime += timeBetween2Labels;
    }

    bool allLabelsTheSame = true;
    for (int i = 1; i < labelTexts.size(); ++i) {
        if (labelTexts[i] != labelTexts[i - 1]) {
            allLabelsTheSame = false;
            break;
        }
    }

    // get nr of major ticks
    const int nrMajorTicks = plot->axisScaleDiv(QwtPlot::xTop).ticks(QwtScaleDiv::MajorTick).size();
    const int midLabelTick = nrMajorTicks / 2;

    if (allLabelsTheSame) {
        // draw delta as middle label
        labelRectangles.clear();
        labelTexts.clear();
        midPoint = leftP;
        currentTime = interval.minValue();
        for (int i = 0; i < labels; ++i) {
            QString text;
            if (i == midLabelTick) {
                text = plot->formatter->format(currentTime,"Hz", 2);
            } else {
                text = plot->formatter->format(currentTime - (interval.minValue() + midLabelTick * timeBetween2Labels), "", 2);
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
            currentTime += timeBetween2Labels;
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

    //sa incerc direct din display plot candva
    const dBgraph *plot = qobject_cast<dBgraph*>(m_plot);
    if (!plot) {
        return false;
    }

    const QwtInterval interval = plot->axisInterval(QwtPlot::yLeft);

    const double topP = area->topPadding();
    const double bottomP = area->bottomPadding();

    const int labels = plot->yAxisNumDiv();

    const double totalHeight = owner->height() - (topP + bottomP);
    const double distBetween2Labels = totalHeight / (labels - 1);
    const double valueBetween2Labels = (interval.maxValue() - interval.minValue()) / (labels - 1);

    // compute rectangles of labels and
    // corresponding text
    QVector<QRectF> labelRectangles;
    QStringList labelTexts;
    double midPoint = topP;
    double currentValue = interval.maxValue();

    for (int i = 0; i < labels; ++i) {
        const QString text = plot->formatter->format(currentValue, plot->yUnit(), 2);
        QSizeF textSize = QwtText(text).textSize(painter->font());
        textSize.setWidth(textSize.width() + 20);

        QRectF textRect(QPointF(0.0, 0.0), textSize);

        textRect.moveCenter(QPointF(textSize.width() / 2.0, midPoint));

        labelRectangles.push_back(textRect);
        labelTexts.push_back(text);

        midPoint += distBetween2Labels;
        currentValue -= valueBetween2Labels;
    }

    bool allLabelsTheSame = true;
    for (int i = 1; i < labelTexts.size(); ++i) {
        if (labelTexts[i] != labelTexts[i - 1]) {
            allLabelsTheSame = false;
            break;
        }
    }

    // get nr of major ticks
    const int nrMajorTicks = plot->axisScaleDiv(QwtPlot::yLeft).ticks(QwtScaleDiv::MajorTick).size();
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
                text = plot->formatter->format(currentValue, plot->yUnit(), 2);
            } else {
                text = plot->formatter->format(currentValue - (interval.maxValue() + midLabelTick * valueBetween2Labels), plot->yUnit(), 2);
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

    // draw the labels
    for (int i = 0; i < labelRectangles.size(); ++i) {
        painter->drawText(labelRectangles[i], labelTexts[i]);
    }

    return false;
}
