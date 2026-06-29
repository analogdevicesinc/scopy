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
		bitVecs[i] = std::get<QVector<quint8>>(v);
		if(bitVecs[i].isEmpty()) continue;
		minLen = std::min(minLen, bitVecs[i].size());
		anyPopulated = true;
	}

	QVector<Annotation> annVec;

	if(!anyPopulated || minLen <= 0
	   || minLen == std::numeric_limits<qsizetype>::max()) {
		// Still publish (empty) so consumers see a defined state.
		if(!m_outKey.key.isEmpty()) {
			store->write(m_outKey, std::move(annVec));
			Q_EMIT cycleProduced(m_outKey);
		}
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

	annVec.reserve(static_cast<qsizetype>(outC.size()));
	for(const auto &a : outC) {
		Annotation out;
		out.startSample = a.start;
		out.endSample   = a.end;
		out.decoder     = QString::fromStdString(a.decoder);
		out.klass       = QString::fromStdString(a.klass);
		out.text        = QString::fromStdString(a.text);
		out.severity    = a.severity;
		annVec.append(out);
	}

	report(AcquisitionError::Severity::Info,
	       QStringLiteral("process(): produced %1 annotations, outKey=%2")
		       .arg(annVec.size())
		       .arg(m_outKey.key));

	if(!m_outKey.key.isEmpty()) {
		store->write(m_outKey, std::move(annVec));
		Q_EMIT cycleProduced(m_outKey);
	}
}

} // namespace acq
} // namespace scopy
