#ifndef LOGICDATACURVE_H
#define LOGICDATACURVE_H

#include <qwt_plot_curve.h>

#include "logic_analyzer.h"

class LogicDataCurve : public QwtPlotCurve
{
public:
	LogicDataCurve(uint16_t *data, uint8_t bit, adiscope::logic::LogicAnalyzer *logic);

    void dataAvailable(uint64_t from, uint64_t to);
    void reset();

    double getPixelOffset() const;
    double getTraceHeight() const;

    void setPixelOffset(double pixelOffset);
    void setTraceHeight(double traceHeight);

    void setPlotConfiguration(double sampleRate, uint64_t bufferSize, double timeTriggerOffset);

protected:
    void drawLines( QPainter *painter,
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        const QRectF &canvasRect, int from, int to ) const;

private:
    void getSubsampledEdges(std::vector<std::pair<uint64_t, bool> > &edges, const QwtScaleMap &xMap) const;
    uint64_t edgeAtX(int x, const std::vector<std::pair<uint64_t, bool> > &edges) const;

    uint64_t fromTimeToSample(double time) const;
    double fromSampleToTime(uint64_t sample) const;

private:

	adiscope::logic::LogicAnalyzer *m_logic;

    // pointer to data which this curve listens to
    uint16_t *m_data;
    // bit to watch in each sample from m_data
    uint8_t m_bit;

    uint64_t m_startSample;
    uint64_t m_endSample;

    // false -> ,,|'' true -> ''|,,
    std::vector<std::pair<uint64_t, bool>> m_edges;

    // pixelOffset for moving the curve up/down the plot
    // using the handle of it
    double m_pixelOffset;
    // We have no notion of V/div? use a pixel height value to draw
    // the distance between the "0" and "1"
    double m_traceHeight;

    double m_sampleRate;
    uint64_t m_bufferSize;
    double m_timeTriggerOffset;

};

#endif // LOGICDATACURVE_H
