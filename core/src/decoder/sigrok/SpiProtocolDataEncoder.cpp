// SPI protocol-data encoder: emits text-format lines "0xMM 0xSS" wrapped by
// cs-assert/cs-release markers (raw-bytes yields no SCK edges).
// Synth defaults: samplerate 10M, bitrate 1M, direction "both".
//
// CS framing matters: a single cs-assert/cs-release pair around every word
// makes the whole capture look like ONE transfer, so upper-layer PDs that key
// off CS (spiflash and friends) parse the first command and treat every
// following byte as its payload. TransferEnd symbols (from the upstream spi
// PD's mosi-/miso-transfer classes) delimit the real CS blocks, and each one is
// reproduced here as a cs-release/cs-assert boundary.

#include "ProtocolDataEncoder.h"

#include <QByteArray>
#include <QString>

namespace scopy {
namespace decoder {
namespace sigrok {
namespace {

class SpiEncoder : public IProtocolDataEncoder
{
public:
	QString downstreamId() const override { return QStringLiteral("spi"); }

	bool encode(const std::vector<ExtractedSymbol> &sym,
	            const AnnInOptions &opts,
	            ProtocolDataInput &out,
	            QString *err) override
	{
		(void)err;

		const double  samplerate = opts.synthSampleRate > 0.0 ? opts.synthSampleRate : 1.0e7;
		const quint64 bitrate    = opts.synthBitrate > 0 ? opts.synthBitrate : 1000000;
		const QString direction  = opts.direction.isEmpty()
			? QStringLiteral("both") : opts.direction;

		const bool useMosi = (direction != QStringLiteral("miso"));
		const bool useMiso = (direction != QStringLiteral("mosi"));

		out.inputOpts = QStringLiteral(
			"protocoldata:protocol=spi:samplerate=%1:bitrate=%2:"
			"textinput=text-format")
			.arg(QString::number(samplerate, 'f', 0), QString::number(bitrate));

		out.stdinBytes.clear();
		out.anchors.clear();

		// Regenerated wire channel names: sck/miso/mosi/cs (clk -> sck).
		out.rootChannelOverrides["clk"]  = "sck";
		out.rootChannelOverrides["miso"] = "miso";
		out.rootChannelOverrides["mosi"] = "mosi";
		out.rootChannelOverrides["cs"]   = "cs";

		QByteArray body;
		// Only payload words get an anchor; pseudo-comments produce no samples
		// on the wire, so anchoring them would map real sample ranges onto
		// zero-width gaps in the regenerated timeline. One line carries both
		// directions of one word, hence one anchor per word.
		auto appendRaw = [&](const QByteArray &line) { body.append(line); };
		auto appendWord = [&](uint8_t mosi, uint8_t miso, int annIdx) {
			body.append("0x")
			    .append(QByteArray::number(mosi, 16).rightJustified(2, '0'))
			    .append(' ')
			    .append("0x")
			    .append(QByteArray::number(miso, 16).rightJustified(2, '0'))
			    .append('\n');
			out.anchors.push_back(SymbolAnchor{annIdx});
		};

		appendRaw(QByteArrayLiteral("# textinput: radix=hex\n"));

		// The upstream spi PD emits miso-data and mosi-data for the same word
		// back to back, so pair them by walking the symbol list once. A pending
		// half is flushed when the opposite direction arrives, and at every CS
		// boundary / end of stream.
		bool    csOpen = false;
		bool    haveMosi = false, haveMiso = false;
		uint8_t pendMosi = 0, pendMiso = 0;
		int     pendAnn = -1;

		auto openCs = [&]() {
			if(csOpen) return;
			appendRaw(QByteArrayLiteral("# spi: cs-assert\n"));
			csOpen = true;
		};
		auto flushWord = [&]() {
			if(!haveMosi && !haveMiso) return;
			openCs();
			appendWord(haveMosi ? pendMosi : 0, haveMiso ? pendMiso : 0, pendAnn);
			haveMosi = haveMiso = false;
			pendMosi = pendMiso = 0;
			pendAnn  = -1;
		};
		auto closeCs = [&]() {
			flushWord();
			if(!csOpen) return;
			appendRaw(QByteArrayLiteral("# spi: cs-release\n"));
			csOpen = false;
		};

		for(const ExtractedSymbol &s : sym) {
			if(s.tag == SymbolTag::TransferEnd) {
				closeCs();
				continue;
			}

			const bool isMosi = (s.tag == SymbolTag::Mosi);
			const bool isMiso = (s.tag == SymbolTag::Miso);
			if(!isMosi && !isMiso) continue;
			if(isMosi && !useMosi) continue;
			if(isMiso && !useMiso) continue;

			// Second sample for the same direction => previous word is done.
			if((isMosi && haveMosi) || (isMiso && haveMiso))
				flushWord();

			if(isMosi) { pendMosi = s.byte; haveMosi = true; }
			else       { pendMiso = s.byte; haveMiso = true; }
			if(pendAnn < 0) pendAnn = s.upstreamAnnIndex;

			// Both halves present and we are not waiting on a pair partner.
			if(haveMosi && haveMiso) flushWord();
		}

		closeCs();

		out.stdinBytes = body;
		return true;
	}
};

} // namespace

std::unique_ptr<IProtocolDataEncoder> makeSpiEncoder()
{
	return std::make_unique<SpiEncoder>();
}

} // namespace sigrok
} // namespace decoder
} // namespace scopy
