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

	// One output DataKey per stack stage (index 0 = root). Annotations
	// are demultiplexed by AnnotationC::stageIndex and written to the
	// matching key each cycle; unknown indices are dropped.
	void setOutputKeys(const QList<DataKey> &keys)     { m_outKeys = keys; }
	const QList<DataKey> &outputKeys() const           { return m_outKeys; }

	double sampleRate() const { return m_cfg.sampleRate; }

	// Window (in samples) the decoder should assemble from its ordered raw
	// keys each cycle. Defaults to 0 which falls back to a single buffer
	// chunk (legacy behavior).
	void setWindowSize(int n) { m_windowSize = n; }
	int  windowSize() const   { return m_windowSize; }

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
	QList<DataKey>                                   m_outKeys;
	std::vector<uint8_t>                             m_packed; // reused across cycles
	int                                              m_windowSize{0};
};

} // namespace acq
} // namespace scopy
