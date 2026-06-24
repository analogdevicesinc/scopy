#pragma once

#include <core/acq_engine/SourceBlock.h>
#include <core/acq_engine/DataStore.h>

#include <libm2k/digital/m2kdigital.hpp>

namespace scopy {
namespace adc {
namespace sim {

// SourceBlock that reads the 16 M2K digital channels via libm2k.
//
// Each enabled channel "DIO0" … "DIO15" is written to the DataStore as
// QVector<quint8> (0 or 1 per sample), keyed as:
//   DataKey::raw(id(), "DIO0") … DataKey::raw(id(), "DIO15")
//
// Acquisition mirrors logic_analyzer.cpp startStop():
//   onStart  → setSampleRateIn + startAcquisition
//   acquire  → getSamplesP (blocking), bit-extract per channel, write to store
//   onStop   → cancelAcquisition + stopAcquisition
//
// Typical usage:
//   auto *src = new M2kLogicSource(m2k->getDigital(), "m2k_logic", parent);
//   src->enableChannel("DIO0", true);
//   src->enableChannel("DIO1", true);
//   src->setBufferSize(1024);
//   engine->addSource(src);
//   engine->run();
class M2kLogicSource : public scopy::acq::SourceBlock
{
	Q_OBJECT
public:
	static constexpr int NR_CHANNELS = 16;

	explicit M2kLogicSource(libm2k::digital::M2kDigital *digital,
				const QString &id     = "m2k_logic",
				QObject       *parent = nullptr);
	~M2kLogicSource() override;

	// Called on the main thread before the worker starts.
	void onStart() override;

	// Called on the main thread when stopping.
	// Cancels any blocked getSamplesP() call.
	void onStop() override;

	// Called on the worker thread. Blocks until one buffer is ready,
	// then writes each enabled channel as QVector<quint8> to the store.
	void acquire(scopy::acq::DataStore *store) override;

	// Sample rate passed to setSampleRateIn(). Default 1 Msps.
	void   setSampleRate(double sr) { m_sampleRate = sr; }
	double sampleRate() const { return m_sampleRate; }

private:
	libm2k::digital::M2kDigital *m_digital;
	double                        m_sampleRate{1e6};
};

} // namespace sim
} // namespace adc
} // namespace scopy
