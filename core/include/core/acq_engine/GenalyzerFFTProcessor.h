#pragma once

#include "scopy-core_export.h"

#include "DataKey.h"
#include "GenalyzerConfig.h"
#include "ProcessorBlock.h"

#include <cgenalyzer.h>
#include <cgenalyzer_simplified_beta.h>

#include <mutex>
#include <vector>

namespace scopy {
namespace acq {

// ProcessorBlock that computes an FFT using genalyzer, with optional
// Fourier analysis (SFDR/SNR/THD/NSD/...) using either auto or fixed-tone
// configuration. Mode is auto-detected from the watched-keys count:
//
//   watchedKeys.size() == 1  -> Real mode      (gn_rfft, axis [0, fs/2],
//                                               output nfft/2+1 bins,
//                                               GnFreqAxisTypeReal)
//   watchedKeys.size() == 2  -> Complex mode   (gn_fft + gn_ifftshift,
//                                               axis [-fs/2, fs/2),
//                                               output nfft bins,
//                                               GnFreqAxisTypeDcLeft)
//
// Watched keys are read at every process() call so they can be reassigned
// at runtime via setWatchedKeys() — do it while the block is disabled.
//
// Thread safety: every gn_* call (FFT, ifftshift, db, analysis) is
// serialised with s_genalyzerMutex because fftw3 (genalyzer's back-end)
// is not re-entrant across instances.
class SCOPY_CORE_EXPORT GenalyzerFFTProcessor : public ProcessorBlock
{
	Q_OBJECT
public:
	enum class FFTMode { Complex, Real };

	// Complex / I-Q constructor (back-compat).
	explicit GenalyzerFFTProcessor(const DataKey &iKey,
				       const DataKey &qKey,
				       const DataKey &outputKey,
				       const DataKey &freqKey,
				       int            nfft,
				       double         sampleRate = 2.4e6,
				       GnWindow       window     = GnWindowHann,
				       QObject       *parent     = nullptr);

	// Real / single-channel constructor.
	explicit GenalyzerFFTProcessor(const DataKey &inKey,
				       const DataKey &outputKey,
				       const DataKey &freqKey,
				       int            nfft,
				       double         sampleRate = 2.4e6,
				       GnWindow       window     = GnWindowHann,
				       QObject       *parent     = nullptr);

	~GenalyzerFFTProcessor() override;

	// Re-zero working buffers between single() acquisitions.
	void reset() override;

	void process(DataStore *store) override;

	DataKey outputKey()  const { return m_outputKey; }
	DataKey freqKey()    const { return m_freqKey; }
	int     nfft()       const { return m_nfft; }
	double  sampleRate() const { return m_sampleRate; }
	void    setSampleRate(double fs);

	FFTMode mode() const;

	GenalyzerConfig config() const { return m_cfg; }

	QWidget *createSettingsWidget(QWidget *parent = nullptr) override;

public Q_SLOTS:
	void setConfig(const scopy::acq::GenalyzerConfig &cfg);

Q_SIGNALS:
	void analysisReady(const scopy::acq::GenalyzerResultsSnapshot &results);
	void analysisFailed(const QString &reason);

private:
	DataKey  m_outputKey;
	DataKey  m_freqKey;
	int      m_nfft;
	double   m_sampleRate;
	GnWindow m_window;

	// Cached output sizing — depends on mode + nfft.
	int m_outBins{0};       // nfft   (complex)  or  nfft/2 + 1 (real)
	int m_fftOutSize{0};    // 2*nfft (complex)  or  2*(nfft/2+1) (real)

	// Buffers
	std::vector<double> m_iBuf;    // float -> double copy of I channel (complex)
	std::vector<double> m_qBuf;    // float -> double copy of Q channel (complex)
	std::vector<double> m_realIn;  // float -> double copy of real input (real mode)
	std::vector<double> m_fftOut;  // gn_fft / gn_rfft output: interleaved Re/Im
	std::vector<double> m_shifted; // after gn_ifftshift (complex mode only)
	std::vector<double> m_dbBuf;   // after gn_db

	// Cached frequency axis (rebuilt when nfft / sample rate / mode change).
	QVector<float> m_freqAxis;
	FFTMode        m_lastModeForAxis{FFTMode::Complex};

	// Analysis state
	GenalyzerConfig          m_cfg;
	gn_config                m_gnConfig{nullptr};   // for auto-analysis
	char                    *m_faKey{nullptr};      // for fixed-tone analysis
	GenalyzerResultsSnapshot m_lastResults;

	// fftw3 (used internally by genalyzer) must not be called concurrently.
	static std::mutex s_genalyzerMutex;

	void resizeForNfft(int nfft, FFTMode mode);
	void rebuildFreqAxis(FFTMode mode);

	int  runComplexFFT(const QVector<float> &iSamples, const QVector<float> &qSamples);
	int  runRealFFT(const QVector<float> &samples);

	void cleanupAutoConfig();
	void cleanupFaConfig();
	int  configureAutoAnalysis();
	int  configureFixedToneAnalysis();
	void performAnalysis(FFTMode mode);

	void publishResults(size_t results_size, char **rkeys, double *rvalues);
};

} // namespace acq
} // namespace scopy
