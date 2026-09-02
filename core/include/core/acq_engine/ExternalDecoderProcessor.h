#pragma once

#include "scopy-core_export.h"

#include "ProcessorBlock.h"
#include "DataKey.h"
#include "decoder/IDecoderBackend.h"

#include <QList>
#include <QVector>
#include <memory>
#include <vector>

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
	QList<DataKey> outputKeys() const override         { return m_outKeys; }

	double sampleRate() const { return m_cfg.sampleRate; }

	// Samples/cycle to assemble from ordered raw keys; 0 = single chunk.
	void setWindowSize(int n) { m_windowSize = n; }
	int  windowSize() const   { return m_windowSize; }

	void process(DataStore *store) override;
	void reset() override;

	// An Annotations descriptor for any of this decoder's output keys, labelled with
	// that stage's decoder id.
	// The protocol-level facts (radix, bitrate, producerId) go out separately as an
	// AnnotationStreamInfo from publishStreamInfo() — that is what a downstream
	// decoder consumes; this is only how a view should draw the stream.
	std::optional<StreamInfo> streamInfo(const DataKey &key) const override;

Q_SIGNALS:
	// Emitted once per cycle after outKey has been written.
	void cycleProduced(scopy::acq::DataKey outKey);

private:
	// Both fill `perStage` (one bucket per output key) and leave publishing
	// to process(), so a bail-out still writes a defined empty result.
	void decodeAnnotationInput(DataStore *store, std::vector<QVector<Annotation>> &perStage);
	void decodeLogicInput(DataStore *store, std::vector<QVector<Annotation>> &perStage);
	void publish(DataStore *store, std::vector<QVector<Annotation>> &perStage);
	// Declares each output stream's AnnotationStreamInfo, so a decoder reading
	// these annotations as input can interpret them without being told how.
	void publishStreamInfo(DataStore *store);

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
