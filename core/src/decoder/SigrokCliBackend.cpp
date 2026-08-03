#include "decoder/SigrokCliBackend.h"

#include "decoder/AnnotationSymbols.h"
#include "decoder/DecoderLogger.h"
#include "decoder/SigrokCliCatalog.h"

#include "sigrok/ProtocolDataEncoder.h"

#include <QByteArray>
#include <QProcess>

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

namespace scopy {
namespace decoder {

static constexpr const char *kBackendId = "sigrok-cli-backend";

SigrokCliBackend::SigrokCliBackend(SigrokCliCatalog *catalog)
	: m_catalog(catalog)
	, m_encoders(std::make_unique<sigrok::ProtocolDataEncoderRegistry>())
{
	m_encoders->registerBuiltins();
}

SigrokCliBackend::~SigrokCliBackend() = default;

QStringList SigrokCliBackend::buildArgs(const DecoderConfig &cfg) const
{
	return buildArgsWithInput(cfg,
		QString("binary:numchannels=%1:samplerate=%2")
			.arg(cfg.numChannels)
			.arg(static_cast<qint64>(cfg.sampleRate)));
}

QStringList SigrokCliBackend::buildArgsWithInput(const DecoderConfig &cfg,
                                                 const QString &inputFormat,
                                                 const sigrok::ProtocolDataInput *pdIn) const
{
	QStringList args;
	args << "-i" << "-";
	args << "-I" << inputFormat;

	// "<id>:role=chan:opt=val:..." (single stage; no leading "-P").
	auto serializeStage = [](const DecoderStage &stage,
	                         const std::map<std::string, std::string> *rootChanOverrides,
	                         const std::map<std::string, std::string> &extraOpts) -> QString {
		QString spec = QString::fromStdString(stage.decoderId);
		for(const auto &ch : stage.channels) {
			QString value;
			if(rootChanOverrides) {
				auto it = rootChanOverrides->find(ch.role);
				if(it != rootChanOverrides->end())
					value = QString::fromStdString(it->second);
			}
			if(value.isEmpty())
				value = QString::number(ch.bitIndex);
			spec += QString(":%1=%2")
					 .arg(QString::fromStdString(ch.role))
					 .arg(value);
		}
		// User options first, then backend overrides (last occurrence wins).
		for(const auto &kv : stage.options) {
			spec += QString(":%1=%2")
					 .arg(QString::fromStdString(kv.first))
					 .arg(QString::fromStdString(kv.second));
		}
		for(const auto &kv : extraOpts) {
			spec += QString(":%1=%2")
					 .arg(QString::fromStdString(kv.first))
					 .arg(QString::fromStdString(kv.second));
		}
		return spec;
	};

	if(pdIn) {
		// Annotation-input path: one comma-joined -P arg (sigrok only wires
		// OUTPUT_PYTHON between stacked PDs sharing a single -P). The
		// synthetic root PD (from the upstream id) is prepended so sigrok
		// sees a valid root+stacked chain (e.g. spi:sck=..,spiflash).
		QStringList stageSpecs;
		stageSpecs.reserve(static_cast<int>(cfg.stack.size()) + 1);

		const AnnInOptions opts = AnnInOptions::fromMeta(cfg.meta);
		if(!opts.upstreamId.isEmpty()) {
			// Root spec built from pdIn->rootChannelOverrides; no source
			// stage exists (upstream PD is reconstructed on the fly).
			QString rootSpec = opts.upstreamId;
			for(const auto &kv : pdIn->rootChannelOverrides) {
				rootSpec += QString(":%1=%2")
					.arg(QString::fromStdString(kv.first))
					.arg(QString::fromStdString(kv.second));
			}
			stageSpecs << rootSpec;
		}

		for(const auto &stage : cfg.stack) {
			const auto extra = sigrok::downstreamOptionOverrides(
				QString::fromStdString(stage.decoderId));
			stageSpecs << serializeStage(stage, nullptr, extra);
		}
		args << "-P" << stageSpecs.join(',');
	} else {
		// Raw-decode path: same single-arg comma-joined -P for stacking.
		QStringList stageSpecs;
		stageSpecs.reserve(static_cast<int>(cfg.stack.size()));
		for(const auto &stage : cfg.stack) {
			stageSpecs << serializeStage(stage, nullptr,
				std::map<std::string, std::string>{});
		}
		args << "-P" << stageSpecs.join(',');
	}

	args << "--protocol-decoder-samplenum"
	     << "--protocol-decoder-ann-class";
	return args;
}

void SigrokCliBackend::parseStdout(const QByteArray &buf,
                                   const std::vector<DecoderStage> &stack,
                                   std::vector<AnnotationC> &out) const
{
	// sigrok labels annotations "<id>-<n>", where n counts instances of that
	// decoder id across the whole -P chain, in order. So the k-th stage using
	// id X answers to "X-(k+1)". Matching on the bare id instead pinned every
	// instance to the first stage that used it, silently merging the rows of a
	// stack like uart,modbus,modbus.
	std::map<std::string, std::vector<int>> stagesById;
	for(std::size_t i = 0; i < stack.size(); ++i)
		stagesById[stack[i].decoderId].push_back(static_cast<int>(i));

	const QList<QByteArray> lines = buf.split('\n');
	for(const QByteArray &raw : lines) {
		const QString s = QString::fromUtf8(raw).trimmed();
		if(s.isEmpty()) continue;

		// Format (with --protocol-decoder-samplenum --protocol-decoder-ann-class):
		//   "<start>-<end> <decoder>: <class>: <text>"
		const int firstSpace = s.indexOf(' ');
		if(firstSpace < 0) continue;

		const QString range = s.left(firstSpace);
		QString rest        = s.mid(firstSpace + 1);

		const int dash = range.indexOf('-');
		if(dash < 0) continue;

		bool okStart = false, okEnd = false;
		const quint64 start = range.left(dash).toULongLong(&okStart);
		const quint64 end   = range.mid(dash + 1).toULongLong(&okEnd);
		if(!okStart || !okEnd) continue;

		const int decColon = rest.indexOf(": ");
		if(decColon < 0) continue;
		QString decoder = rest.left(decColon);
		rest            = rest.mid(decColon + 2);

		// Split "<id>-<n>"; instance numbers are 1-based.
		int       instance = 1;
		const int lastDash = decoder.lastIndexOf('-');
		if(lastDash > 0) {
			bool      ok = false;
			const int n  = decoder.mid(lastDash + 1).toInt(&ok);
			if(ok) {
				instance = n;
				decoder  = decoder.left(lastDash);
			}
		}

		int  stageIndex = 0; // unresolvable id: pin to root
		auto it         = stagesById.find(decoder.toStdString());
		if(it != stagesById.end()) {
			const int nth = instance - 1;
			stageIndex = (nth >= 0 && nth < static_cast<int>(it->second.size()))
				? it->second[static_cast<std::size_t>(nth)]
				: it->second.front();
		}

		const int classColon = rest.indexOf(": ");
		QString klass, text;
		if(classColon >= 0) {
			klass = rest.left(classColon);
			text  = rest.mid(classColon + 2);
		} else {
			text = rest;
		}

		AnnotationC ann;
		ann.start      = start;
		ann.end        = end;
		ann.decoder    = decoder.toStdString();
		ann.klass      = klass.toStdString();
		ann.text       = text.toStdString();
		ann.severity   = 0;
		ann.stageIndex = stageIndex;
		out.push_back(std::move(ann));
	}
}

bool SigrokCliBackend::decode(const DecoderConfig &cfg,
                              const uint8_t *data, std::size_t nSamples,
                              std::vector<AnnotationC> &out)
{
	out.clear();
	m_lastError.clear();

	if(!data || nSamples == 0) {
		if(m_logger)
			m_logger->info(kBackendId, QStringLiteral("decode(): empty input, skipping"));
		return true;
	}

	const QString exe = m_catalog ? m_catalog->resolveCli() : QString{};
	if(exe.isEmpty()) {
		m_lastError = "sigrok-cli executable not found";
		if(m_logger)
			m_logger->critical(kBackendId, QString::fromStdString(m_lastError));
		return false;
	}

	const int unitsize = std::max(1, static_cast<int>(std::ceil(cfg.numChannels / 8.0)));
	const qint64 bytes = static_cast<qint64>(nSamples) * unitsize;

	const QStringList args = buildArgs(cfg);
	m_lastCmdLine          = exe + " " + args.join(' ');
	if(m_logger) {
		m_logger->info(kBackendId,
			QStringLiteral("decode(): exe=%1 argv=%2 nSamples=%3 unitsize=%4 bytes=%5")
				.arg(exe, args.join(' '))
				.arg(nSamples).arg(unitsize).arg(bytes));
	}

	QProcess proc;
	proc.setProcessChannelMode(QProcess::SeparateChannels);
	proc.start(exe, args);
	if(!proc.waitForStarted(2000)) {
		m_lastError = "failed to start sigrok-cli: "
			      + proc.errorString().toStdString();
		if(m_logger)
			m_logger->critical(kBackendId, QString::fromStdString(m_lastError));
		return false;
	}

	const qint64 wrote = proc.write(reinterpret_cast<const char *>(data), bytes);
	if(wrote != bytes && m_logger) {
		m_logger->warning(kBackendId,
			QStringLiteral("decode(): write short: wrote=%1 wanted=%2")
				.arg(wrote).arg(bytes));
	}
	proc.waitForBytesWritten(2000);
	proc.closeWriteChannel();

	if(!proc.waitForFinished(10000)) {
		m_lastError = "sigrok-cli timed out";
		if(m_logger)
			m_logger->critical(kBackendId, QString::fromStdString(m_lastError));
		proc.kill();
		proc.waitForFinished(500);
		return false;
	}

	const QByteArray stdoutBuf = proc.readAllStandardOutput();
	const QByteArray stderrBuf = proc.readAllStandardError();

	if(m_logger) {
		m_logger->info(kBackendId,
			QStringLiteral("decode(): exit=%1 status=%2 stdout=%3 bytes stderr=%4 bytes")
				.arg(proc.exitCode())
				.arg(int(proc.exitStatus()))
				.arg(stdoutBuf.size()).arg(stderrBuf.size()));
	}

	if(!stderrBuf.isEmpty() && m_logger) {
		m_logger->warning(kBackendId,
			QStringLiteral("decode(): stderr: ") + QString::fromUtf8(stderrBuf.trimmed()));
	}

	if(proc.exitStatus() != QProcess::NormalExit || proc.exitCode() != 0) {
		m_lastError = stderrBuf.trimmed().toStdString();
		if(m_lastError.empty())
			m_lastError = "sigrok-cli exited with non-zero status";
		return false;
	}

	parseStdout(stdoutBuf, cfg.stack, out);
	if(m_logger)
		m_logger->info(kBackendId,
			QStringLiteral("decode(): parsed %1 annotations").arg(out.size()));
	return true;
}

// Annotation-in path (chained decoding via `-I protocoldata:...`).

bool SigrokCliBackend::acceptsAnnotationInput(const DecoderConfig &cfg) const
{
	if(!m_extractors || cfg.stack.empty()) return false;

	const AnnInOptions opts = AnnInOptions::fromMeta(cfg.meta);
	if(opts.upstreamId.isEmpty()) return false;

	// Extractor and encoder are both keyed on the upstream id: we regenerate
	// the same wire, and downstream stages are stacked by sigrok on top.
	return m_extractors->find(opts.upstreamId) != nullptr &&
	       m_encoders->find(opts.upstreamId)   != nullptr;
}

bool SigrokCliBackend::decodeAnnotations(const DecoderConfig &cfg,
                                        const std::vector<AnnotationC> &in,
                                        std::vector<AnnotationC> &out)
{
	out.clear();
	m_lastError.clear();

	if(!m_extractors) {
		m_lastError = "no extractor registry attached to backend";
		if(m_logger)
			m_logger->critical(kBackendId, QString::fromStdString(m_lastError));
		return false;
	}
	if(cfg.stack.empty()) {
		m_lastError = "decodeAnnotations(): empty stack";
		if(m_logger)
			m_logger->critical(kBackendId, QString::fromStdString(m_lastError));
		return false;
	}
	if(in.empty()) {
		if(m_logger)
			m_logger->info(kBackendId,
				QStringLiteral("decodeAnnotations(): empty input, skipping"));
		return true;
	}

	QStringList         warnings;
	const AnnInOptions  opts = AnnInOptions::fromMeta(cfg.meta, &warnings);
	if(!opts.unknownKeys.isEmpty())
		warnings.append(QStringLiteral("unrecognized annIn keys: %1")
			.arg(opts.unknownKeys.join(QStringLiteral(", "))));
	for(const QString &w : warnings) {
		if(m_logger)
			m_logger->warning(kBackendId,
				QStringLiteral("decodeAnnotations(): ") + w);
	}

	IAnnotationExtractor          *ext = m_extractors->find(opts.upstreamId);
	sigrok::IProtocolDataEncoder  *enc = m_encoders->find(opts.upstreamId);
	if(!ext || !enc) {
		m_lastError = std::string("no annotation-input support for upstream=")
			      + opts.upstreamId.toStdString();
		if(m_logger)
			m_logger->critical(kBackendId, QString::fromStdString(m_lastError));
		return false;
	}

	// 1. Extract payload symbols from upstream annotations.
	std::vector<ExtractedSymbol> symbols;
	ExtractStats                 stats;
	QString                      extErr;
	if(!ext->extract(in, opts, symbols, &stats, &extErr)) {
		m_lastError = "annotation extractor failed: " + extErr.toStdString();
		if(m_logger)
			m_logger->critical(kBackendId, QString::fromStdString(m_lastError));
		return false;
	}
	// Unreadable records are skipped rather than fatal, so this counter is the
	// only sign of a radix mismatch — a decimal "105" read as hex gives the
	// wrong byte with no error at all. noPayload is deliberately *not* warned
	// about: it fires on every i2c capture (the R/W-bit row reuses the address
	// class), which would bury the case that actually indicates corruption.
	if(stats.sawRadixMismatch() && m_logger) {
		m_logger->warning(kBackendId,
			QStringLiteral("decodeAnnotations(): %1 annotation(s) hold a numeral that "
			               "is not valid in radix=%2 and carried no numeric value — "
			               "the upstream decoder's output format is probably not what "
			               "was declared, so accepted bytes may be wrong too (%3)")
				.arg(stats.radixMismatch)
				.arg(QLatin1String(acq::textRadixName(opts.stream.textRadix)),
				     stats.toString()));
	} else if(m_logger) {
		m_logger->info(kBackendId,
			QStringLiteral("decodeAnnotations(): extract %1").arg(stats.toString()));
	}
	if(symbols.empty()) {
		if(m_logger)
			m_logger->warning(kBackendId,
				QStringLiteral("decodeAnnotations(): no symbols extracted from %1 "
				               "annotation(s); nothing to decode").arg(in.size()));
		return true;
	}

	// 2. Encode symbols into `-I protocoldata:...` input.
	sigrok::ProtocolDataInput pdIn;
	QString                   encErr;
	if(!enc->encode(symbols, opts, pdIn, &encErr)) {
		m_lastError = "protocoldata encoder failed: " + encErr.toStdString();
		if(m_logger)
			m_logger->critical(kBackendId, QString::fromStdString(m_lastError));
		return false;
	}

	// 3. Spawn sigrok-cli with the protocoldata input.
	const QString exe = m_catalog ? m_catalog->resolveCli() : QString{};
	if(exe.isEmpty()) {
		m_lastError = "sigrok-cli executable not found";
		if(m_logger)
			m_logger->critical(kBackendId, QString::fromStdString(m_lastError));
		return false;
	}

	const QStringList args = buildArgsWithInput(cfg, pdIn.inputOpts, &pdIn);
	m_lastCmdLine          = exe + " " + args.join(' ');
	if(m_logger) {
		m_logger->info(kBackendId,
			QStringLiteral("decodeAnnotations(): exe=%1 argv=%2 upstreamAnns=%3 "
			               "symbols=%4 stdinBytes=%5")
				.arg(exe, args.join(' '))
				.arg(in.size()).arg(symbols.size())
				.arg(pdIn.stdinBytes.size()));
	}

	QProcess proc;
	proc.setProcessChannelMode(QProcess::SeparateChannels);
	proc.start(exe, args);
	if(!proc.waitForStarted(2000)) {
		m_lastError = "failed to start sigrok-cli: "
			      + proc.errorString().toStdString();
		if(m_logger)
			m_logger->critical(kBackendId, QString::fromStdString(m_lastError));
		return false;
	}

	const qint64 wrote = proc.write(pdIn.stdinBytes);
	if(wrote != pdIn.stdinBytes.size() && m_logger) {
		m_logger->warning(kBackendId,
			QStringLiteral("decodeAnnotations(): write short: wrote=%1 wanted=%2")
				.arg(wrote).arg(pdIn.stdinBytes.size()));
	}
	proc.waitForBytesWritten(2000);
	proc.closeWriteChannel();

	if(!proc.waitForFinished(10000)) {
		m_lastError = "sigrok-cli timed out";
		if(m_logger)
			m_logger->critical(kBackendId, QString::fromStdString(m_lastError));
		proc.kill();
		proc.waitForFinished(500);
		return false;
	}

	const QByteArray stdoutBuf = proc.readAllStandardOutput();
	const QByteArray stderrBuf = proc.readAllStandardError();

	if(m_logger) {
		m_logger->info(kBackendId,
			QStringLiteral("decodeAnnotations(): exit=%1 status=%2 stdout=%3 "
			               "bytes stderr=%4 bytes")
				.arg(proc.exitCode()).arg(int(proc.exitStatus()))
				.arg(stdoutBuf.size()).arg(stderrBuf.size()));
	}
	if(!stderrBuf.isEmpty() && m_logger) {
		m_logger->warning(kBackendId,
			QStringLiteral("decodeAnnotations(): stderr: ")
				+ QString::fromUtf8(stderrBuf.trimmed()));
	}
	if(proc.exitStatus() != QProcess::NormalExit || proc.exitCode() != 0) {
		m_lastError = stderrBuf.trimmed().toStdString();
		if(m_lastError.empty())
			m_lastError = "sigrok-cli exited with non-zero status";
		return false;
	}

	// 4. Parse stdout with an extended stack [synthetic-root, user...] so
	//    parseStdout can resolve indices for both.
	std::vector<DecoderStage> extendedStack;
	extendedStack.reserve(cfg.stack.size() + 1);
	DecoderStage syntheticRoot;
	syntheticRoot.decoderId = opts.upstreamId.toStdString();
	extendedStack.push_back(syntheticRoot);
	for(const auto &s : cfg.stack) extendedStack.push_back(s);

	std::vector<AnnotationC> parsed;
	parseStdout(stdoutBuf, extendedStack, parsed);

	// Split off the synthetic root stage (stageIndex==0). Its annotations are
	// not user-visible output, but they are the child's own report of where the
	// symbols we fed in landed on the fabricated wire — which is exactly what
	// the remap needs. The user stages shift down by 1.
	std::vector<AnnotationC> rootAnns, syntheticOut;
	syntheticOut.reserve(parsed.size());
	for(auto &a : parsed) {
		if(a.stageIndex < 0) continue;
		if(a.stageIndex == 0) {
			rootAnns.push_back(std::move(a));
			continue;
		}
		a.stageIndex -= 1;
		syntheticOut.push_back(std::move(a));
	}

	remapToUpstream(pdIn, ext, opts, rootAnns, in, syntheticOut, out);

	if(m_logger)
		m_logger->info(kBackendId,
			QStringLiteral("decodeAnnotations(): parsed %1 annotations (remapped)")
				.arg(out.size()));
	return true;
}

void SigrokCliBackend::remapToUpstream(const sigrok::ProtocolDataInput &pdIn,
                                       IAnnotationExtractor *ext,
                                       const AnnInOptions &opts,
                                       const std::vector<AnnotationC> &rootAnns,
                                       const std::vector<AnnotationC> &in,
                                       std::vector<AnnotationC> &syntheticOut,
                                       std::vector<AnnotationC> &out) const
{
	// The child decoded a *fabricated* waveform, so its sample numbers mean
	// nothing on the real timeline and every downstream range has to be mapped
	// back. The mapping is a list of (synthetic range -> upstream range) pairs,
	// one per symbol, built as follows:
	//
	//   pdIn.anchors[k]  = the k-th payload symbol we emitted, and which
	//                      upstream annotation it came from.
	//   rootAnns         = the synthetic root PD's own reading of that same
	//                      wire, so re-extracting it yields the same symbol
	//                      sequence with *real* synthetic sample numbers.
	//
	// Asking the child where the symbols landed is the only reliable answer.
	// Computing it here means re-deriving protocoldata's bit timing, framing
	// overhead and inter-frame gaps for every protocol — and the previous
	// version's shortcut (assume the symbols divide the span into equal slots)
	// is wrong the moment framing appears: an SPI CS release/assert pair or an
	// I2C START inserts idle samples that belong to no symbol, so word k is not
	// at base + k*step and every range after the first gap is off by one symbol
	// or more.
	const int nIn = static_cast<int>(in.size());
	out.reserve(syntheticOut.size());

	// (synthetic start, synthetic end, upstream annotation index), symbol order.
	struct Span
	{
		quint64 synStart{0};
		quint64 synEnd{0};
		int     upstreamAnnIndex{-1};
	};
	std::vector<Span> spans;

	if(ext) {
		std::vector<ExtractedSymbol> rootSymbols;
		QString                      ignoredErr;
		if(ext->extract(rootAnns, opts, rootSymbols, nullptr, &ignoredErr)) {
			// One anchor per emitted *wire slot*, not per symbol: SPI writes
			// MOSI and MISO of one word on a single line, and the root PD
			// annotates both halves over the identical sample range. So collapse
			// payload annotations to their distinct ranges — that granularity
			// matches the anchors for every protocol, without either side having
			// to know how the other groups bytes.
			std::vector<std::pair<quint64, quint64>> wireSlots;
			for(const ExtractedSymbol &s : rootSymbols) {
				if(!symbolTagCarriesByte(s.tag)) continue;
				const int ri = s.upstreamAnnIndex;
				if(ri < 0 || ri >= static_cast<int>(rootAnns.size())) continue;
				const AnnotationC &ra = rootAnns[static_cast<std::size_t>(ri)];
				wireSlots.emplace_back(ra.start, ra.end);
			}
			std::sort(wireSlots.begin(), wireSlots.end());
			wireSlots.erase(std::unique(wireSlots.begin(), wireSlots.end()), wireSlots.end());

			const std::size_t n = std::min(wireSlots.size(), pdIn.anchors.size());
			// A length mismatch means the wire we generated did not read back as
			// the symbol sequence we put on it, so the pairing is guesswork past
			// that point. Map what lines up and say so rather than drifting
			// silently, which is the failure the old proportional map had.
			if(wireSlots.size() != pdIn.anchors.size() && m_logger) {
				m_logger->warning(kBackendId,
					QStringLiteral("decodeAnnotations(): regenerated wire read back "
					               "as %1 payload slot(s) but %2 were emitted; "
					               "annotation sample ranges past slot %3 may be "
					               "approximate")
						.arg(wireSlots.size()).arg(pdIn.anchors.size()).arg(n));
			}
			spans.reserve(n);
			for(std::size_t k = 0; k < n; ++k)
				spans.push_back(Span{wireSlots[k].first, wireSlots[k].second,
				                     pdIn.anchors[k].upstreamAnnIndex});
		}
	}

	if(spans.empty() || nIn <= 0) {
		// No usable mapping (no root annotations, or the extractor disagreed
		// with itself). Report the whole upstream extent rather than sample
		// numbers from a timeline the caller knows nothing about.
		if(m_logger && !syntheticOut.empty() && nIn > 0) {
			m_logger->warning(kBackendId,
				QStringLiteral("decodeAnnotations(): could not map the synthetic "
				               "timeline back (rootAnns=%1 anchors=%2); reporting "
				               "%3 annotation(s) over the full upstream range")
					.arg(rootAnns.size()).arg(pdIn.anchors.size())
					.arg(syntheticOut.size()));
		}
		for(auto &a : syntheticOut) {
			if(nIn > 0) {
				a.start = in.front().start;
				a.end   = in.back().end;
			}
			out.push_back(std::move(a));
		}
		return;
	}

	// Upstream range of the symbol whose synthetic span covers `sample`, or the
	// nearest one when the sample falls in framing/idle between symbols.
	auto upstreamAt = [&](quint64 sample, quint64 &start, quint64 &end) -> bool {
		const Span *best     = nullptr;
		quint64     bestDist = 0;
		for(const Span &sp : spans) {
			if(sample >= sp.synStart && sample < sp.synEnd) { best = &sp; break; }
			const quint64 d = (sample < sp.synStart) ? (sp.synStart - sample)
			                                        : (sample - sp.synEnd);
			if(!best || d < bestDist) { best = &sp; bestDist = d; }
		}
		if(!best) return false;
		const int i = best->upstreamAnnIndex;
		if(i < 0 || i >= nIn) return false;
		start = in[static_cast<std::size_t>(i)].start;
		end   = in[static_cast<std::size_t>(i)].end;
		return true;
	};

	for(auto &a : syntheticOut) {
		quint64 s0 = 0, e0 = 0, s1 = 0, e1 = 0;
		const bool ok0 = upstreamAt(a.start, s0, e0);
		// `end` is exclusive on the synthetic wire; step inside the last symbol
		// so a range ending exactly on a boundary does not claim the next one.
		const bool ok1 = upstreamAt(a.end > a.start ? a.end - 1 : a.end, s1, e1);

		if(ok0 && ok1) {
			a.start = std::min(s0, s1);
			a.end   = std::max(e0, e1);
		} else if(ok0) {
			a.start = s0;
			a.end   = e0;
		} else if(ok1) {
			a.start = s1;
			a.end   = e1;
		} else {
			a.start = in.front().start;
			a.end   = in.back().end;
		}
		out.push_back(std::move(a));
	}
}

} // namespace decoder
} // namespace scopy
