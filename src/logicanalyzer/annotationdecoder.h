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

namespace adiscope {

class AnnotationDecoder
{
public:
    AnnotationDecoder(AnnotationCurve *annotationCurve, std::shared_ptr<logic::Decoder> initialDecoder,
	logic::LogicAnalyzer *logic);

    void stackDecoder(std::shared_ptr<logic::Decoder> decoder);

    void startDecode();
    void stopDecode();

    void newData(uint64_t from, uint64_t to);

    std::vector<std::shared_ptr<logic::Decoder>> getDecoderStack();

    void assignChannel(uint16_t chId, uint16_t bitId);
    void unassignChannel(uint16_t chId);

    std::vector<DecodeChannel *> getDecoderChannels();

private:
    void stackChanged();

    void decodeProc();


private:
    AnnotationCurve *m_annotationCurve;
	logic::LogicAnalyzer *m_logic;

    uint64_t m_lastSample;

    struct srd_session *m_srdSession;
    std::vector<std::shared_ptr<logic::Decoder>> m_stack;
    std::map<std::pair<const srd_decoder*, int>, Row> m_class_rows;
    std::map<const Row, RowData> m_annotation_rows;
    std::vector<DecodeChannel> m_channels;

    std::thread *m_decodeThread;
    std::atomic<bool> m_decodeCanceled;
    std::mutex m_newDataMutex;
    std::condition_variable m_newDataCv;
    std::queue<std::pair<uint64_t, uint64_t>> m_newDataQueue;
    void initDecoderChannels();
};
}

#endif // ANNOTATIONDECODER_H
