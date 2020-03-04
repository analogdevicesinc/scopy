#include "annotationcurve.h"

#include "annotationdecoder.h"

#include "binding/decoder.hpp"

#include "logic_analyzer.h"

#include <qwt_plot.h>
#include <qwt_painter.h>
#include <qwt_painter.h>
#include <qwt_series_data.h>
#include <qwt_text.h>
#include <QStaticText>
#include <QFormLayout>
#include <QComboBox>
#include <QLabel>

#include <QElapsedTimer>

using namespace adiscope;
using namespace adiscope::logic;

AnnotationCurve::AnnotationCurve(logic::LogicAnalyzer *logic, std::shared_ptr<logic::Decoder> initialDecoder):
    QwtPlotCurve(),
    m_pixelOffset(0),
    m_traceHeight(0)
{
    setSamples({0.0}, {0.0});
    setRenderHint(RenderAntialiased, true);
    setBrush(QBrush(Qt::red));
    setBaseline(0.0);

    m_annotationDecoder = new AnnotationDecoder(this, initialDecoder, logic);
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
    //    qDebug() << "Pushed annotation with format: " << format << " to row: " << (*row_iter).first.index();
}

void AnnotationCurve::newData(uint64_t from, uint64_t to)
{
    m_annotationDecoder->newData(from, to);
}

void AnnotationCurve::setClassRows(const map<std::pair<const srd_decoder *, int>, Row> &classRows)
{
    m_classRows = classRows;
}

void AnnotationCurve::setAnnotationRows(const map<const Row, RowData> &annotationRows)
{
    m_annotationRows = annotationRows;
}

double AnnotationCurve::getPixelOffset() const
{
    return m_pixelOffset;
}

double AnnotationCurve::getTraceHeight() const
{
    return m_traceHeight;
}

void AnnotationCurve::setPixelOffset(double pixelOffset)
{
    // TODO: maybe trigger something when pixelOffset
    // changes
    if (m_pixelOffset != pixelOffset) {
            m_pixelOffset = pixelOffset;
    }
    setBaseline(m_pixelOffset + m_traceHeight);
}

void AnnotationCurve::setTraceHeight(double traceHeight)
{
    // TODO: maybe trigger something when traceHeight
    // changes
    if (m_traceHeight != traceHeight) {
            m_traceHeight = traceHeight;
    }
    setBaseline(m_pixelOffset + m_traceHeight);
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
}

void AnnotationCurve::clear()
{
    m_classRows.clear();
    m_annotationRows.clear();
}

QWidget *AnnotationCurve::getCurrentDecoderStackMenu()
{
    // for each decoder append it s binding
    std::vector<std::shared_ptr<logic::Decoder>> stack = m_annotationDecoder->getDecoderStack();

    // delete old layout
//    if (m_menuLayout) {
//        QWidget *temp = new QWidget();
//        temp->setLayout(m_menuLayout);
//        temp->deleteLater();
//    }

    QWidget *widget = new QWidget();
    QFormLayout *layout = new QFormLayout(widget);
    widget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    auto channels = m_annotationDecoder->getDecoderChannels();

    QLabel *title = new QLabel(stack.front()->decoder()->name);
    layout->addRow(title);

    for (auto &ch : channels) {
        QWidget *chls = new QWidget(widget);
        QHBoxLayout *lay = new QHBoxLayout(chls);
        chls->setLayout(lay);
        QString required = (ch->is_optional ? "" : "*");
        QLabel *label = new QLabel(ch->name + "(" + ch->desc + ")" + required, chls);
        label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        lay->insertWidget(0, label);
        QComboBox *box = new QComboBox(chls);
        lay->insertWidget(1, box);
        box->addItem(QString("x"));
        for (int i = 0; i < 16; ++i) {
            box->addItem(QString::number(i));
        }

        box->setCurrentIndex(0);

        QObject::connect(box, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index){
           if (index == 0) {
               m_annotationDecoder->unassignChannel(ch->id);
           } else {
               m_annotationDecoder->assignChannel(ch->id, index - 1);
           }
        });

        layout->addRow(chls);
    }

    for (const auto &dec : stack) {
        if (dec != stack.front()) {
            QLabel *title = new QLabel(dec->decoder()->name);
            layout->addRow(title);
        }
	m_bindings.emplace_back(std::make_shared<adiscope::bind::Decoder>(m_annotationDecoder, dec));
        m_bindings.back()->add_properties_to_form(layout, true);
    }

    return widget;
}

void AnnotationCurve::stackDecoder(std::shared_ptr<logic::Decoder> decoder)
{
    m_annotationDecoder->stackDecoder(decoder);
}

std::vector<std::shared_ptr<adiscope::logic::Decoder> > AnnotationCurve::getDecoderStack()
{
    return m_annotationDecoder->getDecoderStack();
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

    auto interval = plot()->axisInterval(QwtAxis::xBottom);

    int currentRowOnPlot = 0;
    for (int row = 0; row < m_annotationRows.size(); ++row) {
        auto it = std::find_if(m_annotationRows.begin(), m_annotationRows.end(),
                               [row](const std::pair<const Row, RowData> &t) -> bool{
            return t.first.index() == row;
        });

        if (it == m_annotationRows.end()) {
            // Something bad happened here. Might signal wrong classes/row+rowdata
            // assignment when initializing this curve for decoding!!!!
            Q_ASSERT(false);
            continue;
        }

        // skip rows with no annotations to be drawn
        if (!(*it).second.size()) {
            continue;
        }

        uint64_t start, stop;

        std::tie(start, stop) = (*it).second.get_annotation_subset(interval.minValue(), interval.maxValue());


        // Get title of this row
        // TODO: Send size to draw annotation in order to avoid drawing text under the title!!!
        // Cache size of row title!!!! This does not change!!!
        QString title = (*it).first.title();
        QSizeF size = QwtText(title).textSize(painter->font());

        // TODO: draw blocks for annotations that are too close
        QElapsedTimer tttt;
        tttt.start();

        uint64_t previous_end = -1;
        uint64_t annotations_in_block = 0;
        uint64_t block_start = 0;

        Annotation prev_ann;

        const double min_ann_label_width = QFontMetrics(QFont("Times", 10, QFont::Bold)).horizontalAdvance("XX");

        for (; start <= stop; ++start) {
            Annotation ann = ((*it).second.getAnnAt(start));

            const double annotation_width = xMap.transform(ann.end_sample()) - xMap.transform(ann.start_sample());

            bool shouldDraw = false;

            if (annotation_width >= min_ann_label_width) {
                shouldDraw = true;
            }

            const double delta = (previous_end != -1 ?
                        xMap.transform(ann.end_sample()) - xMap.transform(previous_end) :
                        0.0);

            if (qAbs(delta) > 1.5 || shouldDraw) {
                if (annotations_in_block == 1) {
                    drawAnnotation(currentRowOnPlot, prev_ann, painter,
                                   xMap, yMap, canvasRect, mapper);
                } else if (annotations_in_block > 0) {
                    drawBlock(currentRowOnPlot, block_start, previous_end, painter,
                              xMap, yMap, canvasRect, mapper);
                }

                annotations_in_block = 0;
            }

            if (shouldDraw) {
                drawAnnotation(currentRowOnPlot, ann, painter,
                               xMap, yMap, canvasRect, mapper);
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
                                   xMap, yMap, canvasRect, mapper);
                } else if (annotations_in_block > 0) {
                    drawBlock(currentRowOnPlot, block_start, previous_end, painter,
                              xMap, yMap, canvasRect, mapper);
                }
            }
        }

        painter->save();
        painter->setPen(QPen(QBrush(Qt::black), 20));
        painter->setFont(QFont("Times", 10, QFont::Bold));

        double HeightInPoints = yMap.invTransform(30) - yMap.invTransform(10);
        double offset = m_pixelOffset + currentRowOnPlot * (HeightInPoints);
        QRectF textRect(QPointF(0.0, 0.0), size);

        textRect.moveBottomLeft(QPointF(xMap.transform(interval.minValue()), yMap.transform(offset + (HeightInPoints) / 2.0)));

        painter->drawText(textRect, title);

        painter->restore();

        // Draw next row. There is no need to leave empty row on screen
        // if no annotations are generated for it
        currentRowOnPlot++;
    }
}

void AnnotationCurve::drawBlock(int row, uint64_t start, uint64_t end, QPainter *painter,
                                     const QwtScaleMap &xMap, const QwtScaleMap &yMap,
                                     const QRectF &canvasRect, const QwtPointMapper &mapper) const {

    double HeightInPoints = yMap.invTransform(30) - yMap.invTransform(10);
    double offset = m_pixelOffset + row * (HeightInPoints);

    double width = xMap.transform(end) - xMap.transform(start);

    const int r = 20 / 4;

    const QRectF rect(QPointF(xMap.transform(start), yMap.transform(offset) - 10), QSizeF(width, 20));

    painter->save();

    painter->setPen(Qt::gray);
    painter->setBrush(QBrush(Qt::gray, Qt::Dense4Pattern));
    painter->drawRoundedRect(rect, r, r);

    painter->restore();
}

void AnnotationCurve::drawAnnotation(int row, const Annotation &ann, QPainter *painter,
                                     const QwtScaleMap &xMap, const QwtScaleMap &yMap,
                                     const QRectF &canvasRect, const QwtPointMapper &mapper) const {
//    qDebug() << "Drawing annotation for row: " << row << " having the text " << ann.annotations().back();
    if (ann.start_sample() != ann.end_sample()) {
        drawTwoSampleAnnotation(row, ann, painter,
                                xMap, yMap, canvasRect, mapper);
    } else {
        drawOneSampleAnnotation(row, ann, painter,
                                xMap, yMap, canvasRect, mapper);
    }
}

void AnnotationCurve::fillCurve(int row, uint32_t annClass, QPainter *painter,
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

    double HeightInPoints = yMap.invTransform(30) - yMap.invTransform(10);
    double baseline = m_pixelOffset + row * (HeightInPoints);

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

void AnnotationCurve::drawTwoSampleAnnotation(int row, const Annotation &ann, QPainter *painter,
                                              const QwtScaleMap &xMap, const QwtScaleMap &yMap,
                                              const QRectF &canvasRect, const QwtPointMapper &mapper) const
{
    // DRAW MULTI POINT ANNOTATION

    double first = ann.start_sample();
    double last = ann.end_sample();

    double firstPlus1Px = xMap.invTransform(xMap.transform(first) + 3);
    double lastMinus1Px = xMap.invTransform(xMap.transform(last) - 3);

    // convert 20px into plot data.
    double HeightInPoints = yMap.invTransform(30) - yMap.invTransform(10);
    double offset = m_pixelOffset + row * (HeightInPoints);

    QVector<QPointF> displayedData;
    displayedData += QPointF(first, offset);
    displayedData += QPointF(firstPlus1Px, offset - HeightInPoints / 2.0);
    displayedData += QPointF(lastMinus1Px, offset - HeightInPoints / 2.0);
    displayedData += QPointF(last, offset);
    displayedData += QPointF(lastMinus1Px, offset + HeightInPoints / 2.0);
    displayedData += QPointF(firstPlus1Px, offset + HeightInPoints / 2.0);
    displayedData += QPointF(first, offset);

    QwtPointSeriesData *d = new QwtPointSeriesData(displayedData);
    QPolygonF polyline = mapper.toPolygonF(xMap, yMap, d, 0, displayedData.size() - 1);

    delete d;

    fillCurve(row, ann.ann_class(), painter, xMap, yMap, canvasRect, polyline);

    QwtPainter::drawPolyline(painter, polyline);

    // END DRAW MULTI POINT annotation

    // DRAW LABEL
    QwtInterval interval = plot()->axisInterval(QwtAxis::xBottom);
    if (interval.minValue() > displayedData[0].x()) {
        displayedData[0].setX(interval.minValue());
    }

    if (interval.maxValue() < displayedData[3].x()) {
        displayedData[3].setX(interval.maxValue());
    }

    QPointF drawTextPoint = (displayedData[3] + displayedData[0]) / 2.0;

    double x = xMap.transform(drawTextPoint.x());
    double y = yMap.transform(drawTextPoint.y());

    painter->save();
    painter->setPen(QPen(QBrush(Qt::black), 20));
    painter->setFont(QFont("Times", 10, QFont::Bold));

    // TODO: add text that corresponds to given annotation (maybe pass Annotation instead of first/last)
    // See in list of strings available for annotation which one fits due to plotting size. Also if there is space
    // draw the text centered if the annotation goes beyond plot limits, make it fit the visible area (left/right shift
    // from center)
    QSizeF size = QwtText("").textSize(painter->font());
    QString text = "";

    double maxWidth = xMap.transform(displayedData[3].x()) - xMap.transform(displayedData[0].x());
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
                                              const QRectF &canvasRect, const QwtPointMapper &mapper) const {
    double xx = xMap.transform(ann.start_sample());
    // 20 px
    double HeightInPoints = yMap.invTransform(30) - yMap.invTransform(10);
    double yy = yMap.transform(m_pixelOffset + row * (HeightInPoints));
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
    painter->drawEllipse(QPointF(xx, yy), 10, 10);
    painter->restore();


    painter->save();
    painter->setPen(QPen(QBrush(Qt::black), 20));
    painter->setFont(QFont("Times", 10, QFont::Bold));

//    QSizeF size = QwtText("").textSize(painter->font());
//    QString text = "";
//    double maxWidth = xMap.transform(displayedData[3].x()) - xMap.transform(displayedData[0].x());
//    for (auto it = ann.annotations().rbegin(); it != ann.annotations().rend(); ++it) {
//        QString str = *it;
//        QSizeF sz = QwtText(str).textSize(painter->font());
//        qDebug() << "For text: " << str << " we need QSize: " << sz;
//        if (sz.width() < maxWidth) {
//            text = str;
//            size = sz;
//            break; // found something
//        }
//    }

//    qDebug() << "Will draw text: " << text << " that fits: " << maxWidth;
    QSizeF size = QwtText(ann.annotations().back()).textSize(painter->font());

    QRectF textRect(QPointF(0.0, 0.0), size);
    textRect.moveCenter(QPointF(xx, yy));

    painter->drawText(textRect, ann.annotations().back());

    painter->restore();

}
