#pragma once

#include "scopy-core_export.h"

#include "DataKey.h"
#include "GenalyzerConfig.h"
#include "ProcessorBlock.h"

#include <cgenalyzer.h>
#include <cgenalyzer_simplified_beta.h>

#include <atomic>
#include <mutex>
#include <vector>

namespace scopy {
namespace acq {

// FFT via genalyzer, with optional Fourier analysis (SFDR/SNR/THD/NSD/…) in
// either auto or fixed-tone configuration. The number of watched keys picks the
// transform: one key means real (gn_rfft, axis [0, fs/2), nfft/2+1 bins), two
// means complex I/Q (gn_fft + gn_ifftshift, axis [-fs/2, fs/2), nfft bins).
//
// Watched keys are re-read every process(), so setWatchedKeys() can switch mode
// at runtime — do it while the block is disabled.
//
// Averaging is genalyzer's own: gn_fft/gn_rfft take navg and nfft separately and
// average navg power spectra, so the block asks the DataStore for navg*nfft
// samples rather than accumulating dB frames itself. The extra chunks come from
// the store's history — see setAveraging() — which is why the block stays
// stateless between cycles.
//
// Every gn_* call is serialised on s_genalyzerMutex: fftw3, genalyzer's
// back-end, is not re-entrant even across separate instances.
class SCOPY_CORE_EXPORT GenalyzerFFTProcessor : public ProcessorBlock
{
	Q_OBJECT
public:
	enum class FFTMode { Complex, Real };

	// Complex / I-Q.
	explicit GenalyzerFFTProcessor(const DataKey &iKey,
				       const DataKey &qKey,
				       const DataKey &outputKey,
				       const DataKey &freqKey,
				       int            nfft,
				       double         sampleRate = 2.4e6,
				       GnWindow       window     = GnWindowHann,
				       QObject       *parent     = nullptr);

	// Real / single-channel.
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

	QList<DataKey> outputKeys() const override { return {m_outputKey, m_freqKey}; }

	// The magnitude stream as a curve in dBFS against the frequency stream; the
	// frequency stream itself as Hidden. Both are outputs, but only one is a
	// trace — a bin-frequency ramp is an axis, and drawing it would put a
	// diagonal line across the spectrum.
	QHash<DataKey, StreamInfo> declaredStreams() const override;
	int     nfft()       const { return m_nfft; }
	double  sampleRate() const { return m_sampleRate; }
	void    setSampleRate(double fs);

	// Window function applied by gn_fft/gn_rfft and by the analysis config.
	GnWindow window() const;
	void     setWindow(GnWindow w);

	// Real-mode dBFS reference: what a full-scale sinusoid measures. Ignored in
	// complex mode, where gn_fft has no scaling argument.
	GnRfftScale rfftScale() const;
	void        setRfftScale(GnRfftScale s);

	// Number of power spectra genalyzer averages per output frame; 1 disables
	// averaging. Needs navg*nfft input samples, which come from the DataStore's
	// chunk history, so the block registers a depth claim — hence the store.
	// Pass the store the block will run against (and the engine's buffer size)
	// once, at wiring time; nullptr leaves averaging pinned at 1.
	void setAveragingStore(DataStore *store, std::size_t bufferSize);
	int  averaging() const { return m_navg.load(std::memory_order_relaxed); }
	void setAveraging(int navg);

	FFTMode mode() const;

	GenalyzerConfig config() const { return m_cfg; }

	QWidget *createSettingsWidget(QWidget *parent = nullptr) override;

public Q_SLOTS:
	void setConfig(const scopy::acq::GenalyzerConfig &cfg);

Q_SIGNALS:
	void analysisReady(const scopy::acq::GenalyzerResultsSnapshot &results);
	void analysisFailed(const QString &reason);

	// Emitted when the transform parameters change, so a settings widget shows
	// what the block is actually running with. nfftChanged also fires when
	// process() re-derives nfft from the input length.
	void sampleRateChanged(double fs);
	void nfftChanged(int nfft);
	void averagingChanged(int navg);

private:
	DataKey  m_outputKey;
	DataKey  m_freqKey;
	int      m_nfft;
	double   m_sampleRate;
	GnWindow m_window;
	GnRfftScale m_rfftScale{GnRfftScaleDbfsSin};

	// Averaging. m_navg is atomic so process() can read it without taking the
	// gn_* mutex first; the store pointer and buffer size are set once at wiring
	// time and only read afterwards.
	std::atomic<int> m_navg{1};
	DataStore       *m_avgStore{nullptr};
	std::size_t      m_bufferSize{1};

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
	// navg the staging buffers were last sized for; a change means resize.
	int            m_sizedForNavg{1};

	// Analysis state
	GenalyzerConfig          m_cfg;
	gn_config                m_gnConfig{nullptr};   // for auto-analysis
	char                    *m_faKey{nullptr};      // for fixed-tone analysis
	GenalyzerResultsSnapshot m_lastResults;

	// fftw3 (used internally by genalyzer) must not be called concurrently.
	static std::mutex s_genalyzerMutex;

	// `navg` is passed rather than read from m_navg so one cycle uses a single
	// consistent value: sizing the staging buffers with one navg and indexing
	// them with another that arrived from the GUI mid-cycle overruns them.
	void resizeForNfft(int nfft, FFTMode mode, int navg);
	void rebuildFreqAxis(FFTMode mode);

	// Re-register the history claim for navg*nfft samples on every watched key.
	// Cheap and idempotent: claims are keyed (key, claimant) and replace.
	void reclaimAveragingDepth();

	int  runComplexFFT(const QVector<float> &iSamples, const QVector<float> &qSamples, int navg);
	int  runRealFFT(const QVector<float> &samples, int navg);

	void cleanupAutoConfig();
	void cleanupFaConfig();
	int  configureAutoAnalysis();
	int  configureFixedToneAnalysis();
	void performAnalysis(FFTMode mode);

	void publishResults(size_t results_size, char **rkeys, double *rvalues);
};

} // namespace acq
} // namespace scopy
