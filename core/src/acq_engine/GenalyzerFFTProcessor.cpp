#include "GenalyzerFFTProcessor.h"

#include "AcquisitionError.h"
#include "DataStore.h"
#include "GenalyzerSettings.h"

#include <QPushButton>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

#include <cstdlib>
#include <cstring>
#include <variant>

namespace scopy {
namespace acq {

std::mutex GenalyzerFFTProcessor::s_genalyzerMutex;

namespace {
// Register snapshot type with Qt meta-system once per process so it can be
// passed across queued signal/slot connections.
struct MetatypeRegistrar
{
	MetatypeRegistrar()
	{
		qRegisterMetaType<scopy::acq::GenalyzerConfig>("scopy::acq::GenalyzerConfig");
		qRegisterMetaType<scopy::acq::GenalyzerResultsSnapshot>(
			"scopy::acq::GenalyzerResultsSnapshot");
	}
};
static MetatypeRegistrar s_metatypeRegistrar;
} // namespace

// ---------------------------------------------------------------------------
// Construction / destruction
// ---------------------------------------------------------------------------

GenalyzerFFTProcessor::GenalyzerFFTProcessor(const DataKey &iKey,
					     const DataKey &qKey,
					     const DataKey &outputKey,
					     const DataKey &freqKey,
					     int            nfft,
					     double         sampleRate,
					     GnWindow       window,
					     QObject       *parent)
	: ProcessorBlock("genalyzer-fft", parent)
	, m_outputKey(outputKey)
	, m_freqKey(freqKey)
	, m_nfft(nfft)
	, m_sampleRate(sampleRate)
	, m_window(window)
{
	m_watchedKeys = {iKey, qKey};
	resizeForNfft(m_nfft, FFTMode::Complex);
}

GenalyzerFFTProcessor::GenalyzerFFTProcessor(const DataKey &inKey,
					     const DataKey &outputKey,
					     const DataKey &freqKey,
					     int            nfft,
					     double         sampleRate,
					     GnWindow       window,
					     QObject       *parent)
	: ProcessorBlock("genalyzer-fft", parent)
	, m_outputKey(outputKey)
	, m_freqKey(freqKey)
	, m_nfft(nfft)
	, m_sampleRate(sampleRate)
	, m_window(window)
{
	m_watchedKeys = {inKey};
	resizeForNfft(m_nfft, FFTMode::Real);
}

GenalyzerFFTProcessor::~GenalyzerFFTProcessor()
{
	std::lock_guard<std::mutex> lock(s_genalyzerMutex);
	cleanupAutoConfig();
	cleanupFaConfig();
}

// ---------------------------------------------------------------------------
// Configuration helpers
// ---------------------------------------------------------------------------

GenalyzerFFTProcessor::FFTMode GenalyzerFFTProcessor::mode() const
{
	return (m_watchedKeys.size() == 1) ? FFTMode::Real : FFTMode::Complex;
}

void GenalyzerFFTProcessor::setSampleRate(double fs)
{
	std::lock_guard<std::mutex> lock(s_genalyzerMutex);
	if(m_sampleRate == fs)
		return;
	m_sampleRate = fs;
	rebuildFreqAxis(mode());
	// Sample rate is baked into both auto and fixed-tone configs; invalidate.
	cleanupAutoConfig();
	cleanupFaConfig();
}

void GenalyzerFFTProcessor::setConfig(const GenalyzerConfig &cfg)
{
	std::lock_guard<std::mutex> lock(s_genalyzerMutex);
	m_cfg = cfg;
	cleanupAutoConfig();
	cleanupFaConfig();
}

QWidget *GenalyzerFFTProcessor::createSettingsWidget(QWidget *parent)
{
	// Container = [Enable Analysis toggle] + inner GenalyzerSettings widget.
	auto *container = new QWidget(parent);
	auto *layout    = new QVBoxLayout(container);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(6);

	auto *enableBtn = new QPushButton("Enable Analysis", container);
	enableBtn->setCheckable(true);
	enableBtn->setChecked(m_cfg.enabled);
	layout->addWidget(enableBtn);

	auto *settings = new GenalyzerSettings(container);
	settings->setConfig(m_cfg);
	layout->addWidget(settings);

	// Inner settings widget edits mode/SSB/etc. but never flips `enabled`.
	connect(settings, &GenalyzerSettings::configChanged, this,
		[this, enableBtn](const GenalyzerConfig &cfg) {
			GenalyzerConfig merged = cfg;
			merged.enabled         = enableBtn->isChecked();
			setConfig(merged);
		});

	// Enable button drives `enabled` independently.
	connect(enableBtn, &QPushButton::toggled, this,
		[this, settings](bool on) {
			GenalyzerConfig merged = settings->getConfig();
			merged.enabled         = on;
			setConfig(merged);
		});

	return container;
}

// ---------------------------------------------------------------------------
// Buffer / axis management
// ---------------------------------------------------------------------------

void GenalyzerFFTProcessor::resizeForNfft(int nfft, FFTMode mode)
{
	m_nfft = nfft;
	if(mode == FFTMode::Complex) {
		m_outBins    = nfft;
		m_fftOutSize = 2 * nfft;
		m_iBuf.assign(nfft, 0.0);
		m_qBuf.assign(nfft, 0.0);
		m_realIn.clear();
		m_shifted.assign(m_fftOutSize, 0.0);
	} else {
		m_outBins    = nfft / 2 + 1;
		m_fftOutSize = 2 * m_outBins;
		m_realIn.assign(nfft, 0.0);
		m_iBuf.clear();
		m_qBuf.clear();
		m_shifted.clear();
	}
	m_fftOut.assign(m_fftOutSize, 0.0);
	m_dbBuf.assign(m_outBins, 0.0);
	rebuildFreqAxis(mode);
}

void GenalyzerFFTProcessor::rebuildFreqAxis(FFTMode mode)
{
	const double rbw = m_sampleRate / m_nfft;
	if(mode == FFTMode::Complex) {
		m_freqAxis.resize(m_nfft);
		for(int i = 0; i < m_nfft; ++i)
			m_freqAxis[i] = static_cast<float>(i * rbw - m_sampleRate / 2.0);
	} else {
		const int N = m_nfft / 2 + 1;
		m_freqAxis.resize(N);
		for(int i = 0; i < N; ++i)
			m_freqAxis[i] = static_cast<float>(i * rbw);
	}
	m_lastModeForAxis = mode;
}

void GenalyzerFFTProcessor::reset()
{
	std::lock_guard<std::mutex> lock(s_genalyzerMutex);
	std::fill(m_iBuf.begin(),    m_iBuf.end(),    0.0);
	std::fill(m_qBuf.begin(),    m_qBuf.end(),    0.0);
	std::fill(m_realIn.begin(),  m_realIn.end(),  0.0);
	std::fill(m_fftOut.begin(),  m_fftOut.end(),  0.0);
	std::fill(m_shifted.begin(), m_shifted.end(), 0.0);
	std::fill(m_dbBuf.begin(),   m_dbBuf.end(),   0.0);
	m_lastResults = {};
}

// ---------------------------------------------------------------------------
// FFT cores (must be called with s_genalyzerMutex held)
// ---------------------------------------------------------------------------

int GenalyzerFFTProcessor::runComplexFFT(const QVector<float> &iSamples,
					 const QVector<float> &qSamples)
{
	for(int i = 0; i < m_nfft; ++i) {
		m_iBuf[i] = static_cast<double>(iSamples[i]);
		m_qBuf[i] = static_cast<double>(qSamples[i]);
	}

	int err = gn_fft(m_fftOut.data(), m_fftOutSize,
			 m_iBuf.data(), m_nfft,
			 m_qBuf.data(), m_nfft,
			 1, m_nfft, m_window);
	if(err != 0) {
		report(AcquisitionError::Severity::Warning,
		       QStringLiteral("gn_fft failed err=%1").arg(err));
		return err;
	}

	err = gn_ifftshift(m_shifted.data(), m_fftOutSize,
			   m_fftOut.data(),  m_fftOutSize);
	if(err != 0) {
		report(AcquisitionError::Severity::Warning,
		       QStringLiteral("gn_ifftshift failed err=%1").arg(err));
		return err;
	}

	err = gn_db(m_dbBuf.data(), m_outBins,
		    m_shifted.data(), m_fftOutSize);
	if(err != 0) {
		report(AcquisitionError::Severity::Warning,
		       QStringLiteral("gn_db (complex) failed err=%1").arg(err));
		return err;
	}
	return 0;
}

int GenalyzerFFTProcessor::runRealFFT(const QVector<float> &samples)
{
	for(int i = 0; i < m_nfft; ++i)
		m_realIn[i] = static_cast<double>(samples[i]);

	int err = gn_rfft(m_fftOut.data(), m_fftOutSize,
			  m_realIn.data(), m_nfft,
			  1, m_nfft, m_window, GnRfftScaleDbfsSin);
	if(err != 0) {
		report(AcquisitionError::Severity::Warning,
		       QStringLiteral("gn_rfft failed err=%1").arg(err));
		return err;
	}

	err = gn_db(m_dbBuf.data(), m_outBins,
		    m_fftOut.data(), m_fftOutSize);
	if(err != 0) {
		report(AcquisitionError::Severity::Warning,
		       QStringLiteral("gn_db (real) failed err=%1").arg(err));
		return err;
	}
	return 0;
}

// ---------------------------------------------------------------------------
// Analysis configuration
// ---------------------------------------------------------------------------

void GenalyzerFFTProcessor::cleanupAutoConfig()
{
	if(m_gnConfig) {
		gn_config_free(&m_gnConfig);
		m_gnConfig = nullptr;
	}
}

void GenalyzerFFTProcessor::cleanupFaConfig()
{
	if(m_faKey) {
		gn_fa_reset(m_faKey);
		std::free(m_faKey);
		m_faKey = nullptr;
	}
}

int GenalyzerFFTProcessor::configureAutoAnalysis()
{
	cleanupAutoConfig();

	int err = gn_config_fftz(static_cast<size_t>(m_nfft), /*qres=*/1, /*navg=*/1,
				 static_cast<size_t>(m_nfft), m_window, &m_gnConfig);
	if(err != 0) {
		report(AcquisitionError::Severity::Warning,
		       QStringLiteral("gn_config_fftz failed err=%1").arg(err));
		return err;
	}

	err = gn_config_set_sample_rate(m_sampleRate, &m_gnConfig);
	if(err != 0) {
		report(AcquisitionError::Severity::Warning,
		       QStringLiteral("gn_config_set_sample_rate failed err=%1").arg(err));
		return err;
	}

	err = gn_config_fa_auto(m_cfg.auto_params.ssb_width, &m_gnConfig);
	if(err != 0) {
		report(AcquisitionError::Severity::Warning,
		       QStringLiteral("gn_config_fa_auto failed err=%1").arg(err));
		return err;
	}
	return 0;
}

int GenalyzerFFTProcessor::configureFixedToneAnalysis()
{
	cleanupFaConfig();
	m_faKey = strdup("fa");

	int err = gn_fa_create(m_faKey);
	if(err != 0) {
		report(AcquisitionError::Severity::Warning,
		       QStringLiteral("gn_fa_create failed err=%1").arg(err));
		return err;
	}

	const auto &ft = m_cfg.fixed_tone;
	err = gn_fa_fixed_tone(m_faKey, ft.component_label.c_str(),
			       GnFACompTagSignal, ft.expected_freq, ft.ssb_fundamental);
	if(err != 0) {
		report(AcquisitionError::Severity::Warning,
		       QStringLiteral("gn_fa_fixed_tone failed err=%1").arg(err));
		return err;
	}

	if(ft.harmonic_order > 0) {
		err = gn_fa_hd(m_faKey, ft.harmonic_order);
		if(err != 0) {
			report(AcquisitionError::Severity::Warning,
			       QStringLiteral("gn_fa_hd failed err=%1").arg(err));
			return err;
		}
	}

	err = gn_fa_ssb(m_faKey, GnFASsbDefault, ft.ssb_default);
	if(err != 0) {
		report(AcquisitionError::Severity::Warning,
		       QStringLiteral("gn_fa_ssb(Default) failed err=%1").arg(err));
		return err;
	}
	err = gn_fa_ssb(m_faKey, GnFASsbDC, -1);
	if(err != 0) {
		report(AcquisitionError::Severity::Warning,
		       QStringLiteral("gn_fa_ssb(DC) failed err=%1").arg(err));
		return err;
	}
	err = gn_fa_ssb(m_faKey, GnFASsbSignal, -1);
	if(err != 0) {
		report(AcquisitionError::Severity::Warning,
		       QStringLiteral("gn_fa_ssb(Signal) failed err=%1").arg(err));
		return err;
	}
	err = gn_fa_ssb(m_faKey, GnFASsbWO, -1);
	if(err != 0) {
		report(AcquisitionError::Severity::Warning,
		       QStringLiteral("gn_fa_ssb(WO) failed err=%1").arg(err));
		return err;
	}

	err = gn_fa_fsample(m_faKey, m_sampleRate);
	if(err != 0) {
		report(AcquisitionError::Severity::Warning,
		       QStringLiteral("gn_fa_fsample failed err=%1").arg(err));
		return err;
	}
	err = gn_fa_fdata(m_faKey, m_sampleRate); // no decimation
	if(err != 0) {
		report(AcquisitionError::Severity::Warning,
		       QStringLiteral("gn_fa_fdata failed err=%1").arg(err));
		return err;
	}
	err = gn_fa_fshift(m_faKey, ft.fshift);
	if(err != 0) {
		report(AcquisitionError::Severity::Warning,
		       QStringLiteral("gn_fa_fshift failed err=%1").arg(err));
		return err;
	}
	err = gn_fa_conv_offset(m_faKey, ft.fshift != 0.0);
	if(err != 0) {
		report(AcquisitionError::Severity::Warning,
		       QStringLiteral("gn_fa_conv_offset failed err=%1").arg(err));
		return err;
	}
	return 0;
}

void GenalyzerFFTProcessor::publishResults(size_t results_size, char **rkeys, double *rvalues)
{
	m_lastResults.keys.clear();
	m_lastResults.values.clear();
	m_lastResults.keys.reserve(static_cast<int>(results_size));
	m_lastResults.values.reserve(static_cast<int>(results_size));
	for(size_t i = 0; i < results_size; ++i) {
		m_lastResults.keys.append(QString::fromUtf8(rkeys[i] ? rkeys[i] : ""));
		m_lastResults.values.append(rvalues[i]);
	}
	Q_EMIT analysisReady(m_lastResults);
}

static void freeKeyArray(char **rkeys, size_t count)
{
	if(!rkeys)
		return;
	for(size_t i = 0; i < count; ++i)
		std::free(rkeys[i]);
	std::free(rkeys);
}

void GenalyzerFFTProcessor::performAnalysis(FFTMode mode)
{
	if(m_cfg.isAutoMode()) {
		if(!m_gnConfig) {
			if(configureAutoAnalysis() != 0) {
				cleanupAutoConfig();
				Q_EMIT analysisFailed("auto analysis config failed");
				return;
			}
		}
		char  **rkeys        = nullptr;
		double *rvalues      = nullptr;
		size_t  results_size = 0;
		int     err          = gn_get_fa_results(&rkeys, &rvalues, &results_size,
                                                                m_fftOut.data(), &m_gnConfig);
		if(err != 0) {
			report(AcquisitionError::Severity::Warning,
			       QStringLiteral("gn_get_fa_results failed err=%1").arg(err));
			Q_EMIT analysisFailed(QStringLiteral("gn_get_fa_results=%1").arg(err));
			return;
		}
		publishResults(results_size, rkeys, rvalues);
		freeKeyArray(rkeys, results_size);
		std::free(rvalues);
		return;
	}

	// Fixed-tone analysis
	if(!m_faKey) {
		if(configureFixedToneAnalysis() != 0) {
			cleanupFaConfig();
			Q_EMIT analysisFailed("fixed-tone analysis config failed");
			return;
		}
	}

	size_t results_size = 0;
	int    err          = gn_fft_analysis_results_size(&results_size, m_faKey,
                                                           static_cast<size_t>(m_fftOutSize),
                                                           static_cast<size_t>(m_outBins));
	if(err != 0) {
		report(AcquisitionError::Severity::Warning,
		       QStringLiteral("gn_fft_analysis_results_size failed err=%1").arg(err));
		Q_EMIT analysisFailed(QStringLiteral("results_size=%1").arg(err));
		return;
	}

	size_t *rkey_sizes = static_cast<size_t *>(std::malloc(results_size * sizeof(size_t)));
	err = gn_fft_analysis_results_key_sizes(rkey_sizes, results_size, m_faKey,
						static_cast<size_t>(m_fftOutSize),
						static_cast<size_t>(m_outBins));
	if(err != 0) {
		report(AcquisitionError::Severity::Warning,
		       QStringLiteral("gn_fft_analysis_results_key_sizes failed err=%1").arg(err));
		std::free(rkey_sizes);
		Q_EMIT analysisFailed(QStringLiteral("key_sizes=%1").arg(err));
		return;
	}

	char  **rkeys   = static_cast<char **>(std::malloc(results_size * sizeof(char *)));
	double *rvalues = static_cast<double *>(std::malloc(results_size * sizeof(double)));
	for(size_t i = 0; i < results_size; ++i)
		rkeys[i] = static_cast<char *>(std::malloc(rkey_sizes[i]));
	std::free(rkey_sizes);

	const GnFreqAxisType axisType =
		(mode == FFTMode::Complex) ? GnFreqAxisTypeDcLeft : GnFreqAxisTypeReal;

	err = gn_fft_analysis(rkeys, results_size, rvalues, results_size, m_faKey,
			      m_fftOut.data(), static_cast<size_t>(m_fftOutSize),
			      static_cast<size_t>(m_outBins), axisType);
	if(err != 0) {
		report(AcquisitionError::Severity::Warning,
		       QStringLiteral("gn_fft_analysis failed err=%1").arg(err));
		freeKeyArray(rkeys, results_size);
		std::free(rvalues);
		Q_EMIT analysisFailed(QStringLiteral("fft_analysis=%1").arg(err));
		return;
	}

	publishResults(results_size, rkeys, rvalues);
	freeKeyArray(rkeys, results_size);
	std::free(rvalues);
}

// ---------------------------------------------------------------------------
// process()
// ---------------------------------------------------------------------------

void GenalyzerFFTProcessor::process(DataStore *store)
{
	const int nWatched = m_watchedKeys.size();
	if(nWatched != 1 && nWatched != 2) {
		report(AcquisitionError::Severity::Warning,
		       QStringLiteral("unexpected watchedKeys count=%1; skipping cycle").arg(nWatched));
		return;
	}

	const FFTMode currentMode = (nWatched == 1) ? FFTMode::Real : FFTMode::Complex;

	// Resolve input variants outside the gn_* mutex (DataStore handles its own locking).
	QVector<float> iSamples, qSamples;
	{
		const SampleBuffer aBuf = store->read(m_watchedKeys[0]);
		if(aBuf.empty())
			return;
		const auto &aVar = aBuf.sample(0);
		if(!std::holds_alternative<QVector<float>>(aVar)) {
			report(AcquisitionError::Severity::Warning,
			       QStringLiteral("input 0 is not QVector<float>; skipping"));
			return;
		}
		iSamples = std::get<QVector<float>>(aVar);

		if(currentMode == FFTMode::Complex) {
			const SampleBuffer bBuf = store->read(m_watchedKeys[1]);
			if(bBuf.empty())
				return;
			const auto &bVar = bBuf.sample(0);
			if(!std::holds_alternative<QVector<float>>(bVar)) {
				report(AcquisitionError::Severity::Warning,
				       QStringLiteral("input 1 is not QVector<float>; skipping"));
				return;
			}
			qSamples = std::get<QVector<float>>(bVar);
		}
	}

	const int n = (currentMode == FFTMode::Complex)
			      ? qMin(iSamples.size(), qSamples.size())
			      : iSamples.size();
	if(n <= 0)
		return;

	std::lock_guard<std::mutex> lock(s_genalyzerMutex);

	if(n != m_nfft || currentMode != m_lastModeForAxis) {
		report(AcquisitionError::Severity::Info,
		       QStringLiteral("resize: nfft %1 -> %2 mode=%3")
			       .arg(m_nfft)
			       .arg(n)
			       .arg(currentMode == FFTMode::Real ? "Real" : "Complex"));
		resizeForNfft(n, currentMode);
		// nfft or mode changed -> previous gn configs are stale.
		cleanupAutoConfig();
		cleanupFaConfig();
	}

	int err = 0;
	if(currentMode == FFTMode::Complex)
		err = runComplexFFT(iSamples, qSamples);
	else
		err = runRealFFT(iSamples);

	if(err != 0) {
		Q_EMIT analysisFailed(QStringLiteral("fft=%1").arg(err));
		return;
	}

	// Publish dB output + frequency axis.
	QVector<float> out(m_outBins);
	for(int i = 0; i < m_outBins; ++i)
		out[i] = static_cast<float>(m_dbBuf[i]);

	store->write(m_outputKey, std::move(out));
	store->write(m_freqKey, m_freqAxis);

	// Analysis is optional.
	if(m_cfg.enabled)
		performAnalysis(currentMode);
}

} // namespace acq
} // namespace scopy
