/*
 * Copyright (c) 2026 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef ACQMARKERCONTROLLER_H
#define ACQMARKERCONTROLLER_H

#include "scopy-core_export.h"

#include <gui/plotmarkercontroller.h>

#include <QPointer>

namespace scopy {
namespace adc {

class AcqCurveChannel;

// Peak / fixed / single-tone markers over the samples a curve channel is drawing. Peaks are
// searched only inside the X axis's current interval.
class SCOPY_CORE_EXPORT AcqMarkerController : public PlotMarkerController
{
	Q_OBJECT
public:
	enum class PeakStep
	{
		Left,
		Right,
		Higher,
		Lower,
		Highest,
	};

	explicit AcqMarkerController(AcqCurveChannel *ch, QObject *parent = nullptr);

	void bind();
	AcqCurveChannel *channel() const { return m_curve; }

	static QList<MarkerInfo> deltaInfo(const QList<MarkerInfo> &info);

	void attachMarkersToPlot() override;

	void moveFixedMarker(int idx, double x);
	bool stepToPeak(int idx, PeakStep step);

protected:
	int findPeakNearIdx(int idx, int range) override;
	void computeImageMarkers() override;
	void computeSingleToneMarkers() override;
	void computePeaks() override;
	void initFixedMarker() override;
	double valueAt(double x) override;

private:
	QVector<QPointF> samples() const;

	QPointer<AcqCurveChannel> m_curve;
	QVector<QPointF> m_data;
};

} // namespace adc
} // namespace scopy

#endif // ACQMARKERCONTROLLER_H
