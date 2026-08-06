#include "GenalyzerFFTProcessor.h"

#include "AcquisitionError.h"
#include "DataStore.h"
#include "GenalyzerSettings.h"
#include "GenalyzerTransformSettings.h"

#include <gui/widgets/menuonoffswitch.h>

#include <QAbstractButton>
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
	resizeForNfft(m_nfft, FFTMode::Complex, 1);
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
	resizeForNfft(m_nfft, FFTMode::Real, 1);
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
	Q_EMIT sampleRateChanged(fs);
}

GnWindow GenalyzerFFTProcessor::window() const
{
	std::lock_guard<std::mutex> lock(s_genalyzerMutex);
	return m_window;
}

void GenalyzerFFTProcessor::setWindow(GnWindow w)
{
	std::lock_guard<std::mutex> lock(s_genalyzerMutex);
	if(m_window == w)
		return;
	m_window = w;
	// The window is baked into gn_config_fftz, so the auto config is stale. The
	// fixed-tone config carries no window, but drop it too rather than leave the
	// two paths invalidated on different rules.
	cleanupAutoConfig();
	cleanupFaConfig();
}

GnRfftScale GenalyzerFFTProcessor::rfftScale() const
{
	std::lock_guard<std::mutex> lock(s_genalyzerMutex);
	return m_rfftScale;
}

void GenalyzerFFTProcessor::setRfftScale(GnRfftScale s)
{
	std::lock_guard<std::mutex> lock(s_genalyzerMutex);
	m_rfftScale = s;
}

void GenalyzerFFTProcessor::setAveragingStore(DataStore *store, std::size_t bufferSize)
{
	m_avgStore   = store;
	m_bufferSize = qMax<std::size_t>(1, bufferSize);
	reclaimAveragingDepth();
}

void GenalyzerFFTProcessor::setAveraging(int navg)
{
	const int n = qMax(1, navg);
	if(m_navg.exchange(n, std::memory_order_relaxed) == n)
		return;

	{
		std::lock_guard<std::mutex> lock(s_genalyzerMutex);
		// navg*nfft samples now come in per cycle, so the float->double staging
		// buffers grow; nfft itself is unchanged.
		resizeForNfft(m_nfft, mode(), n);
		// navg is an argument to gn_config_fftz.
		cleanupAutoConfig();
		cleanupFaConfig();
	}
	reclaimAveragingDepth();
	Q_EMIT averagingChanged(n);
}

void GenalyzerFFTProcessor::reclaimAveragingDepth()
{
	if(!m_avgStore)
		return;
	// One claim per watched key under this block's name. claimDepth replaces a
	// claimant's previous claim, so shrinking navg releases the depth too.
	std::size_t want;
	{
		std::lock_guard<std::mutex> lock(s_genalyzerMutex);
		want = static_cast<std::size_t>(m_navg.load(std::memory_order_relaxed)) *
		       static_cast<std::size_t>(qMax(1, m_nfft));
	}
	for(const DataKey &k : m_watchedKeys)
		m_avgStore->claimDepth(k, name(), DataStore::depthForWindow(want, m_bufferSize));
}

QHash<DataKey, StreamInfo> GenalyzerFFTProcessor::declaredStreams() const
{
	QHash<DataKey, StreamInfo> result;

	StreamInfo mag;
	mag.label = QStringLiteral("FFT");
	mag.unit = QStringLiteral("dBFS");
	// Read under the same mutex setSampleRate() writes it under; a view asking for
	// descriptors from the GUI thread races the settings widget otherwise.
	{
		std::lock_guard<std::mutex> lock(s_genalyzerMutex);
		mag.sampleRate = m_sampleRate;
	}
	// The bin frequencies this magnitude stream is indexed by. This is the
	// producer stating a fact about its own output, not a UI control — which is
	// the distinction that matters here: a user picking two keys out of combo
	// boxes was the mechanism removed in d0feff018.
	mag.xKey = m_freqKey;
	mag.kind = ReprKind::Curve;
	result.insert(m_outputKey, mag);

	// Declared, so a view can see it exists and label it, but never drawn.
	StreamInfo freq;
	freq.label = QStringLiteral("FFT frequency");
	freq.unit = QStringLiteral("Hz");
	freq.kind = ReprKind::Hidden;
	result.insert(m_freqKey, freq);

	return result;
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
	// [transform settings] + [Enable Analysis switch] + inner GenalyzerSettings
	// widget, under the base block controls.
	auto *container = new QWidget;
	auto *layout    = new QVBoxLayout(container);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(6);

	layout->addWidget(new GenalyzerTransformSettings(this, container));

	// A switch, not a button: this is a boolean in a settings menu, so it reads
	// the same as every other toggle on the page.
	auto *enableSwitch = new MenuOnOffSwitch(QStringLiteral("Enable Analysis"), container, false);
	enableSwitch->onOffswitch()->setChecked(m_cfg.enabled);
	layout->addWidget(enableSwitch);

	auto *settings = new GenalyzerSettings(container);
	settings->setConfig(m_cfg);
	layout->addWidget(settings);

	// Inner settings widget edits mode/SSB/etc. but never flips `enabled`.
	connect(settings, &GenalyzerSettings::configChanged, this,
		[this, enableSwitch](const GenalyzerConfig &cfg) {
			GenalyzerConfig merged = cfg;
			merged.enabled         = enableSwitch->onOffswitch()->isChecked();
			setConfig(merged);
		});

	// Enable switch drives `enabled` independently.
	connect(enableSwitch->onOffswitch(), &QAbstractButton::toggled, this,
		[this, settings](bool on) {
			GenalyzerConfig merged = settings->getConfig();
			merged.enabled         = on;
			setConfig(merged);
		});

	return withBaseSettings(container, parent);
}

// ---------------------------------------------------------------------------
// Buffer / axis management
// ---------------------------------------------------------------------------

void GenalyzerFFTProcessor::resizeForNfft(int nfft, FFTMode mode, int navg)
{
	m_nfft         = nfft;
	m_sizedForNavg = qMax(1, navg);
	// Input staging holds every frame genalyzer will average; the transform output
	// is one averaged spectrum, so output sizing is nfft-only.
	const int inLen = nfft * m_sizedForNavg;
	if(mode == FFTMode::Complex) {
		m_outBins    = nfft;
		m_fftOutSize = 2 * nfft;
		m_iBuf.assign(inLen, 0.0);
		m_qBuf.assign(inLen, 0.0);
		m_realIn.clear();
		m_shifted.assign(m_fftOutSize, 0.0);
	} else {
		m_outBins    = nfft / 2 + 1;
		m_fftOutSize = 2 * m_outBins;
		m_realIn.assign(inLen, 0.0);
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
					 const QVector<float> &qSamples,
					 int                   navg)
{
	const int inLen = m_nfft * navg;
	for(int i = 0; i < inLen; ++i) {
		m_iBuf[i] = static_cast<double>(iSamples[i]);
		m_qBuf[i] = static_cast<double>(qSamples[i]);
	}

	int err = gn_fft(m_fftOut.data(), m_fftOutSize,
			 m_iBuf.data(), inLen,
			 m_qBuf.data(), inLen,
			 navg, m_nfft, m_window);
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

int GenalyzerFFTProcessor::runRealFFT(const QVector<float> &samples, int navg)
{
	const int inLen = m_nfft * navg;
	for(int i = 0; i < inLen; ++i)
		m_realIn[i] = static_cast<double>(samples[i]);

	int err = gn_rfft(m_fftOut.data(), m_fftOutSize,
			  m_realIn.data(), inLen,
			  navg, m_nfft, m_window, m_rfftScale);
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

	// navg must match what runComplexFFT/runRealFFT actually passed, or the
	// analysis reads the spectrum with the wrong noise normalisation.
	const size_t navg = static_cast<size_t>(qMax(1, m_navg.load(std::memory_order_relaxed)));
	int err = gn_config_fftz(static_cast<size_t>(m_nfft) * navg, /*qres=*/1, navg,
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
	const int     navg        = qMax(1, m_navg.load(std::memory_order_relaxed));

	// Resolve inputs outside the gn_* mutex (DataStore handles its own locking).
	// With navg > 1 the frames genalyzer averages come out of the store's chunk
	// history, so ask for the whole window rather than the newest chunk. window()
	// returns a short vector while history is still filling, which the n/navg
	// derivation below absorbs — the first cycles just use a smaller nfft.
	int want;
	{
		// m_nfft is written by resizeForNfft() from both this thread and
		// setAveraging(); take the mutex for the read rather than tearing.
		std::lock_guard<std::mutex> lock(s_genalyzerMutex);
		want = m_nfft * navg;
	}
	QVector<float> iSamples, qSamples;
	for(int i = 0; i < nWatched; ++i) {
		QVector<float> s;
		if(navg > 1) {
			s = store->windowAs<QVector<float>>(m_watchedKeys[i], want);
			if(s.isEmpty()) {
				if(store->contains(m_watchedKeys[i]))
					report(AcquisitionError::Severity::Warning,
					       QStringLiteral("input %1 is not float; skipping").arg(i));
				return;
			}
		} else {
			auto v = store->latestAs<QVector<float>>(m_watchedKeys[i]);
			if(!v) {
				if(store->contains(m_watchedKeys[i]))
					report(AcquisitionError::Severity::Warning,
					       QStringLiteral("input %1 is not float; skipping").arg(i));
				return;
			}
			s = std::move(*v);
		}
		(i == 0 ? iSamples : qSamples) = std::move(s);
	}

	const int n = (currentMode == FFTMode::Complex)
			      ? qMin(iSamples.size(), qSamples.size())
			      : iSamples.size();
	// nfft is derived, not configured: the input carries navg frames of it. A
	// partial window (history still filling) truncates rather than skips, so the
	// spectrum appears from the first cycle at reduced resolution.
	const int nfft = n / navg;
	if(nfft <= 0)
		return;

	bool resized = false;
	{
		std::lock_guard<std::mutex> lock(s_genalyzerMutex);

		if(nfft != m_nfft || currentMode != m_lastModeForAxis || navg != m_sizedForNavg) {
			report(AcquisitionError::Severity::Info,
			       QStringLiteral("resize: nfft %1 -> %2 navg=%3 mode=%4")
				       .arg(m_nfft)
				       .arg(nfft)
				       .arg(navg)
				       .arg(currentMode == FFTMode::Real ? "Real" : "Complex"));
			resizeForNfft(nfft, currentMode, navg);
			// nfft, mode or navg changed -> previous gn configs are stale.
			cleanupAutoConfig();
			cleanupFaConfig();
			resized = true;
		}

		int err = 0;
		if(currentMode == FFTMode::Complex)
			err = runComplexFFT(iSamples, qSamples, navg);
		else
			err = runRealFFT(iSamples, navg);

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

	// Outside the gn_* mutex: a settings widget slot reading nfft() or window()
	// takes that same non-recursive mutex, and a direct connection would deadlock.
	if(resized) {
		// The claim is sized navg*nfft, so a derived nfft has to re-register it —
		// otherwise averaging keeps the old, possibly too-small, depth.
		reclaimAveragingDepth();
		Q_EMIT nfftChanged(nfft);
	}
}

} // namespace acq
} // namespace scopy
