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

#include "measure.h"

#include <QObject>
#include <QVector>

#include <algorithm>
#include <cmath>
#include <limits>

using namespace scopy::acq;

namespace {
constexpr int kHistogramBins = 256;

struct Info
{
	const char *name;
	const char *id;
	// Empty: the y unit; "*s": the y unit times seconds.
	const char *unit;
};

const Info kInfo[MEASURE_COUNT] = {
	{QT_TRANSLATE_NOOP("Measure", "Period"), "period", "s"},
	{QT_TRANSLATE_NOOP("Measure", "Frequency"), "frequency", "Hz"},
	{QT_TRANSLATE_NOOP("Measure", "Min"), "min", ""},
	{QT_TRANSLATE_NOOP("Measure", "Max"), "max", ""},
	{QT_TRANSLATE_NOOP("Measure", "Peak-peak"), "peak_peak", ""},
	{QT_TRANSLATE_NOOP("Measure", "Mean"), "mean", ""},
	{QT_TRANSLATE_NOOP("Measure", "Cycle Mean"), "cycle_mean", ""},
	{QT_TRANSLATE_NOOP("Measure", "RMS"), "rms", ""},
	{QT_TRANSLATE_NOOP("Measure", "Cycle RMS"), "cycle_rms", ""},
	{QT_TRANSLATE_NOOP("Measure", "AC RMS"), "ac_rms", ""},
	{QT_TRANSLATE_NOOP("Measure", "Area"), "area", "*s"},
	{QT_TRANSLATE_NOOP("Measure", "Cycle Area"), "cycle_area", "*s"},
	{QT_TRANSLATE_NOOP("Measure", "Low"), "low", ""},
	{QT_TRANSLATE_NOOP("Measure", "High"), "high", ""},
	{QT_TRANSLATE_NOOP("Measure", "Amplitude"), "amplitude", ""},
	{QT_TRANSLATE_NOOP("Measure", "Middle"), "middle", ""},
	{QT_TRANSLATE_NOOP("Measure", "+Over"), "positive_overshoot", "%"},
	{QT_TRANSLATE_NOOP("Measure", "-Over"), "negative_overshoot", "%"},
	{QT_TRANSLATE_NOOP("Measure", "Rise"), "rise", "s"},
	{QT_TRANSLATE_NOOP("Measure", "Fall"), "fall", "s"},
	{QT_TRANSLATE_NOOP("Measure", "+Width"), "positive_width", "s"},
	{QT_TRANSLATE_NOOP("Measure", "-Width"), "negative_width", "s"},
	{QT_TRANSLATE_NOOP("Measure", "+Duty"), "positive_duty", "%"},
	{QT_TRANSLATE_NOOP("Measure", "-Duty"), "negative_duty", "%"},
};

struct Cross
{
	int index;
	bool rising;
	char level;
};

enum Event
{
	NoCross,
	PosLow,
	PosHigh,
	PosFull,
	NegLow,
	NegHigh,
	NegFull,
};

Event crossing(double s, double prev, double low, double high)
{
	if(s > prev) {
		const bool l = prev <= low && s >= low;
		const bool h = prev <= high && s >= high;
		return l && h ? PosFull : h ? PosHigh : l ? PosLow : NoCross;
	}
	if(s < prev) {
		const bool l = prev >= low && s <= low;
		const bool h = prev >= high && s <= high;
		return l && h ? NegFull : l ? NegLow : h ? NegHigh : NoCross;
	}
	return NoCross;
}

// A level crossed with hysteresis: an edge counts once it passes the whole band, and is
// placed on the sample closest to the level. Edges alternate rising and falling.
class CrossDetector
{
public:
	CrossDetector(double level, double hysteresis, char name, QVector<Cross> *out)
		: m_level(level)
		, m_low(level - hysteresis / 2)
		, m_high(level + hysteresis / 2)
		, m_name(name)
		, m_out(out)
	{}

	void step(const float *d, int i)
	{
		const Event e = crossing(d[i], d[i - 1], m_low, m_high);
		if(m_posBetween) {
			closest(d, i, m_posPoint);
		}
		if(m_negBetween) {
			closest(d, i, m_negPoint);
		}
		if(e == NoCross) {
			return;
		}
		if(!m_posFound) {
			const bool was = m_posBetween;
			const bool crossed = (e == PosHigh && m_posBetween) || e == PosFull;
			m_posBetween = e == PosLow || (m_posBetween && e != PosHigh && e != NegLow);
			if(!was && m_posBetween) {
				m_posPoint = first(d, i);
			}
			if(crossed) {
				m_posFound = true;
				m_negFound = false;
				m_negBetween = false;
				if(e == PosFull) {
					m_posPoint = i;
				}
				m_out->append({m_posPoint, true, m_name});
			}
		}
		if(!m_negFound) {
			const bool was = m_negBetween;
			const bool crossed = (e == NegLow && m_negBetween) || e == NegFull;
			m_negBetween = e == NegHigh || (m_negBetween && e != NegLow && e != PosHigh);
			if(!was && m_negBetween) {
				m_negPoint = first(d, i);
			}
			if(crossed) {
				m_negFound = true;
				m_posFound = false;
				m_posBetween = false;
				if(e == NegFull) {
					m_negPoint = i - 1;
				}
				m_out->append({m_negPoint, false, m_name});
			}
		}
	}

private:
	int first(const float *d, int i) const
	{
		return std::abs(d[i - 1] - m_level) < std::abs(d[i] - m_level) ? i - 1 : i;
	}

	void closest(const float *d, int i, int &point) const
	{
		const int idx = first(d, i);
		if(std::abs(d[idx] - m_level) < std::abs(d[point] - m_level)) {
			point = idx;
		}
	}

	double m_level;
	double m_low;
	double m_high;
	char m_name;
	QVector<Cross> *m_out;
	bool m_posBetween{false};
	bool m_negBetween{false};
	bool m_posFound{false};
	bool m_negFound{false};
	int m_posPoint{0};
	int m_negPoint{0};
};

// Settling levels from a histogram: the fullest bins of each half, when they clearly
// outweigh the extremes (five times).
void levelsFromHistogram(const float *d, int start, int end, double min, double max, int bins, double &low,
			 double &high)
{
	if(bins < 2 || max <= min) {
		return;
	}
	QVector<int> hist(bins, 0);
	const double scale = (bins - 1) / (max - min);
	for(int i = start; i < end; ++i) {
		if(!std::isnan(d[i])) {
			++hist[static_cast<int>(std::lround((d[i] - min) * scale))];
		}
	}
	const int mid = bins / 2;
	const int lowBin = static_cast<int>(std::max_element(hist.begin(), hist.begin() + mid) - hist.begin());
	const int highBin = static_cast<int>(std::max_element(hist.begin() + mid, hist.end()) - hist.begin());
	if(hist[lowBin] / 5.0 >= hist[0] && hist[highBin] / 5.0 >= hist[bins - 1]) {
		low = min + lowBin / scale;
		high = min + highBin / scale;
	}
}

void set(MeasureResult &r, Measure m, double v) { r.values[static_cast<int>(m)] = v; }
} // namespace

QString scopy::acq::measureName(Measure m) { return QObject::tr(kInfo[static_cast<int>(m)].name); }

QString scopy::acq::measureId(Measure m) { return QString::fromLatin1(kInfo[static_cast<int>(m)].id); }

QString scopy::acq::measureUnit(Measure m, const QString &yUnit)
{
	const QString unit = QString::fromLatin1(kInfo[static_cast<int>(m)].unit);
	if(unit.isEmpty()) {
		return yUnit;
	}
	return unit == QLatin1String("*s") ? yUnit + QStringLiteral("s") : unit;
}

MeasureResult scopy::acq::measure(const float *d, int n, const MeasureParams &p)
{
	MeasureResult r;
	const int start = std::clamp(p.gateStart, 0, n);
	const int end = p.gateEnd < 0 ? n : std::clamp(p.gateEnd, start, n);
	if(!d || end - start < 2 || p.sampleRate <= 0) {
		return r;
	}
	const double dt = 1.0 / p.sampleRate;

	double min = std::numeric_limits<double>::max();
	double max = std::numeric_limits<double>::lowest();
	double sum = 0.0;
	double sqrSum = 0.0;
	int count = 0;
	for(int i = start; i < end; ++i) {
		if(std::isnan(d[i])) {
			continue;
		}
		min = std::min<double>(min, d[i]);
		max = std::max<double>(max, d[i]);
		sum += d[i];
		sqrSum += double(d[i]) * d[i];
		++count;
	}
	if(count == 0) {
		return r;
	}
	const double mean = sum / count;
	set(r, Measure::Min, min);
	set(r, Measure::Max, max);
	set(r, Measure::PeakPeak, max - min);
	set(r, Measure::Mean, mean);
	set(r, Measure::Rms, std::sqrt(sqrSum / count));
	set(r, Measure::AcRms, std::sqrt(std::max(0.0, sqrSum / count - mean * mean)));
	set(r, Measure::Area, sum * dt);

	double low = min;
	double high = max;
	levelsFromHistogram(d, start, end, min, max, kHistogramBins, low, high);
	const double amplitude = high - low;
	const double middle = low + amplitude / 2.0;
	set(r, Measure::Low, low);
	set(r, Measure::High, high);
	set(r, Measure::Amplitude, amplitude);
	set(r, Measure::Middle, middle);
	if(amplitude <= 0) {
		return r;
	}
	set(r, Measure::PosOver, (max - high) / amplitude * 100);
	set(r, Measure::NegOver, (low - min) / amplitude * 100);

	QVector<Cross> edges;
	CrossDetector period(middle, amplitude / 10, 'P', &edges);
	for(int i = start + 1; i < end; ++i) {
		period.step(d, i);
	}
	if(edges.size() < 3) {
		return r;
	}
	// Each half cycle averaged on its own, so a duty other than 50% does not bias it.
	double first = 0, second = 0;
	int firstCount = 0, secondCount = 0;
	for(int i = 0; i + 1 < edges.size(); ++i) {
		const int diff = edges[i + 1].index - edges[i].index;
		if(i % 2) {
			second += diff;
			++secondCount;
		} else {
			first += diff;
			++firstCount;
		}
	}
	const double periodS = (first / firstCount + second / secondCount) * dt;
	set(r, Measure::Period, periodS);
	set(r, Measure::Frequency, 1.0 / periodS);

	const int ps = edges[0].index;
	const int pe = edges[2].index;
	const int length = pe - ps + 1;
	r.marks.periodStart = ps;
	r.marks.periodEnd = pe;
	double cycleSum = 0.0;
	double cycleSqrSum = 0.0;
	for(int i = ps; i <= pe; ++i) {
		cycleSum += d[i];
		cycleSqrSum += double(d[i]) * d[i];
	}
	set(r, Measure::CycleMean, cycleSum / length);
	set(r, Measure::CycleRms, std::sqrt(cycleSqrSum / length));
	set(r, Measure::CycleArea, cycleSum * dt);

	// The 10/50/90 % crossings over the cycle, gone through twice so every edge shows.
	QVector<float> cycle(2 * length);
	for(int k = 0; k < cycle.size(); ++k) {
		cycle[k] = d[ps + k % length];
	}
	QVector<Cross> seq;
	CrossDetector cdLow(low + 0.1 * amplitude, amplitude / 10, 'L', &seq);
	CrossDetector cdMid(low + 0.5 * amplitude, amplitude / 10, 'M', &seq);
	CrossDetector cdHigh(low + 0.9 * amplitude, amplitude / 10, 'H', &seq);
	for(int k = 1; k < cycle.size(); ++k) {
		cdLow.step(cycle.constData(), k);
		cdMid.step(cycle.constData(), k);
		cdHigh.step(cycle.constData(), k);
	}
	// Crossings on one sample come out in detector order; put them in edge order.
	auto rank = [](const Cross &c) {
		const int r = c.level == 'L' ? 0 : c.level == 'M' ? 1 : 2;
		return c.rising ? r : -r;
	};
	for(int i = 0; i < seq.size();) {
		int j = i + 1;
		while(j < seq.size() && seq[j].index == seq[i].index && seq[j].rising == seq[i].rising) {
			++j;
		}
		std::sort(seq.begin() + i, seq.begin() + j,
			  [&](const Cross &a, const Cross &b) { return rank(a) < rank(b); });
		i = j;
	}
	const Cross want[] = {{0, true, 'L'},  {0, true, 'M'},	{0, true, 'H'},
			      {0, false, 'H'}, {0, false, 'M'}, {0, false, 'L'}};
	int pos = -1;
	for(int i = 0; i + 6 <= seq.size() && pos < 0; ++i) {
		bool match = true;
		for(int j = 0; j < 6 && match; ++j) {
			match = seq[i + j].level == want[j].level && seq[i + j].rising == want[j].rising;
		}
		if(match) {
			pos = i;
		}
	}
	if(pos < 0) {
		return r;
	}
	auto at = [&](int j) { return ps + seq[pos + j].index % length; };
	auto span = [&](int from, int to) {
		const int diff = at(to) - at(from);
		return (diff < 0 ? diff + length : diff) * dt;
	};
	r.marks.lowRising = at(0);
	r.marks.midRising = at(1);
	r.marks.highRising = at(2);
	r.marks.highFalling = at(3);
	r.marks.midFalling = at(4);
	r.marks.lowFalling = at(5);
	const double width = span(1, 4);
	set(r, Measure::Rise, span(0, 2));
	set(r, Measure::Fall, span(3, 5));
	set(r, Measure::PosWidth, width);
	set(r, Measure::NegWidth, periodS - width);
	set(r, Measure::PosDuty, width / periodS * 100);
	set(r, Measure::NegDuty, (periodS - width) / periodS * 100);
	return r;
}
