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


#ifndef LOGICDATACURVE_H
#define LOGICDATACURVE_H

#include "genericlogicplotcurve.h"

#include "logic_tool.h"

class LogicDataCurve : public GenericLogicPlotCurve
{
public:
	LogicDataCurve(uint16_t *data, uint8_t bit, adiscope::logic::LogicTool *logic);

    virtual void dataAvailable(uint64_t from, uint64_t to) override;
    virtual void reset() override;

    uint8_t getBitId() const;

    void setDisplaySampling(bool display);


protected:
    void drawLines( QPainter *painter,
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        const QRectF &canvasRect, int from, int to ) const;

private:
    void getSubsampledEdges(std::vector<std::pair<uint64_t, bool> > &edges, const QwtScaleMap &xMap) const;
    uint64_t edgeAtX(int x, const std::vector<std::pair<uint64_t, bool> > &edges) const;


private:

	adiscope::logic::LogicTool *m_logic;

    // pointer to data which this curve listens to
    uint16_t *m_data;
    // bit to watch in each sample from m_data
    uint8_t m_bit;

    uint64_t m_startSample;
    uint64_t m_endSample;

    // false -> ,,|'' true -> ''|,,
    std::vector<std::pair<uint64_t, bool>> m_edges;

    bool m_displaySampling;

};

#endif // LOGICDATACURVE_H
