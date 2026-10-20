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

#include "m2kstreamformat.h"

#include "component/streamview.h"

#include <QTest>

#include <cstdint>

using namespace scopy::adalm2000::stream;
using namespace scopy::component;

namespace {
constexpr int HW_CHANNELS = 2;

// Interleaved, 4 samples per channel: ch0 = 100..400, ch1 = -100..-400.
int16_t *fixture()
{
	static int16_t buffer[8] = {100, -100, 200, -200, 300, -300, 400, -400};
	return buffer;
}
} // namespace

class TST_M2kStreamFormat : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void alignRoundsUpToAMultipleOfFourAboveSixteen();
	void alignLeavesValidCountsAlone();
	void bothChannelsInterleaveAtTwoByteOffsets();
	void aSingleChannelKeepsTheFullStride();
	void outOfRangeChannelsAreDropped();
	void streamViewDecodesRawCodesPerChannel();
	void streamViewAppliesScaleAndOffset();
	void channelOrderFollowsTheRequestedOrder();
	void packedFormatIsOneSlotPerChannel();
	void packedFormatOfASingleChannelIsDense();
	void voltsToRawCodeInvertsAPositiveScale();
	void voltsToRawCodeInvertsTheDacsNegativeScale();
	void voltsToRawCodeSaturatesInsteadOfWrapping();
	void voltsToRawCodeHandlesADegenerateScale();
	void encodeThenDecodeRoundTrips();
	void digitalPinsAllViewTheSameWord();
	void digitalPinsOutOfRangeAreDropped();
	void streamViewDecodesIndividualDigitalBits();
	void isBitPackedTellsTheTwoLayoutsApart();
};

void TST_M2kStreamFormat::alignRoundsUpToAMultipleOfFourAboveSixteen()
{
	// libm2k rejects anything not a multiple of 4, and anything <= 16.
	QCOMPARE(alignSamplesCount(0), size_t(20));
	QCOMPARE(alignSamplesCount(1), size_t(20));
	QCOMPARE(alignSamplesCount(16), size_t(20));
	QCOMPARE(alignSamplesCount(17), size_t(20));
	QCOMPARE(alignSamplesCount(21), size_t(24));
	QCOMPARE(alignSamplesCount(4097), size_t(4100));
}

void TST_M2kStreamFormat::alignLeavesValidCountsAlone()
{
	QCOMPARE(alignSamplesCount(20), size_t(20));
	QCOMPARE(alignSamplesCount(4096), size_t(4096));
}

void TST_M2kStreamFormat::bothChannelsInterleaveAtTwoByteOffsets()
{
	const StreamFormat f = interleavedInt16Format(fixture(), 4, HW_CHANNELS, {0, 1}, {});

	QCOMPARE(f.channels.size(), 2);
	QCOMPARE(f.sampleCount, size_t(4));
	QCOMPARE(f.layout, SamplesLayout::Interleaved);
	QCOMPARE(f.channels.at(0).offset, ptrdiff_t(0));
	QCOMPARE(f.channels.at(1).offset, ptrdiff_t(2));
	QCOMPARE(f.channels.at(0).stride, ptrdiff_t(4));
	QCOMPARE(f.channels.at(1).stride, ptrdiff_t(4));
	QCOMPARE(f.channels.at(0).type, DataType::S16);
	QCOMPARE(f.channels.at(0).validBits, uint8_t(16));
}

void TST_M2kStreamFormat::aSingleChannelKeepsTheFullStride()
{
	// Hardware still digitises both channels, so asking for channel 1 alone must
	// keep the 2-channel stride and just start 2 bytes in. This is how
	// StreamConfig::enabledChannels is honoured without touching the hardware.
	const StreamFormat f = interleavedInt16Format(fixture(), 4, HW_CHANNELS, {1}, {});

	QCOMPARE(f.channels.size(), 1);
	QCOMPARE(f.channels.at(0).offset, ptrdiff_t(2));
	QCOMPARE(f.channels.at(0).stride, ptrdiff_t(4));
}

void TST_M2kStreamFormat::outOfRangeChannelsAreDropped()
{
	const StreamFormat f = interleavedInt16Format(fixture(), 4, HW_CHANNELS, {-1, 0, 2, 7}, {});

	QCOMPARE(f.channels.size(), 1);
	QCOMPARE(f.channels.at(0).offset, ptrdiff_t(0));
}

void TST_M2kStreamFormat::streamViewDecodesRawCodesPerChannel()
{
	const StreamFormat f = interleavedInt16Format(fixture(), 4, HW_CHANNELS, {0, 1}, {});
	StreamView view(f);

	QCOMPARE(view.channelCount(), 2);
	QCOMPARE(view.sampleCount(), size_t(4));

	QCOMPARE(view.raw<int16_t>(0, 0), int16_t(100));
	QCOMPARE(view.raw<int16_t>(0, 3), int16_t(400));
	QCOMPARE(view.raw<int16_t>(1, 0), int16_t(-100));
	QCOMPARE(view.raw<int16_t>(1, 3), int16_t(-400));
}

void TST_M2kStreamFormat::streamViewAppliesScaleAndOffset()
{
	// Raw codes are integral, so the integer path through StreamView is lossless
	// and scale/offset land the values in volts.
	QVector<ChannelScale> scales{{0.5, 1.0}, {0.25, -2.0}};
	const StreamFormat f = interleavedInt16Format(fixture(), 4, HW_CHANNELS, {0, 1}, scales);

	QCOMPARE(f.channels.at(0).scale, 0.5);
	QCOMPARE(f.channels.at(0).offsetPhys, 1.0);

	StreamView view(f);
	QCOMPARE(view.sampleAsDouble(0, 0), 100.0 * 0.5 + 1.0);
	QCOMPARE(view.sampleAsDouble(0, 3), 400.0 * 0.5 + 1.0);
	QCOMPARE(view.sampleAsDouble(1, 0), -100.0 * 0.25 - 2.0);
	QCOMPARE(view.sampleAsDouble(1, 3), -400.0 * 0.25 - 2.0);
}

void TST_M2kStreamFormat::channelOrderFollowsTheRequestedOrder()
{
	// Scales are indexed by request position, not by channel number, so a reversed
	// request must pair each scale with the channel the caller asked for.
	QVector<ChannelScale> scales{{2.0, 0.0}, {3.0, 0.0}};
	const StreamFormat f = interleavedInt16Format(fixture(), 4, HW_CHANNELS, {1, 0}, scales);

	QCOMPARE(f.channels.size(), 2);
	QCOMPARE(f.channels.at(0).offset, ptrdiff_t(2));
	QCOMPARE(f.channels.at(0).scale, 2.0);
	QCOMPARE(f.channels.at(1).offset, ptrdiff_t(0));
	QCOMPARE(f.channels.at(1).scale, 3.0);

	StreamView view(f);
	QCOMPARE(view.sampleAsDouble(0, 0), -100.0 * 2.0);
	QCOMPARE(view.sampleAsDouble(1, 0), 100.0 * 3.0);
}

void TST_M2kStreamFormat::packedFormatIsOneSlotPerChannel()
{
	// Playback buffers are ours, so the width is exactly the channel count and
	// nothing is skipped -- unlike capture, which views a subset of a 2-wide stream.
	int16_t buffer[8] = {};
	const StreamFormat f = packedInt16Format(buffer, 4, 2, {});

	QCOMPARE(f.channels.size(), 2);
	QCOMPARE(f.channels.at(0).offset, ptrdiff_t(0));
	QCOMPARE(f.channels.at(1).offset, ptrdiff_t(2));
	QCOMPARE(f.channels.at(0).stride, ptrdiff_t(4));
	QCOMPARE(f.channels.at(1).stride, ptrdiff_t(4));
}

void TST_M2kStreamFormat::packedFormatOfASingleChannelIsDense()
{
	int16_t buffer[4] = {};
	const StreamFormat f = packedInt16Format(buffer, 4, 1, {});

	QCOMPARE(f.channels.size(), 1);
	QCOMPARE(f.channels.at(0).offset, ptrdiff_t(0));
	QCOMPARE(f.channels.at(0).stride, ptrdiff_t(2));
}

void TST_M2kStreamFormat::voltsToRawCodeInvertsAPositiveScale()
{
	const ChannelScale cs{0.01, 0.5}; // volts = raw * 0.01 + 0.5
	QCOMPARE(voltsToRawCode(cs, 0.5), int16_t(0));
	QCOMPARE(voltsToRawCode(cs, 1.5), int16_t(100));
	QCOMPARE(voltsToRawCode(cs, -0.5), int16_t(-100));
}

void TST_M2kStreamFormat::voltsToRawCodeInvertsTheDacsNegativeScale()
{
	// The M2K DAC output stage inverts, so its solved scale is negative. Encoding
	// must follow the sign rather than special-casing it.
	const ChannelScale cs{-0.01, 0.0};
	QCOMPARE(voltsToRawCode(cs, 1.0), int16_t(-100));
	QCOMPARE(voltsToRawCode(cs, -1.0), int16_t(100));
	QCOMPARE(voltsToRawCode(cs, 0.0), int16_t(0));
}

void TST_M2kStreamFormat::voltsToRawCodeSaturatesInsteadOfWrapping()
{
	// An over-range request must clip to the rail, never fold to the opposite one.
	const ChannelScale cs{0.001, 0.0};
	QCOMPARE(voltsToRawCode(cs, 1000.0), int16_t(INT16_MAX));
	QCOMPARE(voltsToRawCode(cs, -1000.0), int16_t(INT16_MIN));

	const ChannelScale inverted{-0.001, 0.0};
	QCOMPARE(voltsToRawCode(inverted, 1000.0), int16_t(INT16_MIN));
	QCOMPARE(voltsToRawCode(inverted, -1000.0), int16_t(INT16_MAX));
}

void TST_M2kStreamFormat::voltsToRawCodeHandlesADegenerateScale()
{
	QCOMPARE(voltsToRawCode(ChannelScale{0.0, 0.0}, 5.0), int16_t(0));
}

void TST_M2kStreamFormat::encodeThenDecodeRoundTrips()
{
	// Encode volts into a packed buffer, then read them back through StreamView:
	// the two conventions must agree, since both sides share one ChannelScale.
	QVector<ChannelScale> scales{{-0.01, 0.25}};
	int16_t buffer[4] = {};
	const StreamFormat f = packedInt16Format(buffer, 4, 1, scales);

	const double wanted[4] = {0.0, 1.0, -1.0, 2.5};
	for(int i = 0; i < 4; ++i) {
		buffer[i] = voltsToRawCode(scales.at(0), wanted[i]);
	}

	StreamView view(f);
	for(int i = 0; i < 4; ++i) {
		// One code is 0.01 V here, so agreement to half a code is exact round-tripping.
		QVERIFY(qAbs(view.sampleAsDouble(0, size_t(i)) - wanted[i]) <= 0.005);
	}
}

void TST_M2kStreamFormat::digitalPinsAllViewTheSameWord()
{
	// One 16-bit word per sample: every pin shares offset 0 and the word stride,
	// and is told apart purely by `shift`.
	uint16_t words[4] = {0x0001, 0x8000, 0xAAAA, 0x0000};
	const StreamFormat f = bitPackedUint16Format(words, 4, {0, 1, 15});

	QCOMPARE(f.channels.size(), 3);
	QCOMPARE(f.sampleCount, size_t(4));
	for(int c = 0; c < 3; ++c) {
		QCOMPARE(f.channels.at(c).offset, ptrdiff_t(0));
		QCOMPARE(f.channels.at(c).stride, ptrdiff_t(2));
		QCOMPARE(f.channels.at(c).type, DataType::U16);
		QCOMPARE(f.channels.at(c).validBits, uint8_t(1));
	}
	QCOMPARE(f.channels.at(0).shift, uint8_t(0));
	QCOMPARE(f.channels.at(1).shift, uint8_t(1));
	QCOMPARE(f.channels.at(2).shift, uint8_t(15));
}

void TST_M2kStreamFormat::digitalPinsOutOfRangeAreDropped()
{
	// The word is 16 bits wide. getNbChannelsIn() reports 18 on hardware, but
	// those extra two are not in the sample word, so pin 16+ must be rejected.
	uint16_t words[2] = {0, 0};
	const StreamFormat f = bitPackedUint16Format(words, 2, {-1, 0, 16, 17, 15});

	QCOMPARE(f.channels.size(), 2);
	QCOMPARE(f.channels.at(0).shift, uint8_t(0));
	QCOMPARE(f.channels.at(1).shift, uint8_t(15));
}

void TST_M2kStreamFormat::streamViewDecodesIndividualDigitalBits()
{
	// 0xAAAA is 1010...10: even pins low, odd pins high.
	uint16_t words[4] = {0x0001, 0x8000, 0xAAAA, 0x0000};
	const StreamFormat f = bitPackedUint16Format(words, 4, {0, 1, 15});
	StreamView view(f);

	// pin 0 (channel 0)
	QCOMPARE(view.sampleAsDouble(0, 0), 1.0);
	QCOMPARE(view.sampleAsDouble(0, 1), 0.0);
	QCOMPARE(view.sampleAsDouble(0, 2), 0.0);
	// pin 1 (channel 1) - set only in 0xAAAA
	QCOMPARE(view.sampleAsDouble(1, 0), 0.0);
	QCOMPARE(view.sampleAsDouble(1, 2), 1.0);
	// pin 15 (channel 2) - set in 0x8000 and 0xAAAA
	QCOMPARE(view.sampleAsDouble(2, 0), 0.0);
	QCOMPARE(view.sampleAsDouble(2, 1), 1.0);
	QCOMPARE(view.sampleAsDouble(2, 2), 1.0);
	QCOMPARE(view.sampleAsDouble(2, 3), 0.0);

	// The raw accessor still yields the whole word, identically for every channel.
	QCOMPARE(view.raw<uint16_t>(0, 2), uint16_t(0xAAAA));
	QCOMPARE(view.raw<uint16_t>(2, 2), uint16_t(0xAAAA));
}

void TST_M2kStreamFormat::isBitPackedTellsTheTwoLayoutsApart()
{
	uint16_t words[2] = {0, 0};
	int16_t analog[4] = {0, 0, 0, 0};

	QVERIFY(isBitPacked(bitPackedUint16Format(words, 2, {0, 1})));
	QVERIFY(!isBitPacked(interleavedInt16Format(analog, 2, 2, {0, 1}, {})));
	QVERIFY(!isBitPacked(packedInt16Format(analog, 2, 2, {})));
	QVERIFY(!isBitPacked(StreamFormat{}));
}

QTEST_MAIN(TST_M2kStreamFormat)

#include "tst_m2kstreamformat.moc"
