/*
 * Copyright (c) 2020 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#include "annotationcurve.h"

#include "annotationdecoder.h"

#include "binding/decoder.hpp"

#include <iostream>
#include <qwt_plot.h>
#include <qwt_painter.h>
#include <qwt_painter.h>
#include <qwt_series_data.h>
#include <qwt_text.h>
#include <qwt_scale_map.h>
#include <QStaticText>
#include <QFormLayout>
#include <QComboBox>
#include <QLabel>
#include <QSpacerItem>
#include <QPainter>
#include <QDebug>
#include <map>
#include <QPushButton>

#include <QElapsedTimer>

#include <QApplication>

using namespace adiscope;
using namespace adiscope::logic;

AnnotationCurve::AnnotationCurve(std::shared_ptr<logic::Decoder> initialDecoder)
	: GenericLogicPlotCurve(initialDecoder->decoder()->name, initialDecoder->decoder()->id, LogicPlotCurveType::Annotations)
	, m_visibleRows(0)
{
    setSamples(QVector<double>({0.0}), QVector<double>({0.0})),
    setRenderHint(RenderAntialiased, true);
    setBrush(QBrush(Qt::red));
    setBaseline(0.0);

    m_annotationDecoder = new AnnotationDecoder(this, initialDecoder);

    m_bindings.emplace_back(std::make_shared<adiscope::bind::Decoder>(m_annotationDecoder, initialDecoder));

    connect(this, &AnnotationCurve::clicked, [=](const QPointF&p) {
        const auto result = annotationAt(p);
        if (result.isValid()) {
            Q_EMIT annotationClicked(result);
        }
    });
}

AnnotationCurve::~AnnotationCurve()
{
	delete m_annotationDecoder;
}

void AnnotationCurve::annotationCallback(srd_proto_data *pdata, void *annotationCurve)
{
    if (!pdata) {
        qDebug() << "Error pdata nullptr!";
        return;
    }
    if (!annotationCurve) {
        qDebug() << "Error logic_analyzer nullptr!";
        return;
    }

    AnnotationCurve *const curve = static_cast<AnnotationCurve*>(annotationCurve);
    if (!curve) {
        qDebug() << "Error logicAnalyzer nullptr!";
        return;
    }

    if (!pdata->pdo) {
        qDebug() << "Error pdata->pdo nullptr!";
        return;
    }

    if (!pdata->pdo->di) {
        qDebug() << "Error pdata->pdo->di nullptr";
        return;
    }

    const srd_decoder *const decc = pdata->pdo->di->decoder;

    if (!decc) {
        qDebug() << "Error decc nullptr";
        return;
    }

    const srd_proto_data_annotation *const pda =
            (const srd_proto_data_annotation*)pdata->data;

    if (!pda) {
        qDebug() << "Error pda nullptr";
        return;
    }

    // Find format of current received annotation
    const auto format = pda->ann_class;

    // Find the row associated with this format
    const auto r = curve->m_classRows.find(std::make_pair(decc, format));

    auto row_iter = curve->m_annotationRows.end();

    if (r != curve->m_classRows.end()) {
        row_iter = curve->m_annotationRows.find((*r).second);
    } else {
        row_iter = curve->m_annotationRows.find(Row(0, decc));
    }

    if (row_iter == curve->m_annotationRows.end()) {
        qDebug() << "Unexpected annotation for decoder: " << decc << " , format = " << format;
        return;
    }

    std::unique_lock<std::mutex> lock(curve->m_mutex);

    (*row_iter).second.emplace_annotation(pdata, &((*row_iter).first));
//	qDebug() << "Pushed annotation with format: " << format << " to row: " << (*row_iter).first.index();
}

void AnnotationCurve::dataAvailable(uint64_t from, uint64_t to, uint16_t *data)
{
	if (from == 0) {
		reset();
	}

    m_annotationDecoder->dataAvailable(from, to, data);
}

void AnnotationCurve::setClassRows(const std::map<std::pair<const srd_decoder *, int>, Row> &classRows)
{
    m_classRows = classRows;
}

void AnnotationCurve::setAnnotationRows(const std::map<Row, RowData> &annotationRows)
{
    m_annotationRows = annotationRows;
}

const std::map<Row, RowData> & AnnotationCurve::getAnnotationRows() const
{
    return m_annotationRows;
}

void AnnotationCurve::sort_rows()
{
    for (auto it = m_annotationRows.begin(); it != m_annotationRows.end(); ++it) {
        it->second.sort_annotations();
    }
}

void AnnotationCurve::newAnnotations()
{
    QMetaObject::invokeMethod(plot(), "replot");
    state = 1;
}

void AnnotationCurve::reset()
{
		std::unique_lock<std::mutex> lock(m_mutex);

    m_classRows.clear();
    m_annotationRows.clear();
	m_annotationDecoder->reset();
	m_visibleRows = 0;
	state = 0;
}

QWidget *AnnotationCurve::getCurrentDecoderStackMenu()
{
    // for each decoder append it s binding
    std::vector<std::shared_ptr<logic::Decoder>> stack = m_annotationDecoder->getDecoderStack();

    QWidget *widget = new QWidget();
    QGridLayout *grid = new QGridLayout();
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setColumnStretch(0, 1);
    widget->setLayout(grid);
    QFormLayout *layout = new QFormLayout();
    layout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    layout->setFormAlignment(Qt::AlignTop);
    layout->setLabelAlignment(Qt::AlignLeft);
    grid->addLayout(layout, 1, 0, 1, 2);


    auto getSubTitleFrame = [=](){
	    QFrame *line = new QFrame();
	    line->setObjectName(QString::fromUtf8("line_4"));
//	    sizePolicy8.setHeightForWidth(line->sizePolicy().hasHeightForWidth());
//	    line->setSizePolicy(sizePolicy8);
	    line->setMaximumSize(QSize(16777215, 1));
	    line->setStyleSheet(QString::fromUtf8("border: 1px solid rgba(255, 255, 255, 70);"));
	    line->setFrameShape(QFrame::HLine);
	    line->setFrameShadow(QFrame::Sunken);

	    return line;
    };

    auto channels = m_annotationDecoder->getDecoderChannels();

    QVBoxLayout *qvbl = new QVBoxLayout();
    QLabel *title = new QLabel(stack.front()->decoder()->name);
    qvbl->addWidget(title);
    qvbl->addWidget(getSubTitleFrame());
    layout->addRow(qvbl);

    for (auto &ch : channels) {
        QWidget *chls = new QWidget(widget);
        QHBoxLayout *lay = new QHBoxLayout(chls);
	lay->setContentsMargins(0, 0, 0, 0);
        chls->setLayout(lay);
        QString required = (ch->is_optional ? "" : "*");
        QLabel *label = new QLabel(ch->name + "(" + ch->desc + ")" + required, chls);
        label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        lay->insertWidget(0, label);
        QComboBox *box = new QComboBox(chls);
	lay->insertSpacerItem(1, new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
	lay->insertWidget(2, box);
        box->addItem(QString("x"));
        for (int i = 0; i < 16; ++i) {
            box->addItem(QString::number(i));
        }

	if (!ch->assigned_signal) {
		box->setCurrentIndex(0);
	} else {
		box->setCurrentIndex(ch->bit_id + 1);
	}

        QObject::connect(box, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index){
           if (index == 0) {
               m_annotationDecoder->unassignChannel(ch->id);
           } else {
               m_annotationDecoder->assignChannel(ch->id, index - 1);
           }
        });
	layout->addRow(label, box);
//        layout->addRow(chls);
    }

    m_bindings.clear();

    for (const auto &dec : stack) {
        if (dec != stack.front()) {
		QVBoxLayout *qvbl = new QVBoxLayout();
		QLabel *title = new QLabel(dec->decoder()->name);
		QHBoxLayout *qhbl = new QHBoxLayout();
		qhbl->addWidget(title);
		qhbl->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
		QPushButton *deleteBtn = new QPushButton();
		deleteBtn->setFlat(true);
		deleteBtn->setIcon(QIcon(":/gui/icons/close_hovered.svg"));
		deleteBtn->setMaximumSize(QSize(16, 16));
		qhbl->addWidget(deleteBtn);
		qvbl->addLayout(qhbl);
		qvbl->addWidget(getSubTitleFrame());
		layout->addRow(qvbl);

		connect(deleteBtn, &QPushButton::clicked, [=](){
			m_classRows.clear();
			m_annotationRows.clear();
			m_annotationDecoder->unstackDecoder(dec);
			Q_EMIT decoderMenuChanged();
		});
        }

	m_bindings.emplace_back(std::make_shared<adiscope::bind::Decoder>(m_annotationDecoder, dec));
	m_bindings.back()->add_properties_to_form(layout, true);
    }

    return widget;
}

void AnnotationCurve::stackDecoder(std::shared_ptr<logic::Decoder> decoder)
{
    m_annotationDecoder->stackDecoder(decoder);

    m_bindings.clear();
    std::vector<std::shared_ptr<logic::Decoder>> stack = m_annotationDecoder->getDecoderStack();
    for (const auto &dec : stack) {
	    m_bindings.emplace_back(std::make_shared<adiscope::bind::Decoder>(m_annotationDecoder, dec));
    }
}

std::vector<std::shared_ptr<adiscope::logic::Decoder> > AnnotationCurve::getDecoderStack()
{
	return m_annotationDecoder->getDecoderStack();
}

int AnnotationCurve::getVisibleRows() const
{
	return m_visibleRows;
}

AnnotationDecoder *AnnotationCurve::getAnnotationDecoder()
{
	return m_annotationDecoder;
}

std::vector<std::shared_ptr<bind::Decoder> > AnnotationCurve::getDecoderBindings()
{
	return m_bindings;
}

void AnnotationCurve::drawLines(QPainter *painter, const QwtScaleMap &xMap, const QwtScaleMap &yMap, const QRectF &canvasRect, int from, int to) const
{
//    qDebug() << "Draw called for annotation curve";

    std::unique_lock<std::mutex> lock(m_mutex);

    QElapsedTimer timer;
    timer.start();

    QwtPointMapper mapper;
    mapper.setFlag( QwtPointMapper::RoundPoints, QwtPainter::roundingAlignment( painter ) );
    mapper.setBoundingRect(canvasRect);

    const QwtInterval interval = plot()->axisInterval(QwtAxis::XBottom);

    QStringList titles;

	painter->save();
	const QString fontFamily = QApplication::font().family();
	painter->setFont(QFont(fontFamily, 10));


    int currentRowOnPlot = 0;
    for (size_t row = 0; row < m_annotationRows.size(); ++row) {
        auto it = std::find_if(m_annotationRows.begin(), m_annotationRows.end(),
                               [row](const std::pair<const Row, RowData> &t) -> bool{
            return t.first.index() == row;
        });

        if (it == m_annotationRows.end()) {
            // Something bad happened here. Might signal wrong classes/row+rowdata
            // assignment when initializing this curve for decoding!!!!
//            Q_ASSERT(false);
            continue;
        }

        // skip rows with no annotations to be drawn
        if (!(*it).second.size()) {
            continue;
        }

        uint64_t start, stop;

	std::tie(start, stop) = (*it).second.get_annotation_subset(fromTimeToSample(interval.minValue()),
								   fromTimeToSample(interval.maxValue()));


        // Get title of this row
        // TODO: Send size to draw annotation in order to avoid drawing text under the title!!!
        // Cache size of row title!!!! This does not change!!!
        QString title = (*it).first.title();
        QSizeF size = QwtText(title).textSize(painter->font());

	titles.push_back(title);

        uint64_t previous_end = -1;
        uint64_t annotations_in_block = 0;
        uint64_t block_start = 0;

        Annotation prev_ann;

        const double min_ann_label_width = QFontMetrics(QFont("Times", 10, QFont::Bold)).horizontalAdvance("XX");

        for (; start <= stop; ++start) {
            Annotation ann = ((*it).second.getAnnAt(start));

	    const double annotation_width = xMap.transform(fromSampleToTime(ann.end_sample())) - xMap.transform(fromSampleToTime(ann.start_sample()));

            bool shouldDraw = false;

            if (annotation_width >= min_ann_label_width) {
                shouldDraw = true;
            }

            const double delta = (previous_end != -1 ?
			xMap.transform(fromSampleToTime(ann.end_sample())) - xMap.transform(fromSampleToTime(previous_end)) :
                        0.0);

            if (qAbs(delta) > 1.5 || shouldDraw) {
                if (annotations_in_block == 1) {
                    drawAnnotation(currentRowOnPlot, prev_ann, painter,
				   xMap, yMap, canvasRect, mapper, interval, size);
                } else if (annotations_in_block > 0) {
                    drawBlock(currentRowOnPlot, block_start, previous_end, painter,
                              xMap, yMap, canvasRect, mapper);
                }

                annotations_in_block = 0;
            }

            if (shouldDraw) {
                drawAnnotation(currentRowOnPlot, ann, painter,
			       xMap, yMap, canvasRect, mapper, interval, size);
                previous_end = -1;
                annotations_in_block = 0;
            } else {
                previous_end = ann.end_sample();
                prev_ann = ann;

                if (!annotations_in_block) {
                    block_start = ann.start_sample();
                }

                annotations_in_block++;
            }

            // Draw final blocks / annotations
            // We need to do this here becaus prev_ann will be a reference
            // to an object created in this for-loop
            if (start == stop) {
                if (annotations_in_block == 1) {
                    drawAnnotation(currentRowOnPlot, prev_ann, painter,
				   xMap, yMap, canvasRect, mapper, interval, size);
                } else if (annotations_in_block > 0) {
                    drawBlock(currentRowOnPlot, block_start, previous_end, painter,
                              xMap, yMap, canvasRect, mapper);
                }
            }
        }

        // Draw next row. There is no need to leave empty row on screen
        // if no annotations are generated for it
        currentRowOnPlot++;
    }
    if (state == 1) {
	    state = 2;
    }

    m_visibleRows = currentRowOnPlot;

	painter->save();

	QColor painterColor;

	if (QIcon::themeName() == "scopy-default") {
		painterColor = Qt::white;
	} else {
		painterColor = Qt::black;
	}

	painter->setPen(QPen(QBrush(painterColor), 2));
	int currentTitleIndex = 0;
	const int pixOffsetFromCanvasMargin = 5;
	for (const QString &title : titles) {
		QSizeF size = QwtText(title).textSize(painter->font());

		double HeightInPoints = yMap.invTransform(m_traceHeight) - yMap.invTransform(0);
		double offset = m_pixelOffset + currentTitleIndex * (HeightInPoints);

		QRectF textRect(QPointF(0.0, 0.0), size);

		textRect.moveCenter(QPointF(xMap.transform(interval.minValue()),
						yMap.transform(offset + HeightInPoints / 2.0)));

		textRect.setBottomLeft(textRect.bottomLeft() + QPointF(size.width() / 2.0 + pixOffsetFromCanvasMargin, 0));
		textRect.setTopRight(textRect.topRight() + QPointF(size.width() / 2.0 + pixOffsetFromCanvasMargin, 0));
		painter->drawText(textRect, title);
		currentTitleIndex++;
	}
	painter->restore();

	painter->restore();
}

void AnnotationCurve::drawBlock(int row, uint64_t start, uint64_t end, QPainter *painter,
                                     const QwtScaleMap &xMap, const QwtScaleMap &yMap,
                                     const QRectF &canvasRect, const QwtPointMapper &mapper) const {

    double HeightInPoints = yMap.invTransform(m_traceHeight) - yMap.invTransform(0);
    double offset = m_pixelOffset + row * (HeightInPoints);

    double width = xMap.transform(fromSampleToTime(end)) - xMap.transform(fromSampleToTime(start));

    const int r = 20 / 4;

    const QRectF rect(QPointF(xMap.transform(fromSampleToTime(start)), yMap.transform(offset)), QSizeF(width, m_traceHeight));

    painter->save();

    painter->setPen(Qt::gray);
    painter->setBrush(QBrush(Qt::gray, Qt::Dense4Pattern));
    painter->drawRoundedRect(rect, r, r);

    painter->restore();
}

QString AnnotationCurve::formatSeconds(double sec) const
{
	QString string;
	auto time = abs(sec);
	if (time > 1) {
		string = QString("%1 s").arg(sec);
	}
	else if (time > 0.001) {
		string = QString("%1 ms").arg(sec * 10E+2);
	}
	else if (time > 0.000001) {
		string = QString("%1 μs").arg(sec * 10E+5);
	}
	else {
		string = QString("%1 ns").arg(sec * 10E+8);
	}

	return string;
}

void AnnotationCurve::drawAnnotationInfo(int row, uint64_t start, uint64_t end, QPainter *painter,
					 const QwtScaleMap &xMap, const QwtScaleMap &yMap,
					 const QRectF &canvasRect) const {

	const double infoHeightInPoints = yMap.invTransform(m_infoHeight) - yMap.invTransform(0);
	const double infoWidthRatio = 0.09;
	double HeightInPoints = yMap.invTransform(m_traceHeight) - yMap.invTransform(0);
	double offset = m_pixelOffset + (row) * (HeightInPoints) - infoHeightInPoints;

	double width = xMap.transform(fromSampleToTime(end)) - xMap.transform(fromSampleToTime(start));

	QRectF rect;
	if (start != end) {
		rect = QRectF(QPointF(xMap.transform(fromSampleToTime(start)), yMap.transform(offset)), QSizeF(width, m_infoHeight));
	} else {
		rect = QRectF(QPointF(canvasRect.x() + canvasRect.width() * infoWidthRatio, yMap.transform(offset)), QSizeF(canvasRect.width() * (1 - infoWidthRatio * 2), m_infoHeight));
	}

	painter->save();

	painter->setPen(Qt::transparent);
	painter->setBrush(QBrush(QColor::fromRgb(0, 0, 20, 150)));

	painter->drawRoundedRect(rect, 5, 5);

	painter->setPen(QPen(QBrush(Qt::white), 1));

	QString info_time = "  start: " + formatSeconds(fromSampleToTime(start));
	QString info_delta_time = "  duration: " + formatSeconds(fromSampleToTime(end) - fromSampleToTime(start));

	painter->drawText(rect, Qt::AlignLeft|Qt::AlignTop, info_time);
	painter->drawText(rect, Qt::AlignLeft|Qt::AlignBottom, info_delta_time);

	QString info_sample = QString("start sample: %1  ").arg(QString::fromStdString(std::to_string(start)));
	QString info_delta_sample = QString("sample count: %1  ").arg(QString::fromStdString(std::to_string(end - start)));

	painter->drawText(rect, Qt::AlignRight|Qt::AlignTop, info_sample);
	painter->drawText(rect, Qt::AlignRight|Qt::AlignBottom, info_delta_sample);
	painter->setPen(QPen(QBrush(Qt::black), 1));

	painter->restore();
}

void  AnnotationCurve::setState(int st)
{
	state = st;
}

int AnnotationCurve::getState()
{
	// -2: decoding failed
	// -1: empty curve
	//  0: started decoding
	//  1: finished decoding
	//  2: finished painting
	return state;
}

QString AnnotationCurve::fromTitleToRowType(QString title) const
{
	return (title.indexOf(':')) ? title.mid(title.indexOf(':') + 1)
				    : title;
}

void AnnotationCurve::drawAnnotation(int row, const Annotation &ann, QPainter *painter,
				     const QwtScaleMap &xMap, const QwtScaleMap &yMap,
				     const QRectF &canvasRect, const QwtPointMapper &mapper,
                     const QwtInterval &interval, const QSizeF &titleSize) const {
//    qDebug() << "Drawing annotation for row: " << row << " having the text " << ann.annotations().back();
    if (ann.start_sample() != ann.end_sample()) {
        drawTwoSampleAnnotation(row, ann, painter,
				xMap, yMap, canvasRect, mapper, interval, titleSize);
    } else {
        drawOneSampleAnnotation(row, ann, painter,
				xMap, yMap, canvasRect, mapper, interval, titleSize);
    }
}

void AnnotationCurve::fillAnnotationCurve(int row, uint32_t annClass, QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    const QRectF &canvasRect, QPolygonF &polygon ) const
{
    if ( brush().style() == Qt::NoBrush )
        return;

    closePolyline(row, annClass, painter, xMap, yMap, polygon );
    if ( polygon.count() <= 2 ) // a line can't be filled
        return;

    QBrush br = brush();
    QColor color;
    const int h = (55 * annClass) % 360;
    const int s = 180;
    const int v = 170;
    color.setHsl(h, s, v);
    br.setColor(color);



    painter->save();

    painter->setPen( Qt::NoPen );
    painter->setBrush( br );

    QwtPainter::drawPolygon( painter, polygon );

    painter->restore();
}

void AnnotationCurve::closePolyline(int row, uint32_t annClass, QPainter *painter, const QwtScaleMap &xMap,
                                    const QwtScaleMap &yMap, QPolygonF &polygon) const
{
    if ( polygon.size() < 2 )
        return;

    const bool doAlign = QwtPainter::roundingAlignment( painter );

    double HeightInPoints = yMap.invTransform(m_traceHeight) - yMap.invTransform(0);
    double baseline = m_pixelOffset + row * (HeightInPoints) + HeightInPoints / 2.0;

    if ( orientation() == Qt::Vertical )
    {
        if ( yMap.transformation() )
            baseline = yMap.transformation()->bounded( baseline );

        double refY = yMap.transform( baseline );
        if ( doAlign )
            refY = qRound( refY );

        polygon += QPointF( polygon.last().x(), refY );
        polygon += QPointF( polygon.first().x(), refY );
    }
    else
    {
        if ( xMap.transformation() )
            baseline = xMap.transformation()->bounded( baseline );

        double refX = xMap.transform( baseline );
        if ( doAlign )
            refX = qRound( refX );

        polygon += QPointF( refX, polygon.last().y() );
        polygon += QPointF( refX, polygon.first().y() );
    }
}

uint64_t AnnotationCurve::getMaxAnnotationCount(int index)
{
	std::map<Row, RowData>::iterator it;
	uint64_t count = 0;
	std::map<Row, RowData> decoder(getAnnotationRows());

	for (it = m_annotationRows.begin(); it != m_annotationRows.end(); it++) {
		auto temp_annotations = it->second.get_annotations();
		if (it->first.index() == index) {
			return temp_annotations.size();
		}
		if (count < temp_annotations.size()) {
			count = temp_annotations.size();
		}
	}

	return count;
}

void AnnotationCurve::drawTwoSampleAnnotation(int row, const Annotation &ann, QPainter *painter,
					      const QwtScaleMap &xMap, const QwtScaleMap &yMap,
					      const QRectF &canvasRect, const QwtPointMapper &mapper,
                          const QwtInterval &interval, const QSizeF &titleSize) const
{
    // DRAW MULTI POINT ANNOTATION

    double first = fromSampleToTime(ann.start_sample());
    double last = fromSampleToTime(ann.end_sample());

    double firstPlus1Px = xMap.invTransform(xMap.transform(first) + 3);
    double lastMinus1Px = xMap.invTransform(xMap.transform(last) - 3);

    double HeightInPoints = yMap.invTransform(m_traceHeight) - yMap.invTransform(0);
    double offset = m_pixelOffset + row * (HeightInPoints);

    QVector<QPointF> displayedData;
    displayedData += QPointF(first, offset + HeightInPoints / 2.0);
    displayedData += QPointF(firstPlus1Px, offset);
    displayedData += QPointF(lastMinus1Px, offset);
    displayedData += QPointF(last, offset + HeightInPoints / 2.0);
    displayedData += QPointF(lastMinus1Px, offset + HeightInPoints);
    displayedData += QPointF(firstPlus1Px, offset + HeightInPoints);
    displayedData += QPointF(first, offset + HeightInPoints / 2.0);

    QwtPointSeriesData *d = new QwtPointSeriesData(displayedData);
    QPolygonF polyline = mapper.toPolygonF(xMap, yMap, d, 0, displayedData.size() - 1);

    delete d;

    fillAnnotationCurve(row, ann.ann_class(), painter, xMap, yMap, canvasRect, polyline);

    QwtPainter::drawPolyline(painter, polyline);

    // END DRAW MULTI POINT annotation

    // DRAW LABEL
    const double bonus = xMap.invTransform(titleSize.width() + 5) - xMap.invTransform(0);

    if (interval.minValue() + bonus > displayedData[0].x()) {
        displayedData[0].setX(interval.minValue() + bonus);
    }

    if (interval.maxValue() < displayedData[3].x()) {
        displayedData[3].setX(interval.maxValue());
    }

    const double maxWidth = xMap.transform(displayedData[3].x()) - xMap.transform(displayedData[0].x());
    if (maxWidth <= 0) {
        return; // There is no room to draw text
    }

    QPointF drawTextPoint = (displayedData[3] + displayedData[0]) / 2.0;

    double x = xMap.transform(drawTextPoint.x());
    double y = yMap.transform(drawTextPoint.y());

    painter->save();
    painter->setPen(QPen(QBrush(Qt::black), 20));

    // TODO: add text that corresponds to given annotation (maybe pass Annotation instead of first/last)
    // See in list of strings available for annotation which one fits due to plotting size. Also if there is space
    // draw the text centered if the annotation goes beyond plot limits, make it fit the visible area (left/right shift
    // from center)
    QSizeF size = QwtText("").textSize(painter->font());
    QString text = "";
    for (auto it = ann.annotations().begin(); it != ann.annotations().end(); ++it) {
        QString str = *it;
        QSizeF sz = QwtText(str).textSize(painter->font());
        if (sz.width() < maxWidth) {
            text = str;
            size = sz;
            break; // found something
        }
    }


    QRectF textRect(QPointF(0.0, 0.0), size);
    textRect.moveCenter(QPointF(x, y));

    painter->drawText(textRect, text);

    painter->restore();

    // END DRAW LABEL

}

void AnnotationCurve::drawOneSampleAnnotation(int row, const Annotation &ann, QPainter *painter,
					      const QwtScaleMap &xMap, const QwtScaleMap &yMap,
					      const QRectF &canvasRect, const QwtPointMapper &mapper,
                          const QwtInterval &interval, const QSizeF &titleSize) const {
    double xx = xMap.transform(fromSampleToTime(ann.start_sample()));
    // 20 px
    double HeightInPoints = yMap.invTransform(m_traceHeight) - yMap.invTransform(0);
    double yy = yMap.transform(m_pixelOffset + row * (HeightInPoints) + HeightInPoints / 2.0);
/*    double w = xMap.transform(6);
    double h = yMap.transform(6); */// TODO: compute height to fit class text and
                                    // compute text font to fit height (balance between this two rules)

    QBrush br = brush();
    QColor color;
    const int h = (55 * ann.ann_class()) % 360;
    const int s = 180;
    const int v = 170;
    color.setHsl(h, s, v);
    br.setColor(color);

    painter->save();
    painter->setBrush(br);
    painter->drawEllipse(QPointF(xx, yy), m_traceHeight / 2.0, m_traceHeight / 2.0);
    painter->restore();


    painter->save();
    painter->setPen(QPen(QBrush(Qt::black), 20));

    const double bonus = xMap.invTransform(titleSize.width() + 5) - xMap.invTransform(0);
    const double WidthInPoints = xMap.invTransform(m_traceHeight) - xMap.invTransform(0);

    double x1 = fromSampleToTime(ann.start_sample()) - WidthInPoints / 2.0;
    double x2 = fromSampleToTime(ann.start_sample()) + WidthInPoints / 2.0;

    if (interval.minValue() + bonus > x1) {
	x1 = interval.minValue() + bonus;
    }

    if (interval.maxValue() < x2) {
	x2 = interval.maxValue();
    }

    QSizeF size = QwtText("").textSize(painter->font());
    QString text = "";

    const double maxWidth = (xMap.transform(x2) - xMap.transform(x1)) / 2.0;
    for (auto it = ann.annotations().begin(); it != ann.annotations().end(); ++it) {
	QString str = *it;
	QSizeF sz = QwtText(str).textSize(painter->font());

	if (sz.width() < maxWidth) {
	    text = str;
	    size = sz;
	    break; // found something
	}
    }

    QRectF textRect(QPointF(0.0, 0.0), size);
    textRect.moveCenter(QPointF(xx, yy));

    painter->drawText(textRect, text);

    painter->restore();

}

AnnotationQueryResult AnnotationCurve::annotationAt(const QPointF& p) const
{
    if (m_visibleRows == 0) return {0, nullptr};

    const auto plt = plot();
    if (plt == nullptr) return {0, nullptr};
    const uint64_t sample = fromTimeToSample(p.x());

    const auto &ymap = plt->canvasMap(yAxis().pos);
    // NOTE: Row height is negative
    const double rowHeight = ymap.invTransform(m_traceHeight) - ymap.invTransform(0);

    int empty_ann_count = 0;
    for (int index = 0; index <= m_annotationRows.size(); index ++) {
	    for (const auto &entry: m_annotationRows) {
		    if (entry.first.index() != index) continue;
		    const Row &row = entry.first;
		    const RowData &data = entry.second;

		    const auto maxY = m_pixelOffset + (row.index() - empty_ann_count) * rowHeight;
		    const auto minY = maxY + rowHeight;

		    const auto y = p.y();
		    if (data.size() == 0) {
			    empty_ann_count ++;
			    continue;
		    }
		    if (y < minY or y > maxY) continue;
		    uint64_t prev_end_sample = 0, next_start_sample = 0;
		    for (uint64_t i = 0; i < data.size(); i++) {
			    const Annotation *ann = data.annAt(i);
			    next_start_sample = (i + 1 == data.size()) ? sample + (sample - prev_end_sample)/2
								       : data.annAt(i + 1)->start_sample();
			    if (ann->end_sample() - ann->start_sample() < 2 &&
					    sample + (sample - prev_end_sample)/2 >= ann->start_sample() and sample - (next_start_sample - sample)/2 <= ann->end_sample()) {
				    return {i, ann};
			    }
			    if (sample >= ann->start_sample() and sample <= ann->end_sample()) {
				    return {i, ann};
			    }
			    prev_end_sample = ann->end_sample();
		    }
	    }
    }
    return {0, nullptr};
}

bool AnnotationCurve::testHit(const QPointF& p) const
{
    return annotationAt(p).isValid();
}

#include "moc_annotationcurve.cpp"
