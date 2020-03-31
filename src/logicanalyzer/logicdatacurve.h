#ifndef LOGICDATACURVE_H
#define LOGICDATACURVE_H

#include "genericlogicplotcurve.h"

#include "logic_analyzer.h"

class LogicDataCurve : public GenericLogicPlotCurve
{
public:
	LogicDataCurve(uint16_t *data, uint8_t bit, adiscope::logic::LogicAnalyzer *logic);

    virtual void dataAvailable(uint64_t from, uint64_t to) override;
    virtual void reset() override;

    uint8_t getBitId() const;


protected:
    void drawLines( QPainter *painter,
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        const QRectF &canvasRect, int from, int to ) const;

private:
    void getSubsampledEdges(std::vector<std::pair<uint64_t, bool> > &edges, const QwtScaleMap &xMap) const;
    uint64_t edgeAtX(int x, const std::vector<std::pair<uint64_t, bool> > &edges) const;


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

};

#endif // LOGICDATACURVE_H
