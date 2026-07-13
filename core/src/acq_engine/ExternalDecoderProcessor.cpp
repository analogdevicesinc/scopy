#include "ExternalDecoderProcessor.h"

#include "DataStore.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace scopy {
namespace acq {

namespace {
// Pack per-bit 0/1 vectors into LSB-first packed bytes:
//   unitsize = ceil(numChannels/8), channel i = bit i.
// `out` is reused across cycles; assign() keeps its capacity.
void packBits(const std::vector<QVector<quint8>> &bitVecs,
	      int numChannels, qsizetype nSamples, int unitsize,
	      std::vector<uint8_t> &out)
{
	out.assign(static_cast<std::size_t>(nSamples) * unitsize, 0);
	const qsizetype nBits = std::min<qsizetype>(bitVecs.size(), numChannels);
	for(qsizetype s = 0; s < nSamples; ++s) {
		uint8_t *dst = out.data() + static_cast<std::size_t>(s) * unitsize;
		for(qsizetype b = 0; b < nBits; ++b) {
			const QVector<quint8> &src = bitVecs[b];
			if(src.size() <= s) continue;
			if(src[s])
				dst[b / 8] |= static_cast<uint8_t>(1u << (b % 8));
		}
	}
}
} // namespace

ExternalDecoderProcessor::ExternalDecoderProcessor(
	const QString &name,
	std::unique_ptr<scopy::decoder::IDecoderBackend> backend,
	QObject *parent)
	: ProcessorBlock(name, parent)
	, m_backend(std::move(backend))
{}

ExternalDecoderProcessor::~ExternalDecoderProcessor() = default;

void ExternalDecoderProcessor::reset()
{
	// One-shot model: nothing to reset between cycles.
}

void ExternalDecoderProcessor::process(DataStore *store)
{
	if(!m_backend || !store) {
		return;
	}
	if(m_orderedRawKeys.isEmpty()) {
		report(AcquisitionError::Severity::Warning,
		       QStringLiteral("process(): no rawKeys set"));
		return;
	}

	// Pull current cycle's per-bit vectors. minLen tracks the shortest
	// populated bit so we don't truncate to zero just because one channel
	// is missing.
	std::vector<QVector<quint8>> bitVecs;
	bitVecs.resize(m_orderedRawKeys.size());
	qsizetype minLen = std::numeric_limits<qsizetype>::max();
	bool anyPopulated = false;
	for(int i = 0; i < m_orderedRawKeys.size(); ++i) {
		QVector<quint8> vec;
		if(m_windowSize > 0) {
			vec = store->readWindowU8(m_orderedRawKeys[i], m_windowSize);
		} else {
			const SampleBuffer buf = store->read(m_orderedRawKeys[i]);
			if(buf.empty()) continue;
			const SampleVariant &v = buf.sample(0);
			if(!std::holds_alternative<QVector<quint8>>(v)) {
				report(AcquisitionError::Severity::Warning,
				       QStringLiteral("process(): key %1 wrong variant index=%2")
					       .arg(m_orderedRawKeys[i].key)
					       .arg(v.index()));
				continue;
			}
			vec = std::get<QVector<quint8>>(v);
		}
		if(vec.isEmpty()) continue;
		bitVecs[i] = std::move(vec);
		minLen = std::min(minLen, bitVecs[i].size());
		anyPopulated = true;
	}

	// One annotation bucket per stack stage; unknown stageIndex → dropped.
	const int nStages = m_outKeys.size();
	std::vector<QVector<Annotation>> perStage(static_cast<std::size_t>(nStages));

	auto publish = [&]() {
		for(int i = 0; i < nStages; ++i) {
			if(m_outKeys[i].key.isEmpty()) continue;
			store->write(m_outKeys[i], std::move(perStage[i]));
			Q_EMIT cycleProduced(m_outKeys[i]);
		}
	};

	if(!anyPopulated || minLen <= 0
	   || minLen == std::numeric_limits<qsizetype>::max()) {
		// Still publish (empty) so consumers see a defined state.
		publish();
		return;
	}

	const int unitsize = std::max(1, static_cast<int>(std::ceil(m_cfg.numChannels / 8.0)));
	packBits(bitVecs, m_cfg.numChannels, minLen, unitsize, m_packed);

	report(AcquisitionError::Severity::Info,
	       QStringLiteral("process(): decode %1 samples, unitsize=%2 bytes, totalBytes=%3")
		       .arg(minLen)
		       .arg(unitsize)
		       .arg(m_packed.size()));

	std::vector<scopy::decoder::AnnotationC> outC;
	const bool ok = m_backend->decode(m_cfg, m_packed.data(),
					  static_cast<std::size_t>(minLen), outC);
	if(!ok) {
		report(AcquisitionError::Severity::Warning,
		       QStringLiteral("backend decode failed: %1")
			       .arg(QString::fromStdString(m_backend->lastError())));
	}

	// Right-anchor pad for the plot window (same as before).
	const qint64 pad = (m_windowSize > 0)
		? std::max<qint64>(0, static_cast<qint64>(m_windowSize)
				      - static_cast<qint64>(minLen))
		: 0;

	for(const auto &a : outC) {
		if(a.stageIndex < 0 || a.stageIndex >= nStages) continue;
		Annotation out;
		out.startSample = static_cast<quint64>(
			static_cast<qint64>(a.start) + pad);
		out.endSample   = static_cast<quint64>(
			static_cast<qint64>(a.end)   + pad);
		out.decoder     = QString::fromStdString(a.decoder);
		out.klass       = QString::fromStdString(a.klass);
		out.text        = QString::fromStdString(a.text);
		out.severity    = a.severity;
		perStage[static_cast<std::size_t>(a.stageIndex)].append(out);
	}

	report(AcquisitionError::Severity::Info,
	       QStringLiteral("process(): produced annotations across %1 stages")
		       .arg(nStages));

	publish();
}

} // namespace acq
} // namespace scopy
