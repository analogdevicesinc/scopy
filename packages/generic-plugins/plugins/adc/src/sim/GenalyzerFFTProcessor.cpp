#include "GenalyzerFFTProcessor.h"
#include "DataStore.h"

#include <cgenalyzer.h>
#include <stdexcept>
#include <variant>

namespace scopy {
namespace adc {
namespace sim {

std::mutex GenalyzerFFTProcessor::s_fftMutex;

GenalyzerFFTProcessor::GenalyzerFFTProcessor(const DataKey &iKey,
					     const DataKey &qKey,
					     const DataKey &outputKey,
					     const DataKey &freqKey,
					     int nfft,
					     double sampleRate,
					     GnWindow window,
					     QObject *parent)
	: ProcessorBlock("genalyzer-fft", parent)
	, m_outputKey(outputKey)
	, m_freqKey(freqKey)
	, m_nfft(nfft)
	, m_sampleRate(sampleRate)
	, m_window(window)
{
	m_watchedKeys = {iKey, qKey};
	reset();
}

void GenalyzerFFTProcessor::rebuildFreqAxis()
{
	const double rbw = m_sampleRate / m_nfft;
	m_freqAxis.resize(m_nfft);
	for(int i = 0; i < m_nfft; ++i)
		m_freqAxis[i] = static_cast<float>(i * rbw - m_sampleRate / 2.0);
}

void GenalyzerFFTProcessor::reset()
{
	m_iBuf.assign(m_nfft, 0.0);
	m_qBuf.assign(m_nfft, 0.0);
	m_fftOut.assign(2 * m_nfft, 0.0);
	m_shifted.assign(2 * m_nfft, 0.0);
	m_dbBuf.assign(m_nfft, 0.0);
	rebuildFreqAxis();
}

void GenalyzerFFTProcessor::process(DataStore *store)
{
	const DataKey &iKey = m_watchedKeys[0];
	const DataKey &qKey = m_watchedKeys[1];

	const SampleBuffer iBuf = store->read(iKey);
	const SampleBuffer qBuf = store->read(qKey);

	if(iBuf.empty() || qBuf.empty())
		return;

	const auto &iVar = iBuf.sample(0);
	const auto &qVar = qBuf.sample(0);

	if(!std::holds_alternative<QVector<float>>(iVar) ||
	   !std::holds_alternative<QVector<float>>(qVar))
		return;

	const QVector<float> &iSamples = std::get<QVector<float>>(iVar);
	const QVector<float> &qSamples = std::get<QVector<float>>(qVar);

	const int n = qMin(iSamples.size(), qSamples.size());
	if(n <= 0)
		return;

	if(n != m_nfft) {
		m_nfft = n;
		m_iBuf.resize(m_nfft);
		m_qBuf.resize(m_nfft);
		m_fftOut.resize(2 * m_nfft);
		m_shifted.resize(2 * m_nfft);
		m_dbBuf.resize(m_nfft);
		rebuildFreqAxis();
	}

	for(int i = 0; i < m_nfft; ++i) {
		m_iBuf[i] = static_cast<double>(iSamples[i]);
		m_qBuf[i] = static_cast<double>(qSamples[i]);
	}

	{
		std::lock_guard<std::mutex> lock(s_fftMutex);

		int err = gn_fft(m_fftOut.data(), 2 * m_nfft,
				 m_iBuf.data(), m_nfft,
				 m_qBuf.data(), m_nfft,
				 1, m_nfft, m_window);
		if(err != 0)
			throw std::runtime_error(
				QString("gn_fft failed (error %1)").arg(err).toStdString());

		err = gn_ifftshift(m_shifted.data(), 2 * m_nfft,
				   m_fftOut.data(), 2 * m_nfft);
		if(err != 0)
			throw std::runtime_error(
				QString("gn_ifftshift failed (error %1)").arg(err).toStdString());

		err = gn_db(m_dbBuf.data(), m_nfft,
			    m_shifted.data(), 2 * m_nfft);
		if(err != 0)
			throw std::runtime_error(
				QString("gn_db failed (error %1)").arg(err).toStdString());
	}

	QVector<float> out(m_nfft);
	for(int i = 0; i < m_nfft; ++i)
		out[i] = static_cast<float>(m_dbBuf[i]);

	store->write(m_outputKey, std::move(out));
	store->write(m_freqKey, m_freqAxis);
}

} // namespace sim
} // namespace adc
} // namespace scopy
