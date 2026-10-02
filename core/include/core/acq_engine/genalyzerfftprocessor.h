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

#pragma once

#include "scopy-core_export.h"

#include "datakey.h"
#include "genalyzerconfig.h"
#include "processorblock.h"

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
// WHICH STREAMS ARE THE INPUTS IS A READER DECISION, not something derived from
// a source's channel names. setInputs() retargets both the keys and the mode at
// runtime, and the settings widget exposes it — so no caller has to find an I/Q
// pair by naming convention, and a block constructed with no inputs at all is a
// valid starting state (process() skips the cycle and says so once). The output
// keys are fixed for the block's life by contrast: a plot channel is created
// against one, so retargeting it under a drawn curve would strand it.
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
	enum class FFTMode
	{
		// No inputs picked. A state rather than an error: a block whose reader
		// has not chosen its streams yet is normal, and the widget shows it.
		None,
		Complex,
		Real
	};

	// No inputs yet: the reader picks them in the settings widget. Prefer this
	// over the two below wherever the host would otherwise have to guess which
	// streams are the FFT's inputs — it has no guess to make, and the block says
	// so on the rail instead of silently never running.
	explicit GenalyzerFFTProcessor(const DataKey &outputKey, const DataKey &freqKey, int nfft,
				       double sampleRate = 2.4e6, GnWindow window = GnWindowHann,
				       QObject *parent = nullptr);

	// Complex / I-Q.
	explicit GenalyzerFFTProcessor(const DataKey &iKey, const DataKey &qKey, const DataKey &outputKey,
				       const DataKey &freqKey, int nfft, double sampleRate = 2.4e6,
				       GnWindow window = GnWindowHann, QObject *parent = nullptr);

	// Real / single-channel.
	explicit GenalyzerFFTProcessor(const DataKey &inKey, const DataKey &outputKey, const DataKey &freqKey, int nfft,
				       double sampleRate = 2.4e6, GnWindow window = GnWindowHann,
				       QObject *parent = nullptr);

	~GenalyzerFFTProcessor() override;

	// --- inputs ----------------------------------------------------------
	//
	// Which streams the transform reads, and with them the mode: empty = no
	// inputs (the block idles), one key = real, two = complex I/Q. A list
	// longer than two is rejected rather than truncated — dropping the extra
	// keys would run a transform the reader did not ask for.
	//
	// NOT SAFE WHILE THE BLOCK IS RUNNING AND ENABLED. The engine reads
	// watchedKeys() on the worker thread to schedule this block, and it only
	// skips that read for a disabled block — which is what makes "disable, then
	// retarget" the contract rather than a suggestion. GenalyzerInputSettings
	// enforces it by greying the pickers while the engine runs with the block
	// enabled; a programmatic caller has to arrange the same.
	//
	// Moves the averaging depth claims with the keys, so a key this block no
	// longer watches stops pinning history.
	void setInputs(const QList<DataKey> &keys);
	QList<DataKey> inputs() const { return m_watchedKeys; }
	bool hasInputs() const { return !m_watchedKeys.isEmpty(); }

	// Routed through setInputs(), so the base-class entry point gets the mode
	// switch, the re-claim and the signal rather than just assigning the list.
	void setWatchedKeys(const QList<DataKey> &keys) override { setInputs(keys); }

	// Re-zero working buffers between single() acquisitions.
	void reset() override;

	void process(DataStore *store) override;

	DataKey outputKey() const { return m_outputKey; }
	DataKey freqKey() const { return m_freqKey; }

	QList<DataKey> outputKeys() const override { return {m_outputKey, m_freqKey}; }

	// The magnitude stream as a curve in dBFS against the frequency stream; the
	// frequency stream itself as Hidden. Both are outputs, but only one is a
	// trace — a bin-frequency ramp is an axis, and drawing it would put a
	// diagonal line across the spectrum.
	std::optional<StreamInfo> streamInfo(const DataKey &key) const override;
	int nfft() const { return m_nfft; }
	double sampleRate() const { return m_sampleRate; }
	void setSampleRate(double fs);

	// Window function applied by gn_fft/gn_rfft and by the analysis config.
	GnWindow window() const;
	void setWindow(GnWindow w);

	// Real-mode dBFS reference: what a full-scale sinusoid measures. Ignored in
	// complex mode, where gn_fft has no scaling argument.
	GnRfftScale rfftScale() const;
	void setRfftScale(GnRfftScale s);

	// Number of power spectra genalyzer averages per output frame; 1 disables
	// averaging. Needs navg past chunks, which come from the DataStore's chunk
	// history, so the block registers a depth claim — hence the store. Pass the
	// store the block will run against once, at wiring time; nullptr leaves
	// averaging pinned at 1.
	void setAveragingStore(DataStore *store);
	int averaging() const { return m_navg.load(std::memory_order_relaxed); }
	void setAveraging(int navg);

	FFTMode mode() const;

	GenalyzerConfig config() const { return m_cfg; }

	QWidget *createSettingsWidget(QWidget *parent = nullptr) override;

public Q_SLOTS:
	void setConfig(const scopy::acq::GenalyzerConfig &cfg);

Q_SIGNALS:
	void analysisReady(const scopy::acq::GenalyzerResultsSnapshot &results);
	void analysisFailed(const QString &reason);

	// GenalyzerConfig::enabled, broken out because a results view has to appear and
	// disappear with it and has no reason to care about the rest of the config.
	// Emitted from whichever thread called setConfig().
	void analysisEnabledChanged(bool en);

	// Emitted when the transform parameters change, so a settings widget shows
	// what the block is actually running with. nfftChanged also fires when
	// process() re-derives nfft from the input length.
	void sampleRateChanged(double fs);
	void nfftChanged(int nfft);
	void averagingChanged(int navg);

	// setInputs() accepted a new key list. Carries the keys rather than just
	// announcing a change: the mode is a function of their count, so a reader
	// that has these needs no follow-up call. Emitted from the calling thread.
	void inputsChanged(const QList<scopy::acq::DataKey> &keys);

private:
	DataKey m_outputKey;
	DataKey m_freqKey;
	int m_nfft;
	double m_sampleRate;
	GnWindow m_window;
	GnRfftScale m_rfftScale{GnRfftScaleDbfsSin};

	// Averaging. m_navg is atomic so process() can read it without taking the
	// gn_* mutex first; the store pointer is set once at wiring time and only
	// read afterwards.
	std::atomic<int> m_navg{1};
	DataStore *m_avgStore{nullptr};

	// Cached output sizing — depends on mode + nfft.
	int m_outBins{0};    // nfft   (complex)  or  nfft/2 + 1 (real)
	int m_fftOutSize{0}; // 2*nfft (complex)  or  2*(nfft/2+1) (real)

	// Buffers
	std::vector<double> m_iBuf;    // float -> double copy of I channel (complex)
	std::vector<double> m_qBuf;    // float -> double copy of Q channel (complex)
	std::vector<double> m_realIn;  // float -> double copy of real input (real mode)
	std::vector<double> m_fftOut;  // gn_fft / gn_rfft output: interleaved Re/Im
	std::vector<double> m_shifted; // after gn_ifftshift (complex mode only)
	std::vector<double> m_dbBuf;   // after gn_db

	// Cached frequency axis (rebuilt when nfft / sample rate / mode change).
	QVector<float> m_freqAxis;
	FFTMode m_lastModeForAxis{FFTMode::Complex};
	// navg the staging buffers were last sized for; a change means resize.
	int m_sizedForNavg{1};
	// Frames the last transform actually averaged, which is below navg while the
	// chunk history fills. The analysis config is built from it, so a change
	// invalidates that config.
	int m_configuredFrames{0};

	// Analysis state
	GenalyzerConfig m_cfg;
	gn_config m_gnConfig{nullptr}; // for auto-analysis
	char *m_faKey{nullptr};	       // for fixed-tone analysis
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

	int runComplexFFT(const QVector<float> &iSamples, const QVector<float> &qSamples, int navg);
	int runRealFFT(const QVector<float> &samples, int navg);

	void cleanupAutoConfig();
	void cleanupFaConfig();
	int configureAutoAnalysis();
	int configureFixedToneAnalysis();
	void performAnalysis(FFTMode mode);

	void publishResults(size_t results_size, char **rkeys, double *rvalues);
};

} // namespace acq
} // namespace scopy
