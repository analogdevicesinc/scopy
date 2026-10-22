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

#include "externaldecoderprocessor.h"

#include "datastore.h"

#include <QMutexLocker>

#include <core/decoder/annotationsymbols.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace scopy {
namespace acq {

namespace {

// Pack per-bit 0/1 vectors LSB-first: unitsize = ceil(numChannels/8),
// channel i -> bit i. `out` is reused across cycles.
void packBits(const std::vector<QVector<quint8>> &bitVecs, int numChannels, qsizetype nSamples, int unitsize,
	      std::vector<uint8_t> &out)
{
	out.assign(static_cast<std::size_t>(nSamples) * unitsize, 0);
	const qsizetype nBits = std::min<qsizetype>(bitVecs.size(), numChannels);
	for(qsizetype s = 0; s < nSamples; ++s) {
		uint8_t *dst = out.data() + static_cast<std::size_t>(s) * unitsize;
		for(qsizetype b = 0; b < nBits; ++b) {
			const QVector<quint8> &src = bitVecs[b];
			if(src.size() <= s)
				continue;
			if(src[s])
				dst[b / 8] |= static_cast<uint8_t>(1u << (b % 8));
		}
	}
}

// DataStore Annotation -> backend AnnotationC (single-stream input).
void toAnnotationC(const QVector<Annotation> &in, std::vector<scopy::decoder::AnnotationC> &out)
{
	out.clear();
	out.reserve(static_cast<std::size_t>(in.size()));
	for(const Annotation &a : in) {
		scopy::decoder::AnnotationC c;
		c.start = a.startSample;
		c.end = a.endSample;
		c.decoder = a.decoder.toStdString();
		c.klass = a.klass.toStdString();
		c.text = a.text.toStdString();
		c.severity = a.severity;
		c.stageIndex = 0;
		c.hasValue = a.value.has_value();
		c.value = a.value.value_or(0);
		out.push_back(std::move(c));
	}
}

// Backend output -> per-stage Annotation buckets. Out-of-range stage indices
// are dropped. `shift` right-anchors samples inside the plot window.
void toPerStage(const std::vector<scopy::decoder::AnnotationC> &outC, qint64 shift,
		std::vector<QVector<Annotation>> &perStage)
{
	const int nStages = static_cast<int>(perStage.size());
	for(const auto &a : outC) {
		if(a.stageIndex < 0 || a.stageIndex >= nStages)
			continue;
		Annotation out;
		out.startSample = static_cast<quint64>(static_cast<qint64>(a.start) + shift);
		out.endSample = static_cast<quint64>(static_cast<qint64>(a.end) + shift);
		out.decoder = QString::fromStdString(a.decoder);
		out.klass = QString::fromStdString(a.klass);
		out.text = QString::fromStdString(a.text);
		out.severity = a.severity;
		if(a.hasValue)
			out.value = a.value;
		perStage[static_cast<std::size_t>(a.stageIndex)].append(out);
	}
}

// How a stage prints its payload text. sigrok PDs that can print in several
// radices expose it as the "format" option and default to hex; a PD with no
// such option always prints hex. Asking the *producer* is the only reliable
// answer — "105" is 0x69 in decimal and 0x105 in hex, and a consumer guessing
// wrong corrupts every byte without an error.
TextRadix stageTextRadix(const scopy::decoder::DecoderStage &stage)
{
	TextRadix r = TextRadix::Hex;
	auto it = stage.options.find("format");
	if(it != stage.options.end())
		(void)textRadixFromString(QString::fromStdString(it->second), r);
	return r;
}

// Line rate a stage declares, if any. 0 = not applicable.
quint64 stageBitrate(const scopy::decoder::DecoderStage &stage)
{
	for(const char *key : {"baudrate", "bitrate"}) {
		auto it = stage.options.find(key);
		if(it == stage.options.end())
			continue;
		bool ok = false;
		const quint64 v = QString::fromStdString(it->second).toULongLong(&ok);
		if(ok)
			return v;
	}
	return 0;
}

} // namespace

ExternalDecoderProcessor::ExternalDecoderProcessor(const QString &name,
						   std::unique_ptr<scopy::decoder::IDecoderBackend> backend,
						   QObject *parent)
	: ProcessorBlock(name, parent)
	, m_backend(std::move(backend))
{}

ExternalDecoderProcessor::~ExternalDecoderProcessor() = default;

void ExternalDecoderProcessor::setConfig(const scopy::decoder::DecoderConfig &cfg)
{
	QMutexLocker lk(&m_inMutex);
	m_in.cfg = cfg;
}

scopy::decoder::DecoderConfig ExternalDecoderProcessor::config() const
{
	QMutexLocker lk(&m_inMutex);
	return m_in.cfg;
}

void ExternalDecoderProcessor::setRawInput(const scopy::decoder::DecoderConfig &cfg, const QList<DataKey> &rawKeys)
{
	QMutexLocker lk(&m_inMutex);
	m_in.cfg = cfg;
	m_in.rawKeys = rawKeys;
}

void ExternalDecoderProcessor::setAnnotationInputKey(const DataKey &k)
{
	QMutexLocker lk(&m_inMutex);
	m_in.annInKey = k;
}

DataKey ExternalDecoderProcessor::annotationInputKey() const
{
	QMutexLocker lk(&m_inMutex);
	return m_in.annInKey;
}

void ExternalDecoderProcessor::setOutputKeys(const QList<DataKey> &keys)
{
	QMutexLocker lk(&m_inMutex);
	m_in.outKeys = keys;
}

QList<DataKey> ExternalDecoderProcessor::outputKeys() const
{
	QMutexLocker lk(&m_inMutex);
	return m_in.outKeys;
}

double ExternalDecoderProcessor::sampleRate() const
{
	QMutexLocker lk(&m_inMutex);
	return m_in.cfg.sampleRate;
}

void ExternalDecoderProcessor::setWindowSize(int n)
{
	QMutexLocker lk(&m_inMutex);
	m_in.windowSize = n;
}

int ExternalDecoderProcessor::windowSize() const
{
	QMutexLocker lk(&m_inMutex);
	return m_in.windowSize;
}

ExternalDecoderProcessor::Inputs ExternalDecoderProcessor::inputs() const
{
	QMutexLocker lk(&m_inMutex);
	return m_in;
}

void ExternalDecoderProcessor::reset() {}

std::optional<StreamInfo> ExternalDecoderProcessor::streamInfo(const DataKey &key) const
{
	// Empty slots are skipped rather than matched: an unconfigured stage has no key,
	// and an empty query key must not collide with it.
	if(key.toString().isEmpty())
		return std::nullopt;

	const Inputs in = inputs();
	const int i = in.outKeys.indexOf(key);
	if(i < 0)
		return std::nullopt;

	StreamInfo info;
	info.kind = ReprKind::Annotations;
	info.sampleRate = in.cfg.sampleRate;
	// The stage's decoder id where there is one — "uart", "i2c" — since that is
	// what a reader recognises. Falls back to the key, which the view does too.
	if(static_cast<std::size_t>(i) < in.cfg.stack.size())
		info.label = QString::fromStdString(in.cfg.stack[static_cast<std::size_t>(i)].decoderId);
	// No xKey and no colorIndex: annotation offsets are relative to the window the
	// decode ran on, so they index the shared ramp, and AnnotationCurve colours
	// per annotation class rather than per stream.
	return info;
}

// Declares, per output key, how to read that stage's annotations: which
// decoder produced them, what radix `text` is in, and the timeline they sit
// on. A downstream decoder taking this stream as input reads it from the store
// instead of being configured by hand — the same fields a vendor library
// publishes, so consumers need no per-producer knowledge.
void ExternalDecoderProcessor::publishStreamInfo(const Inputs &in, DataStore *store)
{
	for(int i = 0; i < in.outKeys.size(); ++i) {
		if(in.outKeys[i].toString().isEmpty())
			continue;
		if(static_cast<std::size_t>(i) >= in.cfg.stack.size())
			continue;
		const scopy::decoder::DecoderStage &stage = in.cfg.stack[static_cast<std::size_t>(i)];

		AnnotationStreamInfo info;
		info.producerId = QString::fromStdString(stage.decoderId);
		info.textRadix = stageTextRadix(stage);
		info.sampleRate = in.cfg.sampleRate;
		info.bitrate = stageBitrate(stage);
		store->setAnnotationInfo(in.outKeys[i], info);
	}
}

// Hands each stage's bucket to the DataStore. Always called, even on an empty
// or failed decode, so consumers see a defined state every cycle.
void ExternalDecoderProcessor::publish(const Inputs &in, DataStore *store, std::vector<QVector<Annotation>> &perStage)
{
	for(int i = 0; i < in.outKeys.size(); ++i) {
		if(in.outKeys[i].toString().isEmpty())
			continue;
		store->write(in.outKeys[i], std::move(perStage[static_cast<std::size_t>(i)]));
		Q_EMIT cycleProduced(in.outKeys[i]);
	}
}

void ExternalDecoderProcessor::process(DataStore *store)
{
	if(!m_backend || !store)
		return;

	// One snapshot for the whole cycle: the GUI may change any of it mid-run, and the
	// cycle must decode, describe and publish against the same config.
	const Inputs in = inputs();

	// Cheap and idempotent; done here so a config change between cycles is
	// picked up without the manager having to remember to re-announce.
	publishStreamInfo(in, store);

	std::vector<QVector<Annotation>> perStage(static_cast<std::size_t>(in.outKeys.size()));
	if(in.cfg.rootInput == scopy::decoder::RootInput::Annotations)
		decodeAnnotationInput(in, store, perStage);
	else
		decodeLogicInput(in, store, perStage);
	publish(in, store, perStage);
}

void ExternalDecoderProcessor::decodeAnnotationInput(const Inputs &in, DataStore *store,
						     std::vector<QVector<Annotation>> &perStage)
{
	if(in.annInKey.toString().isEmpty()) {
		// Same per-cycle misconfiguration shape as "no rawKeys set" below.
		reportOnce(AcquisitionError::Severity::Warning,
			   QStringLiteral("rootInput=Annotations but no annInKey set"));
		return;
	}

	const auto annIn = store->latestAs<QVector<Annotation>>(in.annInKey);
	if(!annIn) {
		if(store->contains(in.annInKey))
			// A wrong stream type does not fix itself between cycles either. The key
			// is in the message, so retargeting to a different bad key still reports.
			reportOnce(
				AcquisitionError::Severity::Warning,
				QStringLiteral("annInKey %1 is not an annotation stream").arg(in.annInKey.toString()));
		return;
	}
	if(annIn->isEmpty())
		return;

	std::vector<scopy::decoder::AnnotationC> inC;
	toAnnotationC(*annIn, inC);

	// Layer the producer's own stream descriptor under the config. Any
	// "annIn.*" key set explicitly still wins, so a misreporting producer can
	// be overridden by hand, but the default needs no user configuration.
	scopy::decoder::DecoderConfig cfg = in.cfg;
	if(auto info = store->annotationInfo(in.annInKey)) {
		std::map<std::string, std::string> declared;
		scopy::decoder::streamInfoToMeta(*info, declared);
		for(const auto &kv : declared)
			cfg.meta.insert(kv); // insert(): existing keys are left alone
	} else if(wantsReport(AcquisitionError::Severity::Info)) {
		report(AcquisitionError::Severity::Info,
		       QStringLiteral("annInKey %1 has no stream descriptor; "
				      "falling back to annIn.* config")
			       .arg(in.annInKey.toString()));
	}

	std::vector<scopy::decoder::AnnotationC> outC;
	if(!m_backend->decodeAnnotations(cfg, inC, outC))
		// A backend that fails usually keeps failing the same way — a missing
		// binary, a bad decoder id. The error text is the identity, so a genuinely
		// different failure still reports.
		reportOnce(AcquisitionError::Severity::Warning,
			   QStringLiteral("backend decodeAnnotations failed: %1")
				   .arg(QString::fromStdString(m_backend->lastError())));

	toPerStage(outC, /*shift=*/0, perStage);

	if(wantsReport(AcquisitionError::Severity::Info))
		report(AcquisitionError::Severity::Info,
		       QStringLiteral("annIn=%1 -> %2 stages, %3 output anns")
			       .arg(annIn->size())
			       .arg(perStage.size())
			       .arg(outC.size()));
}

void ExternalDecoderProcessor::decodeLogicInput(const Inputs &in, DataStore *store,
						std::vector<QVector<Annotation>> &perStage)
{
	if(in.rawKeys.isEmpty()) {
		// reportOnce: this holds until the reader configures the decoder, so at
		// cycle rate it was a warning per frame for as long as it stayed unconfigured.
		reportOnce(AcquisitionError::Severity::Warning, QStringLiteral("no rawKeys set"));
		return;
	}

	// Pull per-bit vectors; minLen = shortest populated channel.
	std::vector<QVector<quint8>> bitVecs(static_cast<std::size_t>(in.rawKeys.size()));
	qsizetype minLen = std::numeric_limits<qsizetype>::max();
	for(int i = 0; i < in.rawKeys.size(); ++i) {
		const DataKey &key = in.rawKeys[i];
		QVector<quint8> vec = in.windowSize > 0
			? store->windowAs<QVector<quint8>>(key, in.windowSize)
			: store->latestAs<QVector<quint8>>(key).value_or(QVector<quint8>{});
		if(vec.isEmpty()) {
			if(store->contains(key) && store->typeOf(key) != SampleType::UInt8)
				// Steady state for as long as that key stays mapped.
				reportOnce(AcquisitionError::Severity::Warning,
					   QStringLiteral("key %1 is not a bit stream").arg(key.toString()));
			continue;
		}
		minLen = std::min(minLen, vec.size());
		bitVecs[static_cast<std::size_t>(i)] = std::move(vec);
	}

	if(minLen == std::numeric_limits<qsizetype>::max() || minLen <= 0)
		return;

	const int unitsize = std::max(1, static_cast<int>(std::ceil(in.cfg.numChannels / 8.0)));
	packBits(bitVecs, in.cfg.numChannels, minLen, unitsize, m_packed);

	if(wantsReport(AcquisitionError::Severity::Info))
		report(AcquisitionError::Severity::Info,
		       QStringLiteral("decode %1 samples, unitsize=%2 bytes, totalBytes=%3")
			       .arg(minLen)
			       .arg(unitsize)
			       .arg(m_packed.size()));

	std::vector<scopy::decoder::AnnotationC> outC;
	if(!m_backend->decode(in.cfg, m_packed.data(), static_cast<std::size_t>(minLen), outC))
		// As above: same failure every cycle until the cause is fixed.
		reportOnce(AcquisitionError::Severity::Warning,
			   QStringLiteral("backend decode failed: %1")
				   .arg(QString::fromStdString(m_backend->lastError())));

	// A short chunk sits at the right edge of the plot window, so annotation
	// sample indices shift by the unfilled remainder.
	const qint64 shift = in.windowSize > 0 ? std::max<qint64>(0, static_cast<qint64>(in.windowSize) - minLen) : 0;
	toPerStage(outC, shift, perStage);
}

} // namespace acq
} // namespace scopy

#include "moc_externaldecoderprocessor.cpp"
