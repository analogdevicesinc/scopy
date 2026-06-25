#pragma once

#include "scopy-core_export.h"

#include "ProcessorBlock.h"
#include "DataKey.h"
#include "decoder/IDecoderBackend.h"

#include <QList>
#include <memory>

namespace scopy {
namespace acq {

// ProcessorBlock that hands packed digital samples to an IDecoderBackend
// once per cycle and publishes the resulting annotations into the
// DataStore.
//
// Per-cycle behavior:
//   1. Reads m_orderedRawKeys[i] (each QVector<quint8>, 0/1 per sample) from
//      the store and packs them into ceil(numChannels/8) bytes per sample.
//   2. Calls backend->decode() with the packed buffer (one-shot).
//   3. Converts the resulting buffer-local annotations to Annotation values
//      and writes them to m_outKey (always, even if empty).
//   4. Emits annotation() per item for direct UI consumption.
//
// Threading: process() runs on the engine worker thread.
class SCOPY_CORE_EXPORT ExternalDecoderProcessor : public ProcessorBlock
{
	Q_OBJECT
public:
	ExternalDecoderProcessor(const QString &name,
				 std::unique_ptr<scopy::decoder::IDecoderBackend> backend,
				 QObject *parent = nullptr);
	~ExternalDecoderProcessor() override;

	void setConfig(const scopy::decoder::DecoderConfig &cfg) { m_cfg = cfg; }
	const scopy::decoder::DecoderConfig &config() const { return m_cfg; }

	// Bit i of each packed sample is taken from keys[i].
	void setOrderedRawKeys(const QList<DataKey> &keys) { m_orderedRawKeys = keys; }
	void setOutputKey(const DataKey &k)                { m_outKey = k; }
	const DataKey &outputKey() const                   { return m_outKey; }

	double sampleRate() const { return m_cfg.sampleRate; }

	void process(DataStore *store) override;
	void reset() override;

Q_SIGNALS:
	// Emitted once per cycle after annotations are written to the store.
	// Consumers read m_outKey from the DataStore in response.
	void cycleProduced(scopy::acq::DataKey outKey);

private:
	std::unique_ptr<scopy::decoder::IDecoderBackend> m_backend;
	scopy::decoder::DecoderConfig                    m_cfg;
	QList<DataKey>                                   m_orderedRawKeys;
	DataKey                                          m_outKey;
	std::vector<uint8_t>                             m_packed; // reused across cycles
};

} // namespace acq
} // namespace scopy
