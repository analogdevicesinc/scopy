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
 */

#ifndef ADALM2000SPECTRUM_API_H
#define ADALM2000SPECTRUM_API_H

#include "scopy-adalm2000_export.h"

#include <QList>
#include <QString>
#include <QVariantList>

#include <pluginbase/apiobject.h>

namespace scopy::adalm2000 {

class M2kSpectrumController;

class SCOPY_ADALM2000_EXPORT Adalm2000Spectrum_API : public ApiObject
{
	Q_OBJECT
	Q_PROPERTY(bool running READ running WRITE setRunning STORED false)
	Q_PROPERTY(int fft_size READ fftSize WRITE setFftSize)
	Q_PROPERTY(double start_freq READ startFreq WRITE setStartFreq)
	Q_PROPERTY(double stop_freq READ stopFreq WRITE setStopFreq)
	Q_PROPERTY(bool log_freq READ logFreq WRITE setLogFreq)
	Q_PROPERTY(int units READ units WRITE setUnits)
	Q_PROPERTY(double top READ top WRITE setTop)
	Q_PROPERTY(double bottom READ bottom WRITE setBottom)
	Q_PROPERTY(QList<bool> enabled READ enabled WRITE setEnabled)
	Q_PROPERTY(QList<int> averaging READ averaging WRITE setAveraging)
	Q_PROPERTY(QList<int> history READ history WRITE setHistory)
	Q_PROPERTY(QList<int> window READ window WRITE setWindow)
	Q_PROPERTY(QList<bool> low_range READ lowRange WRITE setLowRange)
	Q_PROPERTY(bool waterfall READ waterfall WRITE setWaterfall)
	Q_PROPERTY(int waterfall_channel READ waterfallChannel WRITE setWaterfallChannel)
	Q_PROPERTY(int marker_type READ markerType WRITE setMarkerType)
	Q_PROPERTY(int marker_count READ markerCount WRITE setMarkerCount)

public:
	explicit Adalm2000Spectrum_API(M2kSpectrumController *tool);

	bool running() const;
	void setRunning(bool on);

	int fftSize() const;
	void setFftSize(int n);

	double startFreq() const;
	void setStartFreq(double hz);

	double stopFreq() const;
	void setStopFreq(double hz);

	bool logFreq() const;
	void setLogFreq(bool on);

	int units() const;
	void setUnits(int idx);

	double top() const;
	void setTop(double v);

	double bottom() const;
	void setBottom(double v);

	QList<bool> enabled() const;
	void setEnabled(const QList<bool> &on);

	QList<int> averaging() const;
	void setAveraging(const QList<int> &modes);

	QList<int> history() const;
	void setHistory(const QList<int> &frames);

	QList<int> window() const;
	void setWindow(const QList<int> &windows);

	QList<bool> lowRange() const;
	void setLowRange(const QList<bool> &low);

	bool waterfall() const;
	void setWaterfall(bool on);

	int waterfallChannel() const;
	void setWaterfallChannel(int ch);

	int markerType() const;
	void setMarkerType(int idx);

	int markerCount() const;
	void setMarkerCount(int n);

	Q_INVOKABLE void single();
	Q_INVOKABLE void snapshot(int ch);
	Q_INVOKABLE QList<double> spectrum(int ch) const;
	Q_INVOKABLE QList<double> frequencies(int ch) const;
	Q_INVOKABLE QVariantList markers(int ch) const;
	Q_INVOKABLE void setMarkerFrequency(int ch, int idx, double hz);
	Q_INVOKABLE bool stepMarker(int ch, int idx, const QString &step);

private:
	M2kSpectrumController *m_tool;
};

} // namespace scopy::adalm2000

#endif // ADALM2000SPECTRUM_API_H
