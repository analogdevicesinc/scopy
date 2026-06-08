#pragma once

#include "DataKey.h"
#include "ProcessorBlock.h"

#include <cgenalyzer.h>
#include <mutex>
#include <vector>

namespace scopy {
namespace adc {
namespace sim {

// ProcessorBlock that computes a complex FFT of an I/Q pair using genalyzer's
// gn_fft, centres DC via gn_ifftshift, and converts to dBFS via gn_db.
//
// Watched keys  : iKey (in-phase) + qKey (quadrature) — both QVector<float>
// Output key    : outputKey — QVector<float> of nfft magnitude bins in dBFS,
//                 DC-centred (bin 0 = -Fs/2, bin nfft/2 = DC, bin nfft-1 = +Fs/2)
//
// Thread safety : genalyzer's fftw3 back-end is not re-entrant; s_fftMutex
//                 serialises all gn_fft calls across every instance of this class.
class GenalyzerFFTProcessor : public ProcessorBlock
{
	Q_OBJECT
public:
	explicit GenalyzerFFTProcessor(const DataKey &iKey,
				       const DataKey &qKey,
				       const DataKey &outputKey,
				       const DataKey &freqKey,
				       int nfft,
				       double sampleRate = 2.4e6,
				       GnWindow window   = GnWindowHann,
				       QObject  *parent  = nullptr);

	~GenalyzerFFTProcessor() override = default;

	// Re-zero working buffers between single() acquisitions.
	void reset() override;

	void process(DataStore *store) override;

	DataKey outputKey()  const { return m_outputKey; }
	DataKey freqKey()    const { return m_freqKey; }
	int     nfft()       const { return m_nfft; }
	double  sampleRate() const { return m_sampleRate; }
	void    setSampleRate(double fs) { m_sampleRate = fs; }

private:
	DataKey  m_outputKey;
	DataKey  m_freqKey;
	int      m_nfft;
	double   m_sampleRate;
	GnWindow m_window;

	std::vector<double> m_iBuf;    // float → double copy of I channel
	std::vector<double> m_qBuf;    // float → double copy of Q channel
	std::vector<double> m_fftOut;  // gn_fft output: interleaved Re/Im (2*nfft)
	std::vector<double> m_shifted; // after gn_ifftshift (2*nfft)
	std::vector<double> m_dbBuf;   // after gn_db (nfft)

	// Cached frequency axis: f[i] = i*(sampleRate/nfft) - sampleRate/2
	// Rebuilt whenever nfft or sampleRate changes.
	QVector<float> m_freqAxis;

	// fftw3 (used internally by genalyzer) must not be called concurrently.
	static std::mutex s_fftMutex;

	void rebuildFreqAxis();
};

} // namespace sim
} // namespace adc
} // namespace scopy
