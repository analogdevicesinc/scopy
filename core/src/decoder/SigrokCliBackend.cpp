#include "decoder/SigrokCliBackend.h"

#include "decoder/AnnotationCodecRegistry.h"
#include "decoder/DecoderLogger.h"
#include "decoder/IAnnotationExtractor.h"
#include "decoder/IProtocolDataEncoder.h"
#include "decoder/SigrokCliCatalog.h"

#include <QByteArray>
#include <QProcess>

#include <algorithm>
#include <cmath>
#include <limits>

namespace scopy {
namespace decoder {

static constexpr const char *kBackendId = "sigrok-cli-backend";

SigrokCliBackend::SigrokCliBackend(SigrokCliCatalog *catalog)
	: m_catalog(catalog)
{}

SigrokCliBackend::~SigrokCliBackend() = default;

QStringList SigrokCliBackend::buildArgs(const DecoderConfig &cfg) const
{
	return buildArgsWithInput(cfg,
		QString("binary:numchannels=%1:samplerate=%2")
			.arg(cfg.numChannels)
			.arg(static_cast<qint64>(cfg.sampleRate)));
}

std::map<std::string, std::string>
SigrokCliBackend::downstreamOptionOverrides(const std::string &decoderId)
{
	// Remaps needed on the annotation-input regen path (`-I protocoldata`
	// emits a single wire; PDs expecting split TX/RX must fold to one).
	static const std::map<std::string, std::map<std::string, std::string>> table = {
		{"modbus", {{"cschannel", "RX"}, {"scchannel", "RX"}}},
	};
	auto it = table.find(decoderId);
	return (it == table.end()) ? std::map<std::string, std::string>{}
	                           : it->second;
}

QStringList SigrokCliBackend::buildArgsWithInput(const DecoderConfig &cfg,
                                                 const QString &inputFormat,
                                                 const ProtocolDataInput *pdIn) const
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
		// synthetic root PD (from annIn.upstreamId) is prepended so sigrok
		// sees a valid root+stacked chain (e.g. spi:sck=..,spiflash).
		QStringList stageSpecs;
		stageSpecs.reserve(static_cast<int>(cfg.stack.size()) + 1);

		auto upstreamIt = cfg.meta.find(std::string("annIn.upstreamId"));
		if(upstreamIt != cfg.meta.end() && !upstreamIt->second.empty()) {
			// Root spec built from pdIn->rootChannelOverrides; no source
			// stage exists (upstream PD is reconstructed on the fly).
			QString rootSpec = QString::fromStdString(upstreamIt->second);
			for(const auto &kv : pdIn->rootChannelOverrides) {
				rootSpec += QString(":%1=%2")
					.arg(QString::fromStdString(kv.first))
					.arg(QString::fromStdString(kv.second));
			}
			stageSpecs << rootSpec;
		}

		for(const auto &stage : cfg.stack) {
			const auto extra = downstreamOptionOverrides(stage.decoderId);
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

		// Strip sigrok's "<id>-<n>" -P instance suffix; resolve stage index
		// by id match against the configured stack.
		const int lastDash = decoder.lastIndexOf('-');
		if(lastDash > 0) {
			const QString tail = decoder.mid(lastDash + 1);
			bool ok = false;
			(void)tail.toInt(&ok);
			if(ok) decoder = decoder.left(lastDash);
		}
		int stageIndex = -1;
		for(std::size_t i = 0; i < stack.size(); ++i) {
			if(QString::fromStdString(stack[i].decoderId) == decoder) {
				stageIndex = static_cast<int>(i);
				break;
			}
		}
		if(stageIndex < 0) stageIndex = 0; // unknown id: pin to root

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

namespace {

// Strip the "annIn." prefix from cfg.meta entries and return them as a flat map.
std::map<std::string, std::string> extractAnnInOpts(const DecoderConfig &cfg)
{
	std::map<std::string, std::string> out;
	static const std::string kPrefix = "annIn.";
	for(const auto &kv : cfg.meta) {
		if(kv.first.compare(0, kPrefix.size(), kPrefix) != 0) continue;
		out.emplace(kv.first.substr(kPrefix.size()), kv.second);
	}
	return out;
}

QString annInMetaGet(const DecoderConfig &cfg, const std::string &key)
{
	auto it = cfg.meta.find(std::string("annIn.") + key);
	return (it == cfg.meta.end()) ? QString{} : QString::fromStdString(it->second);
}

} // namespace

bool SigrokCliBackend::acceptsAnnotationInput(const DecoderConfig &cfg) const
{
	if(!m_codecs) return false;
	if(cfg.stack.empty()) return false;

	const QString upstreamId = annInMetaGet(cfg, "upstreamId");
	if(upstreamId.isEmpty()) return false;

	// Extractor and encoder are both keyed on upstreamId (we regenerate
	// the same wire; downstream stages are stacked by sigrok on top).
	return m_codecs->findExtractor(upstreamId) != nullptr &&
	       m_codecs->findEncoder(upstreamId)   != nullptr;
}

bool SigrokCliBackend::decodeAnnotations(const DecoderConfig &cfg,
                                        const std::vector<AnnotationC> &in,
                                        std::vector<AnnotationC> &out)
{
	out.clear();
	m_lastError.clear();

	if(!m_codecs) {
		m_lastError = "no codec registry attached to backend";
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

	const QString upstreamId = annInMetaGet(cfg, "upstreamId");

	IAnnotationExtractor *ext = m_codecs->findExtractor(upstreamId);
	IProtocolDataEncoder *enc = m_codecs->findEncoder(upstreamId);
	if(!ext || !enc) {
		m_lastError = std::string("no codec registered for upstream=")
			      + upstreamId.toStdString();
		if(m_logger)
			m_logger->critical(kBackendId, QString::fromStdString(m_lastError));
		return false;
	}

	const auto codecOpts = extractAnnInOpts(cfg);

	// 1. Extract payload symbols from upstream annotations.
	std::vector<ExtractedSymbol> symbols;
	QString extErr;
	if(!ext->extract(in, codecOpts, symbols, &extErr)) {
		m_lastError = "annotation extractor failed: " + extErr.toStdString();
		if(m_logger)
			m_logger->critical(kBackendId, QString::fromStdString(m_lastError));
		return false;
	}

	// 2. Encode symbols into `-I protocoldata:...` input.
	ProtocolDataInput pdIn;
	QString encErr;
	if(!enc->encode(symbols, codecOpts, pdIn, &encErr)) {
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

	// 4. Parse stdout with an extended stack [synthetic-root, user...]
	//    so parseStdout can resolve indices for both.
	std::vector<DecoderStage> extendedStack;
	extendedStack.reserve(cfg.stack.size() + 1);
	DecoderStage syntheticRoot;
	syntheticRoot.decoderId = upstreamId.toStdString();
	extendedStack.push_back(syntheticRoot);
	for(const auto &s : cfg.stack) extendedStack.push_back(s);

	std::vector<AnnotationC> syntheticOut;
	parseStdout(stdoutBuf, extendedStack, syntheticOut);

	// Drop synthetic-root (stageIndex==0); shift the rest down by 1.
	{
		std::vector<AnnotationC> filtered;
		filtered.reserve(syntheticOut.size());
		for(auto &a : syntheticOut) {
			if(a.stageIndex <= 0) continue;
			a.stageIndex -= 1;
			filtered.push_back(std::move(a));
		}
		syntheticOut = std::move(filtered);
	}

	// 5. Remap synthetic sample indices back to the upstream timeline via
	//    pdIn.byteToUpstreamAnn (proportional mapping across [0..nBytes)).
	const int nBytes = static_cast<int>(pdIn.byteToUpstreamAnn.size());
	const int nIn    = static_cast<int>(in.size());

	quint64 syntheticMin = std::numeric_limits<quint64>::max();
	quint64 syntheticMax = 0;
	for(const auto &a : syntheticOut) {
		if(a.start < syntheticMin) syntheticMin = a.start;
		if(a.end   > syntheticMax) syntheticMax = a.end;
	}
	if(syntheticMax < syntheticMin) { syntheticMin = 0; syntheticMax = 1; }
	const double span = static_cast<double>(syntheticMax - syntheticMin);

	auto mapByteIdx = [&](quint64 syntheticSample) -> int {
		if(nBytes <= 0 || span <= 0.0) return -1;
		const double t = (static_cast<double>(syntheticSample) - syntheticMin) / span;
		int idx = static_cast<int>(t * nBytes);
		if(idx < 0) idx = 0;
		if(idx >= nBytes) idx = nBytes - 1;
		return idx;
	};

	auto upstreamRange = [&](int byteIdx, quint64 &start, quint64 &end) -> bool {
		if(byteIdx < 0 || byteIdx >= nBytes) return false;
		const int annIdx = pdIn.byteToUpstreamAnn[byteIdx];
		if(annIdx < 0 || annIdx >= nIn) return false;
		start = in[annIdx].start;
		end   = in[annIdx].end;
		return true;
	};

	out.reserve(syntheticOut.size());
	for(auto &a : syntheticOut) {
		const int b0 = mapByteIdx(a.start);
		const int b1 = mapByteIdx(a.end);

		quint64 s0 = 0, e0 = 0, s1 = 0, e1 = 0;
		const bool ok0 = upstreamRange(b0, s0, e0);
		const bool ok1 = upstreamRange(b1, s1, e1);
		if(ok0 && ok1) {
			a.start = std::min(s0, s1);
			a.end   = std::max(e0, e1);
		} else if(ok0) {
			a.start = s0;
			a.end   = e0;
		} else if(ok1) {
			a.start = s1;
			a.end   = e1;
		} else if(!in.empty()) {
			a.start = in.front().start;
			a.end   = in.back().end;
		}
		out.push_back(std::move(a));
	}

	if(m_logger)
		m_logger->info(kBackendId,
			QStringLiteral("decodeAnnotations(): parsed %1 annotations (remapped)")
				.arg(out.size()));
	return true;
}

} // namespace decoder
} // namespace scopy
