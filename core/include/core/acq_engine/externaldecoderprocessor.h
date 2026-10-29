/*
 * Copyright (c) 2024 Analog Devices Inc.
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
 *
 */

#pragma once

#include "scopy-core_export.h"

#include "processorblock.h"
#include "datakey.h"
#include "decoder/idecoderbackend.h"

#include <QList>
#include <QMutex>
#include <QVector>
#include <memory>
#include <vector>

namespace scopy {
namespace acq {

// ProcessorBlock that packs raw digital keys, invokes an IDecoderBackend
// once per cycle, and writes the resulting annotations back to DataStore.
// process() runs on the engine worker thread.
//
// Every setter is safe while the engine runs: the inputs live under one mutex and
// process() works on a snapshot taken at the start of its cycle, so an edit lands on
// the next cycle and a cycle never sees half of one. The backends hold no state
// between calls — the config travels with every decode — so nothing else has to
// follow a change.
class SCOPY_CORE_EXPORT ExternalDecoderProcessor : public ProcessorBlock
{
	Q_OBJECT
public:
	ExternalDecoderProcessor(const QString &name, std::unique_ptr<scopy::decoder::IDecoderBackend> backend,
				 QObject *parent = nullptr);
	~ExternalDecoderProcessor() override;

	void setConfig(const scopy::decoder::DecoderConfig &cfg);
	scopy::decoder::DecoderConfig config() const;

	// The config and the raw inputs it describes, swapped together: cfg.numChannels and
	// the key count must agree, and two separate calls would let a cycle in between see
	// one without the other. Bit i of each packed sample is taken from rawKeys[i].
	void setRawInput(const scopy::decoder::DecoderConfig &cfg, const QList<DataKey> &rawKeys);

	// Annotation-input source (used only when cfg.rootInput == Annotations).
	void setAnnotationInputKey(const DataKey &k);
	DataKey annotationInputKey() const;

	// One DataKey per stack stage (index 0 = root); demuxed by stageIndex.
	void setOutputKeys(const QList<DataKey> &keys);
	QList<DataKey> outputKeys() const override;

	double sampleRate() const;

	// Samples/cycle to assemble from ordered raw keys; 0 = single chunk.
	void setWindowSize(int n);
	int windowSize() const;

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
	// Everything the GUI may change, kept together so process() can copy it in one lock.
	struct Inputs
	{
		scopy::decoder::DecoderConfig cfg;
		QList<DataKey> rawKeys;
		DataKey annInKey;
		QList<DataKey> outKeys;
		int windowSize{0};
	};
	Inputs inputs() const;

	// Both fill `perStage` (one bucket per output key) and leave publishing
	// to process(), so a bail-out still writes a defined empty result.
	void decodeAnnotationInput(const Inputs &in, DataStore *store, std::vector<QVector<Annotation>> &perStage);
	void decodeLogicInput(const Inputs &in, DataStore *store, std::vector<QVector<Annotation>> &perStage);
	void publish(const Inputs &in, DataStore *store, std::vector<QVector<Annotation>> &perStage);
	// Declares each output stream's AnnotationStreamInfo, so a decoder reading
	// these annotations as input can interpret them without being told how.
	void publishStreamInfo(const Inputs &in, DataStore *store);

	std::unique_ptr<scopy::decoder::IDecoderBackend> m_backend;
	mutable QMutex m_inMutex;
	Inputs m_in;
	// Worker-only scratch.
	std::vector<uint8_t> m_packed;
};

} // namespace acq
} // namespace scopy
