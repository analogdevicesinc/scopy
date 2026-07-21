#pragma once

#include "scopy-core_export.h"

#include "ProcessorBlock.h"
#include "DataKey.h"
#include "decoder/IDecoderBackend.h"

#include <QList>
#include <memory>

namespace scopy {
namespace acq {

// ProcessorBlock that packs raw digital keys, invokes an IDecoderBackend
// once per cycle, and writes the resulting annotations back to DataStore.
// process() runs on the engine worker thread.
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

	// Annotation-input source (used only when cfg.rootInput == Annotations).
	void setAnnotationInputKey(const DataKey &k) { m_annInKey = k; }
	const DataKey &annotationInputKey() const    { return m_annInKey; }

	// One DataKey per stack stage (index 0 = root); demuxed by stageIndex.
	void setOutputKeys(const QList<DataKey> &keys)     { m_outKeys = keys; }
	const QList<DataKey> &outputKeys() const           { return m_outKeys; }

	double sampleRate() const { return m_cfg.sampleRate; }

	// Samples/cycle to assemble from ordered raw keys; 0 = single chunk.
	void setWindowSize(int n) { m_windowSize = n; }
	int  windowSize() const   { return m_windowSize; }

	void process(DataStore *store) override;
	void reset() override;

Q_SIGNALS:
	// Emitted once per cycle after outKey has been written.
	void cycleProduced(scopy::acq::DataKey outKey);

private:
	std::unique_ptr<scopy::decoder::IDecoderBackend> m_backend;
	scopy::decoder::DecoderConfig                    m_cfg;
	QList<DataKey>                                   m_orderedRawKeys;
	DataKey                                          m_annInKey;
	QList<DataKey>                                   m_outKeys;
	std::vector<uint8_t>                             m_packed;
	int                                              m_windowSize{0};
};

} // namespace acq
} // namespace scopy
