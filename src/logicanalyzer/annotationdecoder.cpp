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


#include "annotationdecoder.h"
#include <libsigrokdecode/libsigrokdecode.h>
#include "logic_analyzer.h"
#include <QDebug>
#include <algorithm>

using namespace adiscope;

constexpr uint64_t MAX_CHUNK_SIZE = 256 * 1024;

std::mutex AnnotationDecoder::g_sessionMutex;

void AnnotationDecoder::initDecoderChannels()
{
    uint16_t id = 0;

    // Copy existing entries, create new as needed
    const srd_decoder* srd_d = m_stack.front()->decoder();
    const GSList *l;

    // Mandatory channels
    for (l = srd_d->channels; l; l = l->next) {
        const struct srd_channel *const pdch = (struct srd_channel *)l->data;
        bool ch_added = false;

        if (!ch_added) {
            // Create new entry without a mapped signal
            DecodeChannel ch = {id++, 0, false, false,
                QString::fromUtf8(pdch->name), QString::fromUtf8(pdch->desc),
                SRD_INITIAL_PIN_SAME_AS_SAMPLE0, m_stack.front(), pdch};
            m_channels.push_back(ch);
        }
    }

    // Optional channels
    int optIndex = 0;
    for (l = srd_d->opt_channels; l; l = l->next) {
        const struct srd_channel *const pdch = (struct srd_channel *)l->data;
        bool ch_added = false;

        if (!ch_added) {
            // Create new entry without a mapped signal
            DecodeChannel ch = {id++, 0, true, false,
                QString::fromUtf8(pdch->name), QString::fromUtf8(pdch->desc),
                SRD_INITIAL_PIN_SAME_AS_SAMPLE0, m_stack.front(), pdch};
            m_channels.push_back(ch);
        }

        optIndex++;
    }
}

AnnotationDecoder::AnnotationDecoder(AnnotationCurve *annotationCurve, std::shared_ptr<logic::Decoder> initialDecoder, logic::LogicTool *logic)
    : m_annotationCurve(annotationCurve)
    , m_decodeThread(nullptr)
    , m_srdSession(nullptr)
    , m_logic(logic)
    , m_decodeCanceled(false)
    , m_lastSample(0)
{
    // 1. Get stacked decoder from annotation Curve
    // 2. Configure curve (channels and annotations)
    // 3. spawn thread for decoding and make it wait

    // m_annotationCurve->getDecoder()

    m_stack.push_back(initialDecoder);

    initDecoderChannels();

    stackChanged();
    startDecode();
}

AnnotationDecoder::~AnnotationDecoder()
{
	if (m_srdSession) {
	    m_decodeCanceled = true;
	    srd_session_terminate_reset(m_srdSession);
	    {
	    std::unique_lock<std::mutex> lock(m_newDataMutex);
	    m_newDataCv.notify_one();
	    }

	srd_session_metadata_set(m_srdSession, SRD_CONF_SAMPLERATE,
				 g_variant_new_uint64(m_annotationCurve->getSampleRate()));
	    for (const std::shared_ptr<logic::Decoder> &dec : m_stack) {
		dec->apply_all_options();
	    }
	}
	stopDecode();
}

void AnnotationDecoder::stackDecoder(std::shared_ptr<logic::Decoder> decoder)
{
    if (m_srdSession) {
        m_decodeCanceled = true;
        srd_session_terminate_reset(m_srdSession);
        {
            std::unique_lock<std::mutex> lock(m_newDataMutex);
            m_newDataCv.notify_one();
        }

        srd_session_metadata_set(m_srdSession, SRD_CONF_SAMPLERATE,
                                 g_variant_new_uint64(m_annotationCurve->getSampleRate()));
//        qDebug() << "SampleRate: " << m_annotationCurve->getSampleRate();
        for (const std::shared_ptr<logic::Decoder> &dec : m_stack) {
            dec->apply_all_options();
        }
    }

    m_stack.push_back(decoder);

    // reconfigure stack
    stackChanged();
    startDecode();
}

void AnnotationDecoder::unstackDecoder(std::shared_ptr<logic::Decoder> decoder)
{
	if (m_srdSession) {
	    m_decodeCanceled = true;
	    srd_session_terminate_reset(m_srdSession);
	    {
		std::unique_lock<std::mutex> lock(m_newDataMutex);
		m_newDataCv.notify_one();
	    }

	    srd_session_metadata_set(m_srdSession, SRD_CONF_SAMPLERATE,
				     g_variant_new_uint64(m_annotationCurve->getSampleRate()));
    //        qDebug() << "SampleRate: " << m_annotationCurve->getSampleRate();
	    for (const std::shared_ptr<logic::Decoder> &dec : m_stack) {
		dec->apply_all_options();
	    }
	}

	qDebug() << "stack size before deleting: " << m_stack.size();

	m_stack.erase(std::find(m_stack.begin(), m_stack.end(), decoder));

	qDebug() << "stack size after deleting: " << m_stack.size();

	// reconfigure stack
	stackChanged();
	startDecode();
}

void AnnotationDecoder::startDecode()
{
//    qDebug() << "Start decode!";
    // TODO: cancel mechanism

    if (m_srdSession) {
        m_decodeCanceled = true;
        srd_session_terminate_reset(m_srdSession);
        {
            std::unique_lock<std::mutex> lock(m_newDataMutex);
            m_newDataCv.notify_one();
        }

        srd_session_metadata_set(m_srdSession, SRD_CONF_SAMPLERATE,
                                 g_variant_new_uint64(m_annotationCurve->getSampleRate()));
//                qDebug() << "SampleRate: " << m_annotationCurve->getSampleRate();
        for (const std::shared_ptr<logic::Decoder> &dec : m_stack) {
            dec->apply_all_options();
        }
    }

    if (m_lastSample != 0) {

//	m_annotationCurve->reset();
        // set curves class rows and annotation rows
        m_annotationCurve->setClassRows(m_class_rows);
        m_annotationCurve->setAnnotationRows(m_annotation_rows);

        std::unique_lock<std::mutex> lock(m_newDataMutex);
        {
            // clear the current queue content
            std::queue<std::pair<uint64_t, uint64_t>> empty;
            m_newDataQueue.swap(empty);
        }
        uint64_t q = m_lastSample / MAX_CHUNK_SIZE;
        uint64_t r = m_lastSample % MAX_CHUNK_SIZE;
        for (uint64_t i = 0; i < q; ++ i) {
            m_newDataQueue.emplace(0 + i * MAX_CHUNK_SIZE, MAX_CHUNK_SIZE * (i + 1));
        }

        if (r != 0) {
            m_newDataQueue.emplace(MAX_CHUNK_SIZE * q, MAX_CHUNK_SIZE * q + r);
        }
    }


    if (srd_session_start(m_srdSession) != SRD_OK) {
	qDebug() << "srd_session_start returned error!";
    } else {
//        qDebug() << "srd_session_start returned SRD_OK";
    }

    if (m_decodeThread) {
        m_decodeThread->join();
        delete m_decodeThread;
    }

    m_decodeCanceled = false;
    m_decodeThread = new std::thread(&AnnotationDecoder::decodeProc, this);

    m_newDataCv.notify_one();
//    qDebug() << "finished start decode";
}

void AnnotationDecoder::stopDecode()
{
//    qDebug() << "stop decoder!";
    m_decodeCanceled = true;
    {
        std::unique_lock<std::mutex> lock(m_newDataMutex);
        m_newDataCv.notify_one();
    }
    if (m_decodeThread) {
        if (m_decodeThread->joinable())
            m_decodeThread->join();
        delete m_decodeThread;
        m_decodeThread = nullptr;
    }

    std::lock_guard<std::mutex> srd_lock(g_sessionMutex);

    if (m_srdSession) {
        srd_session_destroy(m_srdSession);
        m_srdSession = nullptr;
    }

    for (const shared_ptr<logic::Decoder> &dec : m_stack) {
        dec->invalidate_decoder_inst();
    }

//    qDebug() << "finished stop decoder";
}

void AnnotationDecoder::dataAvailable(uint64_t from, uint64_t to)
{
//	Emplace new data segment in a queue as new data might arrive
//	faster than libsigrokdecode can process

	if (from != to) {
		std::unique_lock<std::mutex> lock(m_newDataMutex);


		m_lastSample = to;

		m_newDataQueue.emplace(from, to);
		lock.unlock();
		m_newDataCv.notify_one();
	}
}

std::vector<std::shared_ptr<logic::Decoder> > AnnotationDecoder::getDecoderStack()
{
    return m_stack;
}

void AnnotationDecoder::unassignChannel(uint16_t chId)
{
    if (m_srdSession) {
        m_decodeCanceled = true;
        srd_session_terminate_reset(m_srdSession);
        {
            std::unique_lock<std::mutex> lock(m_newDataMutex);
            m_newDataCv.notify_one();
        }

        srd_session_metadata_set(m_srdSession, SRD_CONF_SAMPLERATE,
                                 g_variant_new_uint64(m_annotationCurve->getSampleRate()));
//                qDebug() << "SampleRate: " << m_annotationCurve->getSampleRate();
        for (const std::shared_ptr<logic::Decoder> &dec : m_stack) {
            dec->apply_all_options();
        }
    }

    for (auto & ch : m_channels) {
        if (ch.id == chId) {
            ch.assigned_signal = false;
            break;
        }
    }

    stackChanged();
    startDecode();
}

std::vector<DecodeChannel *> AnnotationDecoder::getDecoderChannels()
{
    vector<DecodeChannel *> chls;
    for (DecodeChannel &ch : m_channels) {
        chls.push_back(&ch);
    }

    return chls;
}

void AnnotationDecoder::reset()
{
//	std::unique_lock<std::mutex> lock(m_newDataMutex);

	m_lastSample = 0;
	stopDecode();
	stackChanged();
	startDecode();
}

void AnnotationDecoder::assignChannel(uint16_t chId, uint16_t bitId)
{
    if (m_srdSession) {
        m_decodeCanceled = true;
        srd_session_terminate_reset(m_srdSession);
        {
            std::unique_lock<std::mutex> lock(m_newDataMutex);
            m_newDataCv.notify_one();
        }

        srd_session_metadata_set(m_srdSession, SRD_CONF_SAMPLERATE,
                                 g_variant_new_uint64(m_annotationCurve->getSampleRate()));
//                qDebug() << "SampleRate: " << m_annotationCurve->getSampleRate();
        for (const std::shared_ptr<logic::Decoder> &dec : m_stack) {
            dec->apply_all_options();
        }
    }

    qDebug() << "Assigning to chId: " << chId << " bitid: " << bitId;

    for (auto & ch : m_channels) {
        if (ch.id == chId) {
            ch.bit_id = bitId;
            ch.assigned_signal = true;
            break;
        }
    }

    for (auto &ch : m_channels) {
	qDebug() << "ch: " << ch.id << " has ch assigned: " << ch.assigned_signal << " with bitid: " << ch.bit_id;
    }

    stackChanged();
    startDecode();
}

int AnnotationDecoder::getNrOfChannels() const
{
	return m_channels.size();
}

void AnnotationDecoder::stackChanged()
{
    stopDecode();

    std::lock_guard<std::mutex> srd_lock(g_sessionMutex);

    if (srd_session_new(&m_srdSession) != SRD_OK) {
        qDebug() << "srd_session_new returned error!";
    } else {
//        qDebug() << "srd_session_new returned SRD_OK";
    }

    vector<DecodeChannel *> chls;
    for (DecodeChannel &ch : m_channels) {
        chls.push_back(&ch);
    }

    m_stack.front()->set_channels(chls);

    srd_decoder_inst *prev_di = nullptr;
    for (const shared_ptr<logic::Decoder>& dec : m_stack) {
        srd_decoder_inst *const di = dec->create_decoder_inst(m_srdSession);
        if (prev_di)
            srd_inst_stack(m_srdSession, prev_di, di);

        prev_di = di;
    }

    // Map out all annotation classes
    int row_index = 0;
    for (const shared_ptr<logic::Decoder>& dec : m_stack) {
        assert(dec);
        const srd_decoder *const decc = dec->decoder();
        assert(dec->decoder());

        for (const GSList *l = decc->annotation_rows; l; l = l->next) {
            const srd_decoder_annotation_row *const ann_row =
                (srd_decoder_annotation_row *)l->data;
            assert(ann_row);

            const Row row(row_index++, decc, ann_row);

            int count = 0;
            for (const GSList *ll = ann_row->ann_classes;
                ll; ll = ll->next) {
                m_class_rows[std::make_pair(decc,
                    GPOINTER_TO_INT(ll->data))] = row;
                count ++;
            }
        }
    }

    int index = 0;
    for (const shared_ptr<logic::Decoder>& dec : m_stack) {
        assert(dec);
        const srd_decoder *const decc = dec->decoder();
        assert(dec->decoder());

//        int index = 0;
        // Add a row for the decoder if it doesn't have a row list
        if (!decc->annotation_rows)
            (m_annotation_rows)[Row(index++, decc)] = RowData();

        // Add the decoder rows
        for (const GSList *l = decc->annotation_rows; l; l = l->next) {
            const srd_decoder_annotation_row *const ann_row =
                (srd_decoder_annotation_row *)l->data;
            assert(ann_row);

            const Row row(index++, decc, ann_row);

            // Add a new empty row data object
            (m_annotation_rows)[row] = RowData();
        }
    }

    // set curves class rows and annotation rows
    m_annotationCurve->setClassRows(m_class_rows);
    m_annotationCurve->setAnnotationRows(m_annotation_rows);

    // register curve to receive new annotations from libsigrokdecode
    srd_pd_output_callback_add(m_srdSession, SRD_OUTPUT_ANN,
                               AnnotationCurve::annotationCallback, m_annotationCurve);
}

void AnnotationDecoder::decodeProc()
{
    while (!m_decodeCanceled) {

        std::unique_lock<std::mutex> lock(m_newDataMutex);

        // Wait for data
//        if (m_newDataQueue.empty()) {
            m_newDataCv.wait(lock, [&]{return !m_newDataQueue.empty() || m_decodeCanceled;});
//        }

        if (m_decodeCanceled) {
//            qDebug() << "cancel requested!";
            break;
        }

        for (const shared_ptr<logic::Decoder> & dec : m_stack) {
            if (!dec->have_required_channels()) {
//                qDebug() << "not having required channels!";
                // TODO: SET ERROR MESSAGE
                return;
            }
        }

        // TODO: CHECK FOR ERRORS

//        qDebug() << "exit wait!";

        uint64_t start, stop;
        std::tie(start, stop) = m_newDataQueue.front();
        m_newDataQueue.pop();
        lock.unlock(); // unlock to allow new data to enter the queue

        uint64_t chunkSize = stop - start;
        std::unique_ptr<uint16_t []> chunk(new uint16_t[chunkSize]);

        uint16_t *data = m_logic->getData();

	if (!data) {
		continue;
	}

        memcpy(chunk.get(), data + start, chunkSize * sizeof(uint16_t));

//        qDebug() << "send data!";
        std::lock_guard<std::mutex> srd_lock(g_sessionMutex);

        if (srd_session_send(m_srdSession, start, stop, reinterpret_cast<uint8_t*>(
                                 chunk.get()), chunkSize, sizeof(uint16_t)) != SRD_OK) {
//            qDebug() << "No bueno!";
        }

        // Notify curve that annotations are now available to be drawn on the plot
        // srd_session_send blocks untill all samples are processed
        m_annotationCurve->newAnnotations();
    }
}
