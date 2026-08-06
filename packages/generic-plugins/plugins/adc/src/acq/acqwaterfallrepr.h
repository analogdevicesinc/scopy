/*
 * Copyright (c) 2024 Analog Devices Inc.
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

#ifndef ACQWATERFALLREPR_H
#define ACQWATERFALLREPR_H

#include "acqchannelrepr.h"

#include <core/acq_engine/DataKey.h>

#include <QPointer>

namespace scopy {
class WaterfallPlotWidget;

namespace adc {

// A spectrogram of the key's chunk history: one pixel row per retained chunk.
//
// This is the repr the interface was shaped for, because it is the one that is not
// an item on a shared plot:
//
//   - it *is* a plot. createOwnPlot() hands the manager a WaterfallPlotWidget and
//     the manager gives it an exclusive splitter row, so the manager never names
//     this class or includes its header;
//   - it reads snapshot(), not window(). History is the vertical axis here, so the
//     depth claim is a row count rather than ceil(plotSize / bufferSize) — plotSize
//     is ignored entirely;
//   - the row count is a *setting*, so unlike every other repr its claim changes
//     without the manager doing anything. requestReclaim() is what closes that
//     loop.
//
// Cost, and it is real (plan T10): snapshot() copies the whole history by value
// while holding the store mutex — with the worker blocked on it — and toFloat()
// copies each chunk again into the vector handed to setHistorySnapshot(). At
// 256 rows x 1024 bins that is ~1 MB copied twice per cycle. Hence the row spinbox
// caps at 512 rather than being open-ended, and hence the claim is exactly m_rows:
// retaining chunks nobody paints would pay the first copy for nothing.
class WaterfallRepr : public AcqChannelRepr
{
public:
	WaterfallRepr();
	~WaterfallRepr() override;

	QString kindName() const override { return QStringLiteral("WATERFALL"); }

	PlotWidget *createOwnPlot(QWidget *parent) override;

	void attach(AcqPlotRow *row, const QString &name, const QColor &color) override;
	void detach() override;
	void pull(scopy::acq::DataStore *store, const scopy::acq::DataKey &key, int plotSize) override;
	void reset() override;
	std::size_t claimDepth(int plotSize, std::size_t bufferSize) const override;
	QWidget *createSettingsWidget(QWidget *parent) override;
	void setEnabled(bool en) override;

	// The key carrying the bin frequencies, so the X axis reads in Hz. Same role as
	// CurveRepr::setXKey — and the same processor writes both.
	void setXKey(const scopy::acq::DataKey &k) { m_xKey = k; }

	void setRows(int rows);
	int rows() const { return m_rows; }

private:
	// Owned by the manager's splitter once createOwnPlot() has handed it over, which
	// is why this is a QPointer and not a unique_ptr.
	QPointer<WaterfallPlotWidget> m_wf;

	scopy::acq::DataKey m_xKey;

	int m_rows{200};
	double m_minDb{-120.0};
	double m_maxDb{0.0};
	bool m_enabled{true};

	// The last frequency range pushed, so it is only pushed again when it actually
	// changes (an FFT reconfigure). setFrequencyRange() rescales the axis and
	// invalidates the raster cache, which is not worth paying every cycle for a
	// number that is constant across a run.
	double m_freqStart{0.0};
	double m_freqStop{0.0};
};

} // namespace adc
} // namespace scopy

#endif // ACQWATERFALLREPR_H
