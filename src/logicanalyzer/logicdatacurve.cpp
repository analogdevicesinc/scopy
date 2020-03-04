#include "logicdatacurve.h"

#include <QElapsedTimer>

#include <qwt_plot.h>
#include <qwt_painter.h>
#include <qwt_point_mapper.h>
#include <qwt_painter.h>
#include <qwt_series_data.h>

static const QColor EdgeColor(0x80, 0x80, 0x80);
static const QColor HighColor(0x00, 0xC0, 0x00);
static const QColor LowColor(0xC0, 0x00, 0x00);
static const QColor SamplingPointColor(0x77, 0x77, 0x77);

LogicDataCurve::LogicDataCurve(uint16_t *data, uint8_t bit, adiscope::logic::LogicAnalyzer *logic) :
    QwtPlotCurve(),
    m_logic(logic),
    m_startSample(0),
    m_endSample(0),
    m_bit(bit),
    m_pixelOffset(0),
    m_traceHeight(0)
{
    // If there are no set samples, QwtPlot::replot() won't call our
    // draw() method. Trick the plot into thinking that we have data
    // set on our curve.

    // TODO: maybe custom plot with reimplemented replot() ?

    setSamples({0.0}, {0.0});
    setBrush(QBrush(Qt::black));
}

void LogicDataCurve::dataAvailable(uint64_t from, uint64_t to)
{
    m_data = m_logic->getData();

    // Take into account the last pushed edge from the previous chunk of
    // available data
    uint64_t currentSample = from;
    if (m_edges.size()) {
        if (m_edges.back().first + 1 < currentSample) {
            currentSample = m_edges.back().first + 1;
        }
    }

    for (; currentSample < to - 1; ++currentSample) {
        bool transition = (m_data[currentSample] & (1 << m_bit)) ^ (m_data[currentSample + 1] & (1 << m_bit));
        bool high = (m_data[currentSample] & (1 << m_bit)) > (m_data[currentSample + 1] & (1 << m_bit));
        if (transition) {
            m_edges.emplace_back(currentSample, high);
        }
    }

    m_endSample = to;
}

void LogicDataCurve::reset()
{

}

double LogicDataCurve::getPixelOffset() const
{
    return m_pixelOffset;
}

double LogicDataCurve::getTraceHeight() const
{
    return m_traceHeight;
}

void LogicDataCurve::setPixelOffset(double pixelOffset)
{
    // TODO: maybe trigger something when pixelOffset
    // changes
    if (m_pixelOffset != pixelOffset) {
            m_pixelOffset = pixelOffset;
    }
    setBaseline(m_pixelOffset + m_traceHeight);
}

void LogicDataCurve::setTraceHeight(double traceHeight)
{
    // TODO: maybe trigger something when traceHeight
    // changes
    if (m_traceHeight != traceHeight) {
            m_traceHeight = traceHeight;
    }
    setBaseline(m_pixelOffset + m_traceHeight);
}

void LogicDataCurve::drawLines(QPainter *painter, const QwtScaleMap &xMap,
                               const QwtScaleMap &yMap, const QRectF &canvasRect,
                               int from, int to) const
{



    // No data to plot
    if (!m_edges.size()) {
        return;
    }

    std::vector<std::pair<uint64_t, bool>> edges;
    getSubsampledEdges(edges, xMap);

    if (!edges.size()) {
        return;
    }

    QwtPointMapper mapper;
    mapper.setFlag( QwtPointMapper::RoundPoints, QwtPainter::roundingAlignment( painter ) );
    mapper.setBoundingRect(canvasRect);

    QVector<QPointF> displayedData;

    if (edges.front().first > 0) {
        displayedData += QPointF(0, edges.front().second * m_traceHeight + m_pixelOffset);
    }

    for (const auto & edge : edges) {
        double y1 = edge.second * m_traceHeight + m_pixelOffset;
        double y2 = !edge.second * m_traceHeight + m_pixelOffset;

        displayedData += QPointF(edge.first, y1);
        displayedData += QPointF(edge.first + 1, y1);
        displayedData += QPointF(edge.first + 1, y2);
    }

    if (edges.back().first + 1 < m_endSample - 1) {
        displayedData += QPointF(m_endSample - 1, (!edges.back().second) * m_traceHeight + m_pixelOffset);
    }

    QwtPointSeriesData *d = new QwtPointSeriesData(displayedData);
    QPolygonF polyline = mapper.toPolygonF(xMap, yMap, d, 0, displayedData.size() - 1);
    QwtPainter::drawPolyline(painter, polyline);

    delete d;


    // Draw sampling points
    // Optimize for each segment we can draw the points connecting it
    // knowing from segment.second if it is "1" or "0"
    double dist = xMap.transform(1) - xMap.transform(0);

    if (dist <= 4.0) {
        return;
    }

    QwtInterval interval = plot()->axisInterval(QwtAxis::xBottom);

    int start = interval.minValue();
    int end = interval.maxValue();

    start = start < 0 ? 0 : start;
    end = end > (m_endSample - 1) ? (m_endSample - 1) : end;

    QVector<QPointF> points;
    for (; start <= end; ++start) {
        double y = ((m_logic->getData()[start] & (1 << m_bit)) >> m_bit) * m_traceHeight + m_pixelOffset;
        points += QPointF(start, y);
    }

    QwtPointSeriesData *d2 = new QwtPointSeriesData(points);
    QPolygonF points_poly = mapper.toPointsF(xMap, yMap, d2, 0, points.size() - 1);

    delete d2;

    painter->save();

    painter->setPen(QPen(QBrush(SamplingPointColor), 3));
    QwtPainter::drawPoints( painter, points_poly );

    painter->restore();

//    qDebug() << "Drawing took: " << t.elapsed();

}

void LogicDataCurve::getSubsampledEdges(std::vector<std::pair<uint64_t, bool>> &edges, const QwtScaleMap &xMap) const {



    double dist = xMap.transform(1) - xMap.transform(0);

    QwtInterval interval = plot()->axisInterval(QwtAxis::xBottom);
    uint64_t firstEdge = edgeAtX(interval.minValue(), m_edges);
    uint64_t lastEdge = edgeAtX(interval.maxValue(), m_edges);

    if (firstEdge > 0) {
        firstEdge--;
    }

    if (lastEdge < m_edges.size() - 1) {
        lastEdge++;
    }

    // If plot is zoomed in / not so many edges close together
    // draw them all
    if (dist > 0.10) {
        for (; firstEdge < lastEdge; ++firstEdge) {
            edges.emplace_back(m_edges[firstEdge]);
        }
    } else {

        const uint64_t pointsPerPixel = 1.0 / dist;

        // Save last transition (high, low)
        bool lastTransition = m_edges[firstEdge].second;
        for (; firstEdge < lastEdge; ) {
            const int64_t lastSample = m_edges[firstEdge].first;
            edges.emplace_back(m_edges[firstEdge]);

            // Find the next edge that is at least "pointsPerPixel" away
            // from the current one
            auto next = std::upper_bound(m_edges.begin(), m_edges.end(),
                        std::make_pair(edges.back().first + pointsPerPixel - 1, false),
                        [=](const std::pair<uint64_t, bool> &lhs, const std::pair<uint64_t, bool> &rhs) -> bool {
                return lhs.first < rhs.first;
            });


            auto previous = next;
            std::advance(previous, -1);

//            // If we reached the end just insert the last edge from the subset
//            if (next == m_edges.end()) {
//                edges.emplace_back(m_edges[lastEdge]);
//                break;
//            }

            const int64_t a1 = (*next).first;
            const int64_t a2 = (*previous).first;

            // If the distance between next found edge and it's predecessor is greater
            // than the nr of samples/1px draw both edges as there will be a visible gap
            // between the blocks and we want to make sure we display the correct logic level
            if (std::abs(a1 - a2) > pointsPerPixel) {
                edges.emplace_back(m_edges[std::distance(m_edges.begin(), previous)]);
//                qDebug() << "gap emplace!!!!!!!";
            } else {
                const int64_t currentSample = (*next).first;
                if ((*next).second == lastTransition) {
                    edges.emplace_back((lastSample + currentSample) / 2, !lastTransition);
//                    qDebug() << "HACK!!!!!!";
                }
            }

//            if (edges.back().second == (*next).second) {
//                edges.emplace_back(m_edges[lastEdge]);
//            }

            firstEdge = std::distance(m_edges.begin(), next);

            lastTransition = (*next).second;
        }
    }
}

uint64_t LogicDataCurve::edgeAtX(int x, const std::vector<std::pair<uint64_t, bool>> &edges) const {
    // returns position of edge close to x value
    // O(log N)

    int64_t start = 0;
    int64_t end = edges.size();

    int64_t mid = 0;

    while (end >= start) {
        mid = start + (end - start) / 2;

        if (edges[mid].first < x) {
            start = mid + 1;
        } else if (edges[mid].first > x) {
            end = mid - 1;
        } else {
            return mid;
        }
    }

    return mid;
}

