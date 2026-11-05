/*
 * Copyright (c) 2026 Analog Devices Inc.
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

#include "iiosource.h"

#include <core/acq_engine/datakey.h>
#include <core/acq_engine/datastore.h>

#include <component/attribute.h>
#include <component/attributereader.h>
#include <component/backends/iio/iiosamplecodec.h>
#include <component/backends/iio/iioscanelement.h>
#include <component/channel.h>
#include <component/context.h>
#include <component/device.h>
#include <component/inputstream.h>
#include <component/navigation.h>
#include <component/stream.h>
#include <component/streamformat.h>

#include <gui/widgets/menucollapsesection.h>
#include <gui/widgets/menucombo.h>
#include <gui/widgets/menusectionwidget.h>
#include <gui/widgets/menuspinbox.h>
#include <gui/style.h>
#include <gui/style_attributes.h>

#include <qcoro/qcorotask.h>

#include <QCheckBox>
#include <QComboBox>
#include <QDateTime>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMutexLocker>
#include <QThread>
#include <QVBoxLayout>

#include <algorithm>
#include <numeric>

using namespace scopy;
using namespace scopy::multiacq;

namespace {

// The attributes a polled channel reads by default, best first.
const QStringList kPreferredAttrs{"input", "processed", "raw"};

// How long acquire() waits for a lane to produce something: a read interval and a bit, but
// never so long that the sources after this one in the cycle go stale.
constexpr int kAcquireSlackMs = 100;
constexpr int kAcquireMaxWaitMs = 1000;

// A lane that cannot reach its device retries this often.
constexpr int kRetryMs = 500;

double nowMs() { return static_cast<double>(QDateTime::currentMSecsSinceEpoch()); }

// One attribute read, parsed as a number. Blocking: runs on a lane thread.
bool readNumber(component::Attribute *attr, double &out)
{
	if(!attr || !attr->readCapability()) {
		return false;
	}
	const auto resp = QCoro::waitFor(attr->readCapability()->readAsync());
	if(!resp) {
		return false;
	}
	QByteArray bytes = resp.value();
	const int nul = bytes.indexOf('\0');
	if(nul >= 0) {
		bytes.truncate(nul);
	}
	// Some attributes hold several values ("1 2 3"); the first one is the reading.
	bool ok = false;
	out = QString::fromUtf8(bytes).trimmed().section(' ', 0, 0).toDouble(&ok);
	return ok;
}

// convert() sign-extends to its own width only. Re-extend from the format's width.
int64_t signExtend(int64_t raw, const scopy::iio::DataFormat &fmt)
{
	if(!fmt.is_signed || fmt.length == 0 || fmt.length >= 64) {
		return raw;
	}
	const int shift = 64 - static_cast<int>(fmt.length);
	return (raw << shift) >> shift;
}

} // namespace

IioSource::IioSource(const QString &id, const QString &uri, component::ContextHandle ctx, QObject *parent)
	: SourceBlock(id, parent)
	, m_uri(uri)
	, m_ctx(std::move(ctx))
{
	if(m_ctx) {
		discover();
	}
}

IioSource::~IioSource() { stopLanes(); }

acq::DataKey IioSource::timeKey(const QString &sourceId, const QString &channelId)
{
	return acq::DataKey::withStage(sourceId, channelId, QStringLiteral("time"));
}

bool IioSource::isAvailable() const { return m_ctx && !m_devices.isEmpty(); }

void IioSource::discover()
{
	for(component::Device *dev : m_ctx->findChildren<component::Device *>(QString(), Qt::FindDirectChildrenOnly)) {
		Dev d;
		d.device = dev;
		d.name = dev->name().isEmpty() ? dev->id() : dev->name();
		d.stream = component::streamAt<component::InputStream>(dev, 0);

		QHash<QString, component::iio::IIOScanElement *> elements;
		if(d.stream) {
			for(component::iio::IIOScanElement *el : d.stream->findChildren<component::iio::IIOScanElement *>(
				    QString(), Qt::FindDirectChildrenOnly)) {
				if(!el->isOutput()) {
					elements.insert(el->id(), el);
				}
			}
		}

		DeviceDesc desc;
		desc.name = d.name;
		desc.hasRate = component::attributeByName(dev, "sampling_frequency") != nullptr;
		for(component::Channel *c :
		    dev->findChildren<component::Channel *>(QString(), Qt::FindDirectChildrenOnly)) {
			if(c->isOutput()) {
				continue;
			}
			Chan ch;
			ch.channel = c;
			ch.desc.iioId = c->id();
			ch.desc.channelId = d.name + ":" + c->id();
			for(component::Attribute *a :
			    c->findChildren<component::Attribute *>(QString(), Qt::FindDirectChildrenOnly)) {
				if(a->readCapability()) {
					ch.desc.attributes.append(a->name());
				}
			}
			ch.element = elements.value(c->id());
			ch.codec = c->findChild<component::iio::IIOSampleCodec *>(QString(), Qt::FindDirectChildrenOnly);
			ch.desc.bufferable = ch.element && ch.codec;
			ch.desc.hasScale = ch.desc.attributes.contains(QStringLiteral("scale"));
			ch.desc.hasOffset = ch.desc.attributes.contains(QStringLiteral("offset"));
			if(ch.desc.attributes.isEmpty() && !ch.desc.bufferable) {
				continue;
			}
			d.channels.append(ch);
			desc.channels.append(ch.desc);
			desc.bufferable = desc.bufferable || ch.desc.bufferable;
			desc.hasRate = desc.hasRate || ch.desc.attributes.contains(QStringLiteral("sampling_frequency"));
		}
		if(d.channels.isEmpty()) {
			continue;
		}

		for(const Chan &ch : std::as_const(d.channels)) {
			// All off: the reader enables the ones to read on the settings page.
			enableChannel(ch.desc.channelId, false);
			ChannelConfig cfg;
			cfg.attribute = ch.desc.attributes.value(0);
			for(const QString &pref : kPreferredAttrs) {
				if(ch.desc.attributes.contains(pref)) {
					cfg.attribute = pref;
					break;
				}
			}
			// A channel with nothing to read but its buffer starts on the buffer.
			if(ch.desc.attributes.isEmpty()) {
				cfg.mode = Mode::Buffered;
			}
			m_configs.insert(ch.desc.channelId, cfg);
		}
		m_devices.append(d);
		m_descs.append(desc);
	}
}

bool IioSource::canScale(const ChannelDesc &ch, const ChannelConfig &cfg)
{
	if(!ch.hasScale && !ch.hasOffset) {
		return false;
	}
	return cfg.mode == Mode::Buffered || cfg.attribute == QLatin1String("raw");
}

QString IioSource::describe(const ChannelDesc &ch, const ChannelConfig &cfg)
{
	const QString f = formula(ch, cfg);
	QStringList attrs;
	if(cfg.mode == Mode::Polled) {
		attrs << cfg.attribute;
	}
	if(!f.isEmpty()) {
		if(ch.hasScale) {
			attrs << QStringLiteral("scale");
		}
		if(ch.hasOffset) {
			attrs << QStringLiteral("offset");
		}
	}
	const QString value = f.isEmpty() ? (cfg.mode == Mode::Buffered ? tr("buffer sample") : cfg.attribute) : f;
	if(attrs.isEmpty()) {
		return tr("Plotted: %1").arg(value);
	}
	return tr("Plotted: %1 · attributes: %2").arg(value, attrs.join(QStringLiteral(", ")));
}

QString IioSource::formula(const ChannelDesc &ch, const ChannelConfig &cfg)
{
	if(!cfg.scaled || !canScale(ch, cfg)) {
		return QString();
	}
	const QString raw = cfg.mode == Mode::Buffered ? tr("buffer sample") : QStringLiteral("raw");
	if(ch.hasScale && ch.hasOffset) {
		return QStringLiteral("(%1 + offset) × scale").arg(raw);
	}
	return ch.hasScale ? QStringLiteral("%1 × scale").arg(raw) : QStringLiteral("%1 + offset").arg(raw);
}

IioSource::ChannelConfig IioSource::config(const QString &channelId) const
{
	QMutexLocker lk(&m_cfgMutex);
	return m_configs.value(channelId);
}

void IioSource::setConfig(const QString &channelId, const ChannelConfig &cfg)
{
	{
		QMutexLocker lk(&m_cfgMutex);
		const ChannelConfig old = m_configs.value(channelId);
		if(old.mode == cfg.mode && old.attribute == cfg.attribute && old.scaled == cfg.scaled) {
			return;
		}
		m_configs.insert(channelId, cfg);
	}
	Q_EMIT configChanged();
}

double IioSource::rateOverride(const QString &device) const
{
	QMutexLocker lk(&m_cfgMutex);
	return m_rateOverrides.value(device, 0.0);
}

void IioSource::setRateOverride(const QString &device, double hz)
{
	{
		QMutexLocker lk(&m_cfgMutex);
		if(qFuzzyCompare(m_rateOverrides.value(device, 0.0) + 1.0, hz + 1.0)) {
			return;
		}
		m_rateOverrides.insert(device, hz);
	}
	Q_EMIT configChanged();
}

void IioSource::setReadIntervalMs(int ms)
{
	// Read by the lanes every tick, so no restart is needed.
	m_readIntervalMs = qMax(1, ms);
}

void IioSource::onStart()
{
	SourceBlock::onStart();
	stopLanes();
	m_lanesStop = false;
	{
		QMutexLocker lk(&m_pendingMutex);
		m_pending.clear();
	}
	{
		QMutexLocker lk(&m_failingMutex);
		m_failing.clear();
	}
	updateOnline();

	const QList<QString> enabled = enabledChannels();
	QHash<QString, ChannelConfig> configs;
	{
		QMutexLocker lk(&m_cfgMutex);
		configs = m_configs;
	}

	// Up to two lanes per device: one reading attributes, one streaming its buffer.
	for(const Dev &dev : std::as_const(m_devices)) {
		QList<Chan> polled, buffered;
		QList<ChannelConfig> polledCfgs, bufferedCfgs;
		for(const Chan &ch : dev.channels) {
			if(!enabled.contains(ch.desc.channelId)) {
				continue;
			}
			const ChannelConfig cfg = configs.value(ch.desc.channelId);
			if(cfg.mode == Mode::Buffered && ch.desc.bufferable && dev.stream) {
				buffered.append(ch);
				bufferedCfgs.append(cfg);
			} else if(cfg.mode == Mode::Polled && !cfg.attribute.isEmpty()) {
				polled.append(ch);
				polledCfgs.append(cfg);
			}
		}

		if(!polled.isEmpty()) {
			QThread *lane = QThread::create(
				[this, dev, polled, polledCfgs]() { runPolled(dev, polled, polledCfgs); });
			m_lanes.append(lane);
			lane->start();
		}
		if(!buffered.isEmpty()) {
			// Ascending scan index: the order the stream lays the enabled channels out in.
			QList<int> order(buffered.size());
			std::iota(order.begin(), order.end(), 0);
			std::sort(order.begin(), order.end(), [&buffered](int a, int b) {
				return buffered[a].element->index() < buffered[b].element->index();
			});
			QList<Chan> chans;
			QList<ChannelConfig> cfgs;
			for(int i : std::as_const(order)) {
				chans.append(buffered[i]);
				cfgs.append(bufferedCfgs[i]);
			}
			QThread *lane = QThread::create([this, dev, chans, cfgs]() { runBuffered(dev, chans, cfgs); });
			m_lanes.append(lane);
			lane->start();
		}
	}
}

IioSource::Conversion IioSource::conversion(const Chan &ch, const ChannelConfig &cfg)
{
	Conversion c;
	if(cfg.scaled && canScale(ch.desc, cfg)) {
		readNumber(component::attributeByName(ch.channel, "scale"), c.scale);
		readNumber(component::attributeByName(ch.channel, "offset"), c.offset);
	}
	return c;
}

void IioSource::runPolled(const Dev &dev, const QList<Chan> &chans, const QList<ChannelConfig> &cfgs)
{
	struct Reader
	{
		QString channelId;
		component::Attribute *attr{nullptr};
		Conversion conv;
	};

	QList<Reader> readers;
	for(int i = 0; i < chans.size(); ++i) {
		Reader r;
		r.channelId = chans[i].desc.channelId;
		r.attr = component::attributeByName(chans[i].channel, cfgs[i].attribute);
		r.conv = conversion(chans[i], cfgs[i]);
		readers.append(r);
	}

	while(!m_lanesStop) {
		const qint64 start = QDateTime::currentMSecsSinceEpoch();
		bool failed = false;
		for(const Reader &r : std::as_const(readers)) {
			double v = 0.0;
			if(!readNumber(r.attr, v)) {
				failed = true;
				continue;
			}
			if(m_lanesStop) {
				return;
			}
			push(r.channelId, {nowMs()}, {r.conv.apply(v)});
		}
		setDeviceOk(dev.name, !failed, dev.name + ": attribute read failed");
		const int interval = failed ? qMax(kRetryMs, m_readIntervalMs.load()) : m_readIntervalMs.load();
		if(!laneSleep(interval - static_cast<int>(QDateTime::currentMSecsSinceEpoch() - start))) {
			return;
		}
	}
}

void IioSource::runBuffered(const Dev &dev, const QList<Chan> &chans, const QList<ChannelConfig> &cfgs)
{
	// Reported apart from the attribute lane of the same device, which may be fine.
	const QString lane = dev.name + tr(" (buffer)");
	QList<Conversion> convs;
	for(int i = 0; i < chans.size(); ++i) {
		convs.append(conversion(chans[i], cfgs[i]));
	}

	QList<int> mask;
	for(const Chan &ch : chans) {
		mask.append(static_cast<int>(ch.element->index()));
	}
	const auto opened = QCoro::waitFor(dev.stream->openAsync({mask, m_bufferSize.load()}));
	if(!opened) {
		setDeviceOk(lane, false, dev.name + ": cannot open buffer: " + opened.error().errorString());
		return;
	}

	// The rate is the device's, or else the first channel's that publishes one.
	double rate = 0.0;
	QString from = tr("device sampling_frequency");
	readNumber(component::attributeByName(dev.device, "sampling_frequency"), rate);
	for(int i = 0; rate <= 0 && i < chans.size(); ++i) {
		from = tr("channel sampling_frequency");
		readNumber(component::attributeByName(chans[i].channel, "sampling_frequency"), rate);
	}
	if(rate <= 0) {
		from = tr("set on this page");
		rate = rateOverride(dev.name);
	}
	if(rate > 0) {
		Q_EMIT bufferRateChanged(dev.name, rate, from);
	} else {
		report(acq::AcquisitionError::Severity::Warning,
		       dev.name + tr(": no sample rate, buffer samples are spread over the time between refills"));
	}
	qint64 lastRateReport = 0;

	// One continuous timeline: sample i of a buffer is at next + i * dt, and the next
	// buffer carries on where this one stopped. Even spacing and no going back in time,
	// whatever the refills look like.
	double next = 0.0;
	double prevEnd = nowMs();
	while(!m_lanesStop) {
		const auto refilled = QCoro::waitFor(dev.stream->refillAsync());
		if(m_lanesStop) {
			break;
		}
		if(!refilled) {
			setDeviceOk(lane, false, dev.name + ": refill failed: " + refilled.error().errorString());
			if(!laneSleep(kRetryMs)) {
				break;
			}
			next = 0.0;
			prevEnd = nowMs();
			continue;
		}
		setDeviceOk(lane, true);

		const component::StreamFormat &f = dev.stream->readFormat();
		const int n = static_cast<int>(f.sampleCount);
		if(n <= 0) {
			continue;
		}
		// Spacing: the device's rate, or else the time this buffer took to arrive.
		const double end = nowMs();
		const double span = rate > 0 ? n * 1000.0 / rate : std::max(end - prevEnd, 1.0);
		const double dt = span / n;
		prevEnd = end;
		if(rate <= 0 && end - lastRateReport >= 1000) {
			lastRateReport = static_cast<qint64>(end);
			Q_EMIT bufferRateChanged(dev.name, 1000.0 / dt, tr("measured from refills"));
		}
		// Started a buffer's length ago. Re-anchored to the clock on the first buffer and
		// whenever the timeline has drifted off it by more than a buffer.
		const double start = end - span;
		if(next <= 0.0 || std::abs(start - next) > span) {
			next = start;
		}
		QVector<double> t(n);
		for(int s = 0; s < n; ++s) {
			t[s] = next + s * dt;
		}
		next += span;

		// Positional: the format holds one entry per enabled channel, in scan index order.
		const int count = std::min(static_cast<int>(chans.size()), static_cast<int>(f.channels.size()));
		for(int c = 0; c < count; ++c) {
			const Chan &ch = chans[c];
			const component::ChannelFormat &cf = f.channels[c];
			const char *p = static_cast<const char *>(f.data) + cf.offset;
			QVector<float> v(n);
			for(int s = 0; s < n; ++s, p += cf.stride) {
				int64_t raw = 0;
				ch.codec->convert(&raw, p);
				// The channel's own scale/offset, not the stream's: the same pair the
				// attribute mode uses and the settings page names.
				v[s] = convs[c].apply(static_cast<double>(signExtend(raw, ch.codec->dataFormat())));
			}
			push(ch.desc.channelId, t, v);
		}

		// Pace to the device rate: a source that answers refills at once (an emulator) would
		// race ahead of its own timeline. An absolute target, so time spent here does not drift.
		if(rate > 0 && !laneSleep(static_cast<int>(next + span - nowMs()))) {
			break;
		}
	}
	QCoro::waitFor(dev.stream->closeAsync());
}

void IioSource::push(const QString &channelId, const QVector<double> &t, const QVector<float> &v)
{
	QMutexLocker lk(&m_pendingMutex);
	Pending &p = m_pending[channelId];
	p.t += t;
	p.v += v;
	m_pendingCond.wakeAll();
}

void IioSource::setDeviceOk(const QString &device, bool ok, const QString &why)
{
	{
		QMutexLocker lk(&m_failingMutex);
		if(ok == !m_failing.contains(device)) {
			return;
		}
		if(ok) {
			m_failing.remove(device);
		} else {
			m_failing.insert(device);
		}
	}
	// On the transition only, so a device that stays unreachable is reported once.
	if(!ok) {
		report(acq::AcquisitionError::Severity::Warning, why);
	}
	updateOnline();
}

void IioSource::updateOnline()
{
	bool online;
	{
		QMutexLocker lk(&m_failingMutex);
		online = m_failing.isEmpty();
	}
	if(m_online.exchange(online) != online) {
		Q_EMIT onlineChanged(online);
	}
}

bool IioSource::laneSleep(int ms)
{
	QMutexLocker lk(&m_sleepMutex);
	if(!m_lanesStop && ms > 0) {
		m_sleepCond.wait(&m_sleepMutex, ms);
	}
	return !m_lanesStop;
}

void IioSource::acquire(acq::DataStore *store)
{
	QHash<QString, Pending> batch;
	{
		QMutexLocker lk(&m_pendingMutex);
		// Paced by the lanes: wait for the next reading rather than spin on an empty queue.
		if(m_pending.isEmpty() && !stopRequested()) {
			m_pendingCond.wait(&m_pendingMutex, std::min(m_readIntervalMs.load() + kAcquireSlackMs, kAcquireMaxWaitMs));
		}
		batch.swap(m_pending);
	}
	for(auto it = batch.begin(); it != batch.end(); ++it) {
		// Time first: a view that finds the values has their timestamps too.
		store->write(timeKey(id(), it.key()), std::move(it->t));
		store->write(acq::DataKey::raw(id(), it.key()), std::move(it->v));
	}
}

void IioSource::requestStop()
{
	m_lanesStop = true;
	{
		QMutexLocker lk(&m_sleepMutex);
		m_sleepCond.wakeAll();
	}
	{
		QMutexLocker lk(&m_pendingMutex);
		m_pendingCond.wakeAll();
	}
	SourceBlock::requestStop();
}

void IioSource::onStop()
{
	SourceBlock::onStop();
	stopLanes();
}

void IioSource::stopLanes()
{
	m_lanesStop = true;
	{
		QMutexLocker lk(&m_sleepMutex);
		m_sleepCond.wakeAll();
	}
	// A lane in the middle of a read finishes it first; that is as long as this waits.
	for(QThread *lane : std::as_const(m_lanes)) {
		lane->wait();
		delete lane;
	}
	m_lanes.clear();
}

QList<acq::DataKey> IioSource::outputKeys() const
{
	QList<acq::DataKey> keys;
	for(const QString &ch : channelIds()) {
		keys << acq::DataKey::raw(id(), ch) << timeKey(id(), ch);
	}
	return keys;
}

std::optional<acq::StreamInfo> IioSource::streamInfo(const acq::DataKey &key) const
{
	if(key.sourceId() != id() || !channelIds().contains(key.channelId())) {
		return std::nullopt;
	}
	acq::StreamInfo info;
	info.label = displayName() + " " + key.channelId();
	if(key.isRaw()) {
		info.kind = acq::ReprKind::Curve;
		info.xKey = timeKey(id(), key.channelId());
	} else {
		info.unit = QStringLiteral("ms");
	}
	return info;
}

QWidget *IioSource::createSettingsWidget(QWidget *parent)
{
	QWidget *own = new QWidget();
	QVBoxLayout *lay = new QVBoxLayout(own);
	lay->setContentsMargins(0, 0, 0, 0);
	lay->setSpacing(6);

	QLabel *uri = new QLabel(m_uri, own);
	uri->setTextInteractionFlags(Qt::TextSelectableByMouse);
	lay->addWidget(uri);

	gui::MenuSpinbox *interval = new gui::MenuSpinbox(tr("Read interval"), m_readIntervalMs / 1000.0, "s", 0.001,
							  3600, true, false, false, own);
	interval->setToolTip(tr("How often attribute-read channels are read"));
	connect(interval, &gui::MenuSpinbox::valueChanged, this,
		[this](double s) { setReadIntervalMs(static_cast<int>(s * 1000)); });
	lay->addWidget(interval);

	for(const DeviceDesc &dev : std::as_const(m_descs)) {
		auto *section = new MenuSectionCollapseWidget(dev.name, MenuCollapseSection::MHCW_ARROW,
							      MenuCollapseSection::MHW_BASEWIDGET, own);
		section->contentLayout()->setSpacing(4);
		if(dev.bufferable) {
			QLabel *rateLabel = new QLabel(tr("Buffer rate: shown when the buffer runs"), section);
			Style::setStyle(rateLabel, style::properties::label::subtle);
			const QString devName = dev.name;
			connect(
				this, &IioSource::bufferRateChanged, rateLabel,
				[rateLabel, devName](const QString &device, double hz, const QString &from) {
					if(device == devName) {
						rateLabel->setText(tr("Buffer rate: %1 Hz (%2)").arg(QString::number(hz, 'g', 6), from));
					}
				},
				Qt::QueuedConnection);
			section->contentLayout()->addWidget(rateLabel);
		}
		if(dev.bufferable && !dev.hasRate) {
			// Without it the buffer's own timing is all there is to go by.
			gui::MenuSpinbox *rate = new gui::MenuSpinbox(tr("Sample rate"), rateOverride(dev.name), "Hz", 0,
								      1e9, true, false, false, section);
			rate->setToolTip(tr("The device publishes no sampling_frequency. Buffer samples are spaced at "
					    "this rate; 0 spreads them over the time between refills."));
			const QString name = dev.name;
			connect(rate, &gui::MenuSpinbox::valueChanged, this,
				[this, name](double hz) { setRateOverride(name, hz); });
			section->contentLayout()->addWidget(rate);
		}
		for(const ChannelDesc &ch : dev.channels) {
			section->contentLayout()->addWidget(createChannelSettings(ch, section));
		}
		lay->addWidget(section);
	}
	lay->addStretch();
	return withBaseSettings(own, parent);
}

QWidget *IioSource::createChannelSettings(const ChannelDesc &ch, QWidget *parent)
{
	QWidget *w = new QWidget(parent);
	QVBoxLayout *lay = new QVBoxLayout(w);
	lay->setContentsMargins(0, 0, 0, 4);
	lay->setSpacing(2);

	const QString channelId = ch.channelId;
	QCheckBox *cb = new QCheckBox(ch.iioId, w);
	cb->setChecked(isChannelEnabled(channelId));
	lay->addWidget(cb);

	// Only for a channel that is read: the rest would just be noise.
	QWidget *details = new QWidget(w);
	QGridLayout *grid = new QGridLayout(details);
	grid->setContentsMargins(Style::getDimension(json::global::unit_2), 0, 0, 0);
	grid->setHorizontalSpacing(6);
	grid->setVerticalSpacing(2);
	lay->addWidget(details);

	QComboBox *modeCombo = new QComboBox(details);
	if(!ch.attributes.isEmpty()) {
		modeCombo->addItem(tr("Attribute read"), static_cast<int>(Mode::Polled));
	}
	if(ch.bufferable) {
		modeCombo->addItem(tr("Buffer stream"), static_cast<int>(Mode::Buffered));
	}
	modeCombo->setToolTip(tr("Attribute read: one read per interval. Buffer stream: samples from the "
				 "device buffer."));
	grid->addWidget(new QLabel(tr("Read"), details), 0, 0);
	grid->addWidget(modeCombo, 0, 1);

	QLabel *attrLabel = new QLabel(tr("Attribute"), details);
	QComboBox *attrCombo = new QComboBox(details);
	attrCombo->addItems(ch.attributes);
	grid->addWidget(attrLabel, 1, 0);
	grid->addWidget(attrCombo, 1, 1);

	QLabel *valueLabel = new QLabel(tr("Value"), details);
	QComboBox *valueCombo = new QComboBox(details);
	valueCombo->addItem(tr("Raw"), false);
	valueCombo->addItem(tr("Scaled"), true);
	grid->addWidget(valueLabel, 2, 0);
	grid->addWidget(valueCombo, 2, 1);

	QLabel *formulaLabel = new QLabel(details);
	Style::setStyle(formulaLabel, style::properties::label::subtle);
	formulaLabel->setWordWrap(true);
	grid->addWidget(formulaLabel, 3, 0, 1, 2);
	grid->setColumnStretch(1, 1);

	// Every control shows the stored config; only what applies to it is visible.
	auto show = [this, ch, details, modeCombo, attrLabel, attrCombo, valueLabel, valueCombo, formulaLabel]() {
		const ChannelConfig cfg = config(ch.channelId);
		QSignalBlocker b1(modeCombo), b2(attrCombo), b3(valueCombo);
		modeCombo->setCurrentIndex(modeCombo->findData(static_cast<int>(cfg.mode)));
		attrCombo->setCurrentText(cfg.attribute);
		valueCombo->setCurrentIndex(cfg.scaled ? 1 : 0);

		const bool polled = cfg.mode == Mode::Polled;
		const bool scalable = canScale(ch, cfg);
		modeCombo->setEnabled(modeCombo->count() > 1);
		attrLabel->setVisible(polled);
		attrCombo->setVisible(polled);
		valueLabel->setVisible(scalable);
		valueCombo->setVisible(scalable);
		formulaLabel->setText(describe(ch, cfg));
		details->setVisible(isChannelEnabled(ch.channelId));
	};
	show();

	connect(cb, &QCheckBox::toggled, this, [this, channelId, show](bool en) {
		enableChannel(channelId, en);
		show();
		Q_EMIT configChanged();
	});
	connect(this, &SourceBlock::channelEnabledChanged, cb, [cb, channelId, show](const QString &id, bool en) {
		if(id == channelId) {
			QSignalBlocker b(cb);
			cb->setChecked(en);
			show();
		}
	});
	auto edit = [this, channelId, show](auto change) {
		ChannelConfig cfg = config(channelId);
		change(cfg);
		setConfig(channelId, cfg);
		show();
	};
	connect(modeCombo, &QComboBox::currentIndexChanged, this, [edit, modeCombo](int) {
		edit([modeCombo](ChannelConfig &c) { c.mode = static_cast<Mode>(modeCombo->currentData().toInt()); });
	});
	connect(attrCombo, &QComboBox::currentTextChanged, this,
		[edit](const QString &a) { edit([a](ChannelConfig &c) { c.attribute = a; }); });
	connect(valueCombo, &QComboBox::currentIndexChanged, this, [edit, valueCombo](int) {
		edit([valueCombo](ChannelConfig &c) { c.scaled = valueCombo->currentData().toBool(); });
	});
	return w;
}

#include "moc_iiosource.cpp"
