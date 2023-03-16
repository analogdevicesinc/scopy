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

#include <atomic>
#include <map>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

#include <libsigrokdecode/libsigrokdecode.h>

#include "annotationcurve.h"
#include "decoder.h"
#include "scopygui_export.h"

namespace adiscope {

class SCOPYGUI_EXPORT AnnotationDecoder
{
public:
    AnnotationDecoder(AnnotationCurve *annotationCurve, std::shared_ptr<logic::Decoder> initialDecoder);
    ~AnnotationDecoder();

    void stackDecoder(std::shared_ptr<logic::Decoder> decoder);
    void unstackDecoder(std::shared_ptr<logic::Decoder> decoder);

    void startDecode();
    void stopDecode();

    void dataAvailable(uint64_t from, uint64_t to, uint16_t *data);

    std::vector<std::shared_ptr<logic::Decoder>> getDecoderStack();

    void assignChannel(uint16_t chId, uint16_t bitId);
    void unassignChannel(uint16_t chId);

    std::vector<DecodeChannel *> getDecoderChannels();

    void reset();

    int getNrOfChannels() const;
private:
    void stackChanged();

    void decodeProc();


private:
    AnnotationCurve *m_annotationCurve;
    uint64_t m_lastSample;
    uint16_t *m_data;

    struct srd_session *m_srdSession;
    std::vector<std::shared_ptr<logic::Decoder>> m_stack;
    std::map<std::pair<const srd_decoder*, int>, Row> m_class_rows;
    std::map<Row, RowData> m_annotation_rows;
    std::vector<DecodeChannel> m_channels;

    std::thread *m_decodeThread;
    std::atomic<bool> m_decodeCanceled;
    std::mutex m_newDataMutex;
    std::condition_variable m_newDataCv;
    static std::mutex g_sessionMutex;
    std::queue<std::pair<uint64_t, uint64_t>> m_newDataQueue;
    void initDecoderChannels();
};
}

#endif // ANNOTATIONDECODER_H
