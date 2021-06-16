/*
 * Copyright (c) 2020 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ANNOTATIONDECODER_H
#define ANNOTATIONDECODER_H

#include <queue>

#include <atomic>
#include <condition_variable>
#include <libsigrokdecode/libsigrokdecode.h>
#include <map>
#include <memory>
#include <mutex>
#include <scopy/gui/logicanalyzer/annotation_curve.hpp>
#include <scopy/gui/logicanalyzer/logic_decoder.hpp>
#include <thread>
#include <vector>

namespace scopy {
namespace gui {

class AnnotationDecoder
{
public:
	AnnotationDecoder(gui::AnnotationCurve* annotationCurve, std::shared_ptr<logic::Decoder> initialDecoder,
			  logic::LogicTool* logic);
	~AnnotationDecoder();

	void stackDecoder(std::shared_ptr<logic::Decoder> decoder);
	void unstackDecoder(std::shared_ptr<logic::Decoder> decoder);

	void startDecode();
	void stopDecode();

	void dataAvailable(uint64_t from, uint64_t to);

	std::vector<std::shared_ptr<logic::Decoder>> getDecoderStack();

	void assignChannel(uint16_t chId, uint16_t bitId);
	void unassignChannel(uint16_t chId);

	std::vector<DecodeChannel*> getDecoderChannels();

	void reset();

	int getNrOfChannels() const;

private:
	void stackChanged();
	void decodeProc();
	void initDecoderChannels();

private:
	AnnotationCurve* m_annotationCurve;
	logic::LogicTool* m_logic;

	uint64_t m_lastSample;

	struct srd_session* m_srdSession;
	std::vector<std::shared_ptr<logic::Decoder>> m_stack;
	std::map<std::pair<const srd_decoder*, int>, Row> m_class_rows;
	std::map<Row, RowData> m_annotation_rows;
	std::vector<DecodeChannel> m_channels;

	std::thread* m_decodeThread;
	std::atomic<bool> m_decodeCanceled;
	std::mutex m_newDataMutex;
	std::condition_variable m_newDataCv;
	std::queue<std::pair<uint64_t, uint64_t>> m_newDataQueue;
};
} // namespace gui
} // namespace scopy

#endif // ANNOTATIONDECODER_H
