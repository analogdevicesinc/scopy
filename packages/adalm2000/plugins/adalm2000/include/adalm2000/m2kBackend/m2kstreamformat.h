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

#pragma once

#include "component/streamformat.h"

#include <QList>
#include <QVector>

#include <cstddef>
#include <cstdint>

namespace scopy::adalm2000::stream {

// libm2k requires a sample count that is a multiple of 4 and greater than 16.
inline constexpr size_t SAMPLES_MIN = 20;
inline constexpr size_t SAMPLES_MULTIPLE = 4;

// An affine raw -> volts mapping, solved from convertRawToVolts() at open time.
struct ChannelScale
{
	double scale = 1.0;
	double offset = 0.0;
};

inline size_t alignSamplesCount(size_t requested)
{
	size_t n = (requested < SAMPLES_MIN) ? SAMPLES_MIN : requested;
	const size_t remainder = n % SAMPLES_MULTIPLE;
	if(remainder != 0) {
		n += SAMPLES_MULTIPLE - remainder;
	}
	return n;
}

// Describe an interleaved raw-int16 buffer.
//
// The M2K ADC digitises every channel in one DMA stream and the interleaved
// getters re-enable both regardless of what was asked for, so `hwChannels` fixes
// the stride while `channels` selects which of them the format exposes. That is
// how StreamConfig::enabledChannels is honoured: at the descriptor, not by
// toggling hardware. Channels outside [0, hwChannels) are dropped.
inline component::StreamFormat interleavedInt16Format(void *data, size_t samplesPerChannel, int hwChannels,
						      const QList<int> &channels, const QVector<ChannelScale> &scales)
{
	component::StreamFormat format;
	format.data = data;
	format.sampleCount = samplesPerChannel;
	format.layout = component::SamplesLayout::Interleaved;
	format.order = component::ByteOrder::LittleEndian;

	const ptrdiff_t unit = static_cast<ptrdiff_t>(sizeof(int16_t));
	for(int i = 0; i < channels.size(); ++i) {
		const int channel = channels.at(i);
		if(channel < 0 || channel >= hwChannels) {
			continue;
		}

		component::ChannelFormat cf;
		cf.offset = static_cast<ptrdiff_t>(channel) * unit;
		cf.stride = static_cast<ptrdiff_t>(hwChannels) * unit;
		cf.type = component::DataType::S16;
		cf.validBits = 16;
		cf.shift = 0;
		if(i < scales.size()) {
			cf.scale = scales.at(i).scale;
			cf.offsetPhys = scales.at(i).offset;
		}
		format.channels.append(cf);
	}
	return format;
}

// Describe a densely packed raw-int16 buffer that WE own, one slot per enabled
// channel. The output direction differs from the input one: for capture the
// hardware hands back a fixed 2-wide stream and the format views a subset of it,
// whereas for playback we allocate the buffer, so its width is exactly the number
// of channels being pushed and nothing is skipped.
inline component::StreamFormat packedInt16Format(void *data, size_t samplesPerChannel, int channelCount,
						 const QVector<ChannelScale> &scales)
{
	component::StreamFormat format;
	format.data = data;
	format.sampleCount = samplesPerChannel;
	format.layout = component::SamplesLayout::Interleaved;
	format.order = component::ByteOrder::LittleEndian;

	const ptrdiff_t unit = static_cast<ptrdiff_t>(sizeof(int16_t));
	for(int i = 0; i < channelCount; ++i) {
		component::ChannelFormat cf;
		cf.offset = static_cast<ptrdiff_t>(i) * unit;
		cf.stride = static_cast<ptrdiff_t>(channelCount) * unit;
		cf.type = component::DataType::S16;
		cf.validBits = 16;
		cf.shift = 0;
		if(i < scales.size()) {
			cf.scale = scales.at(i).scale;
			cf.offsetPhys = scales.at(i).offset;
		}
		format.channels.append(cf);
	}
	return format;
}

// Invert a ChannelScale to encode a sample for playback.
//
// ChannelScale always describes raw -> volts, in both directions, so writers
// invert it rather than carrying a second convention. On the DAC `scale` is
// negative (the output stage inverts), which this handles without a special case.
// Saturates instead of wrapping, so an over-range request clips rather than
// folding to the opposite rail.
inline int16_t voltsToRawCode(const ChannelScale &cs, double volts)
{
	if(cs.scale == 0.0) {
		return 0;
	}
	const double raw = (volts - cs.offset) / cs.scale;
	if(raw <= double(INT16_MIN)) {
		return INT16_MIN;
	}
	if(raw >= double(INT16_MAX)) {
		return INT16_MAX;
	}
	return static_cast<int16_t>(raw < 0.0 ? raw - 0.5 : raw + 0.5);
}

// The digital sample word is 16 bits wide: DIO_CHANNEL runs 0..15 and both
// getSamplesP() and push() work in `unsigned short`. Note this is NOT
// getNbChannelsIn(), which reports 18 on hardware -- those two extra channels are
// not part of the sample word.
inline constexpr int DIGITAL_WORD_BITS = 16;

// Describe a digital buffer: one 16-bit word per sample, one bit per pin.
//
// Every pin reads the SAME word, so they all share offset 0 and the word stride,
// and are told apart by `shift` with a single valid bit. That is exactly the
// shift-then-mask path StreamView::decodeInt already implements, so no
// per-channel copying or custom decode is needed. Pins outside [0, 16) are
// dropped. Digital levels are unitless, so scale stays 1 and offset 0.
inline component::StreamFormat bitPackedUint16Format(void *data, size_t samplesPerChannel, const QList<int> &pins)
{
	component::StreamFormat format;
	format.data = data;
	format.sampleCount = samplesPerChannel;
	format.layout = component::SamplesLayout::Interleaved;
	format.order = component::ByteOrder::LittleEndian;

	const ptrdiff_t unit = static_cast<ptrdiff_t>(sizeof(uint16_t));
	for(int pin : pins) {
		if(pin < 0 || pin >= DIGITAL_WORD_BITS) {
			continue;
		}
		component::ChannelFormat cf;
		cf.offset = 0;
		cf.stride = unit;
		cf.type = component::DataType::U16;
		cf.validBits = 1;
		cf.shift = static_cast<uint8_t>(pin);
		format.channels.append(cf);
	}
	return format;
}

// True when `format` came from bitPackedUint16Format: every channel is a single
// bit of one shared word. Writers need to know, because setting one pin is a
// read-modify-write of the word rather than a store into its own slot.
inline bool isBitPacked(const component::StreamFormat &format)
{
	if(format.channels.isEmpty()) {
		return false;
	}
	for(const component::ChannelFormat &cf : format.channels) {
		if(cf.validBits != 1 || cf.offset != 0) {
			return false;
		}
	}
	return true;
}

} // namespace scopy::adalm2000::stream
