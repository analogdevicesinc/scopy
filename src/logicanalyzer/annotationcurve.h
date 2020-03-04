#ifndef ANNOTATIONCURVE_H
#define ANNOTATIONCURVE_H

#include <qwt_plot_curve.h>
#include <qwt_point_mapper.h>

#include <map>

#include <libsigrokdecode/libsigrokdecode.h>

#include <memory>
#include <mutex>

#include <QWidget>

#include "annotation.h"
#include "row.h"
#include "rowdata.h"

namespace adiscope {
namespace logic {
class LogicAnalyzer;
class Decoder;
}
}


namespace adiscope {
namespace bind {
class Decoder;
}
}

namespace adiscope {

class AnnotationDecoder;

class AnnotationCurve : public QwtPlotCurve
{
public:
	AnnotationCurve(logic::LogicAnalyzer *logic, std::shared_ptr<logic::Decoder> initialDecoder);

public:
    static void annotationCallback(srd_proto_data *pdata, void *annotationCurve);

    void newData(uint64_t from, uint64_t to);

    void setClassRows(const std::map<std::pair<const srd_decoder*, int>, Row> &classRows);
    void setAnnotationRows(const std::map<const Row, RowData> &annotationRows);

    double getPixelOffset() const;
    double getTraceHeight() const;

    void setPixelOffset(double pixelOffset);
    void setTraceHeight(double traceHeight);

    void sort_rows();

    void newAnnotations();

    void clear();

    QWidget * getCurrentDecoderStackMenu();
	void stackDecoder(std::shared_ptr<adiscope::logic::Decoder> decoder);
	std::vector<std::shared_ptr<adiscope::logic::Decoder>> getDecoderStack();

protected:
    void drawLines( QPainter *painter,
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        const QRectF &canvasRect, int from, int to ) const override;

private:
    void fillCurve(int row, uint32_t annClass, QPainter *painter, const QwtScaleMap &xMap,
                   const QwtScaleMap &yMap, const QRectF &canvasRect,
                   QPolygonF &polygon) const;

    void closePolyline(int row, uint32_t annClass, QPainter *painter,
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        QPolygonF &polygon ) const;

    void drawTwoSampleAnnotation(int row, const Annotation &ann, QPainter *painter,
                                 const QwtScaleMap &xMap, const QwtScaleMap &yMap,
                                 const QRectF &canvasRect, const QwtPointMapper &mapper) const;

    void drawOneSampleAnnotation(int row, const Annotation &ann, QPainter *painter,
                                 const QwtScaleMap &xMap, const QwtScaleMap &yMap,
                                 const QRectF &canvasRect, const QwtPointMapper &mapper) const;

    void drawAnnotation(int row, const Annotation &ann, QPainter *painter,
                        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
                        const QRectF &canvasRect, const QwtPointMapper &mapper) const;

    void drawBlock(int row, uint64_t start, uint64_t end, QPainter *painter,
                   const QwtScaleMap &xMap, const QwtScaleMap &yMap,
                   const QRectF &canvasRect, const QwtPointMapper &mapper) const;


private:

    AnnotationDecoder *m_annotationDecoder;
    mutable std::mutex m_mutex;

    std::map<std::pair<const srd_decoder*, int>, Row> m_classRows;
    std::map<const Row, RowData> m_annotationRows;

	std::vector<std::shared_ptr<adiscope::bind::Decoder>> m_bindings;

    // pixelOffset for moving the curve up/down the plot
    // using the handle of it
    double m_pixelOffset;
    // We have no notion of V/div? use a pixel height value to draw
    // the distance between the "0" and "1"
    double m_traceHeight;
};
}

#endif // ANNOTATIONCURVE_H
