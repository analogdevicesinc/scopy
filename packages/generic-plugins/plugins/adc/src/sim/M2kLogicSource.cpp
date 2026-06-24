#include "M2kLogicSource.h"

#include <core/acq_engine/DataKey.h>

#include <libm2k/m2kexceptions.hpp>

#include <QDebug>

namespace scopy {
namespace adc {
namespace sim {

M2kLogicSource::M2kLogicSource(libm2k::digital::M2kDigital *digital,
				const QString &id,
				QObject       *parent)
	: SourceBlock(id, parent)
	, m_digital(digital)
{
	// Register all 16 digital channels (disabled by default)
	for(int ch = 0; ch < NR_CHANNELS; ++ch)
		m_channels.insert(QString("DIO%1").arg(ch), false);
}

M2kLogicSource::~M2kLogicSource()
{
	onStop();
}

void M2kLogicSource::onStart()
{
	SourceBlock::onStart();

	m_stopRequested = false;

	try {
		m_digital->setSampleRateIn(m_sampleRate);
		m_digital->startAcquisition(m_bufferSize);
	} catch(libm2k::m2k_exception &e) {
		throw std::runtime_error(
			QString("M2kLogicSource::onStart: %1").arg(e.what()).toStdString());
	}
}

void M2kLogicSource::onStop()
{
	m_stopRequested = true;

	try {
		// m_digital->cancelAcquisition(); // THIS CAUSES SEG FAULT
	} catch(...) {}

	try {
		m_digital->stopAcquisition();
	} catch(...) {}
}

void M2kLogicSource::acquire(scopy::acq::DataStore *store)
{
	if(m_stopRequested)
		return;

	const uint16_t *raw = nullptr;
	try {
		raw = m_digital->getSamplesP(m_bufferSize);
	} catch(libm2k::m2k_exception &e) {
		if(m_stopRequested)
			return;
		throw std::runtime_error(
			QString("M2kLogicSource::acquire: %1").arg(e.what()).toStdString());
	}

	if(!raw || m_stopRequested)
		return;

	for(int ch = 0; ch < NR_CHANNELS; ++ch) {
		const QString chId = QString("DIO%1").arg(ch);

		if(!isChannelEnabled(chId))
			continue;

		QVector<quint8> samples(static_cast<int>(m_bufferSize));
		for(std::size_t i = 0; i < m_bufferSize; ++i)
			samples[static_cast<int>(i)] = (raw[i] >> ch) & 1u;

		store->write(scopy::acq::DataKey::raw(m_id, chId), std::move(samples));
	}
}

} // namespace sim
} // namespace adc
} // namespace scopy
