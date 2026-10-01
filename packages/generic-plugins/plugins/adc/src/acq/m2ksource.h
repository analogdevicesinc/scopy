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

#ifndef M2KSOURCE_H
#define M2KSOURCE_H

#include <core/acq_engine/SourceBlock.h>

#include <atomic>
#include <QStringList>

struct iio_context;

namespace libm2k {
namespace context {
class M2k;
}
namespace analog {
class M2kAnalogIn;
}
namespace digital {
class M2kDigital;
}
} // namespace libm2k

namespace scopy {
namespace adc {

// SourceBlock over an ADALM2000, covering both of its input subsystems in one block:
//
//   DIO0 … DIO15   the 16 logic inputs, one QVector<quint8> of 0/1 per channel
//   voltage0/1     the 2 scope inputs, QVector<float> in volts
//
// One block rather than two because a single M2k handle owns both subsystems, and the
// engine calls one acquire() per cycle: two blocks sharing the handle would interleave
// libm2k calls on the same transport from the same thread anyway, with nothing gained.
// The digital channels are published as ReprKind::Digital, which is what makes them
// available to the decoders; the analog pair as Curve.
//
// The two subsystems are independent in libm2k — separate rates, separate buffers — so
// this only starts the ones that have an enabled channel. A run with no digital channel
// enabled never calls startAcquisition() on the digital side, and the same for analog.
// Enabling nothing at all is not an error: the cycle simply publishes nothing.
//
// SAMPLE RATES ARE NOT SHARED. Digital defaults to 10 MHz and analog to 1 MHz, each
// clamped by the device to what it supports. streamInfo() reports whatever the device
// accepted, per channel group, so a decoder reading DIO timing and a plot reading volts
// each get their own real timeline instead of one averaged guess.
//
// Threading: onStart()/onStop() run on the worker around the cycle loop, acquire() per
// cycle. onStop() deliberately does not cancel a blocked read — see its comment.
//
// A run with an analog channel enabled calibrates the ADC first, unless the board reports
// itself already calibrated: libm2k applies the coefficients when it converts to volts, so
// without them the readings carry the board's raw offset and gain error.
class M2kSource : public scopy::acq::SourceBlock
{
	Q_OBJECT
public:
	static constexpr int kDigitalChannels = 16;
	static constexpr int kAnalogChannels = 2;

	// Opens an M2k over `ctx` and registers all 18 channels. Does not take the context:
	// libm2k adopts it without owning it, so `ctx` must outlive this block. Tolerant of a
	// context that is not an M2K — the handle stays null and isAvailable() answers false.
	explicit M2kSource(iio_context *ctx, const QString &id = "m2k", QObject *parent = nullptr);
	~M2kSource() override;

	// Whether m2kOpen() produced a handle in the constructor. A non-M2K context, or one
	// already opened elsewhere, answers false rather than throwing later.
	bool isAvailable() const override { return m_m2k != nullptr; }

	// Configures and starts only the subsystems with an enabled channel.
	void onStart() override;

	// Stops both subsystems. Idempotent, and reached from the destructor.
	void onStop() override;

	void acquire(scopy::acq::DataStore *store) override;

	// Digital as 0/1 on the digital rate, analog as volts on the analog rate. The base
	// class would answer unitless curves on an unknown timeline for all 18.
	std::optional<scopy::acq::StreamInfo> streamInfo(const scopy::acq::DataKey &key) const override;

	// The rates the device last accepted, in Hz. 0 before onStart() has run.
	double digitalSampleRate() const { return m_digitalRate.load(std::memory_order_relaxed); }
	double analogSampleRate() const { return m_analogRate.load(std::memory_order_relaxed); }

	// Requested rates, applied at the next onStart(). The device clamps them to what it
	// supports, which is what the getters above then report.
	void setDigitalSampleRate(double sr) { m_wantDigitalRate = sr; }
	void setAnalogSampleRate(double sr) { m_wantAnalogRate = sr; }

private:
	// "DIO0" … "DIO15" and "voltage0"/"voltage1". Static, unlike the IIO sources' discovered
	// lists: the M2K's input set is fixed by the hardware, and libm2k indexes it positionally.
	static QString digitalChannelId(int index);
	static QString analogChannelId(int index);

	// The engine's buffer size rounded up to what the analog side will accept: libm2k documents
	// nb_samples as "a multiple of 4 and greater than 16" for every M2kAnalogIn read. The engine's
	// spinbox allows 16 and does not enforce multiples of 4, so this cannot be assumed away. The
	// digital side has no such rule and uses the buffer size as given, which is why the two groups
	// can publish chunks of different lengths — each key carries its own.
	unsigned int analogSampleCount() const;

	// Which subsystems this run should drive, decided once in onStart() from the enable flags
	// so a mid-run toggle cannot leave acquire() reading a buffer that was never started.
	bool m_runDigital{false};
	bool m_runAnalog{false};

	// Reads one digital buffer and publishes the enabled channels' bits.
	void acquireDigital(scopy::acq::DataStore *store);
	// Reads one analog buffer and publishes the enabled channels' volts.
	void acquireAnalog(scopy::acq::DataStore *store);

	// Borrowed from libm2k's context registry, which owns them. Null when the context is not
	// an M2K; every use is guarded.
	libm2k::context::M2k         *m_m2k{nullptr};
	libm2k::digital::M2kDigital  *m_digital{nullptr};
	libm2k::analog::M2kAnalogIn  *m_analog{nullptr};

	// Written by onStart() and read per cycle by the same worker thread, but also read by the
	// GUI thread through the getters — hence atomic, like the other sources' cached rates.
	std::atomic<double> m_digitalRate{0.0};
	std::atomic<double> m_analogRate{0.0};

	// GUI-thread requests, read once per onStart(). Plain doubles: a torn read here would only
	// mean the device clamps a value the user is in the middle of typing.
	double m_wantDigitalRate{10e6};
	double m_wantAnalogRate{1e6};
};

} // namespace adc
} // namespace scopy

#endif // M2KSOURCE_H
