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

#include "dcsource.h"

#include "sourceregistry.h"

#include <core/acq_engine/datakey.h>
#include <core/acq_engine/datastore.h>

#include <component/attribute.h>
#include <component/attributereader.h>
#include <component/backends/iio/iiosamplecodec.h>
#include <component/backends/iio/iioscanelement.h>
#include <component/context.h>
#include <component/device.h>
#include <component/inputstream.h>
#include <component/navigation.h>
#include <component/stream.h>
#include <component/streamformat.h>

#include <iio-widgets/iiowidget.h>
#include <iio-widgets/iiowidgetbuilder.h>

#include <gui/widgets/menusectionwidget.h>

#include <qcoro/qcorotask.h>

#include <QCheckBox>
#include <QDebug>
#include <QMutexLocker>
#include <QVBoxLayout>
#include <QWidget>

#include <algorithm>

using namespace scopy;
using namespace scopy::adc;

namespace {

// The device attribute every IIO buffer driver publishes its rate as. Not an API on the
// component tree — just an ordinary Attribute, looked up by name.
constexpr const char *kRateAttr = "sampling_frequency";

// convert() sign-extends to its own width only, so a signed 32-bit element decoded into an
// int64_t leaves the upper half clear. Re-extend from the format's width.
int64_t signExtend(int64_t raw, const scopy::iio::DataFormat &fmt)
{
	if(!fmt.is_signed || fmt.length == 0 || fmt.length >= 64) {
		return raw;
	}
	const int shift = 64 - static_cast<int>(fmt.length);
	return (raw << shift) >> shift;
}

} // namespace

DcSource::DcSource(component::Context *ctx, const QString &id, QObject *parent)
	: SourceBlock(id, parent)
	, m_ctx(ctx)
{
	if(m_ctx) {
		discover();
	}
}

DcSource::~DcSource() { closeAll(); }

bool DcSource::isAvailable() const { return m_ctx && !m_devices.isEmpty(); }

void DcSource::discover()
{
	for(component::Device *dev : m_ctx->findChildren<component::Device *>(QString(), Qt::FindDirectChildrenOnly)) {
		// Buffer index 0 only: the sibling streams a multi-buffer device gets are more
		// kernel buffers over the same channels, not more channels.
		component::InputStream *stream = component::streamAt<component::InputStream>(dev, 0);
		if(!stream) {
			continue;
		}

		Dev entry;
		entry.device = dev;
		entry.stream = stream;
		// The id ("iio:device0") when the driver publishes no name, which is legal — without
		// the fallback every channel on it would be keyed on a bare ":" and two unnamed
		// devices would collide.
		entry.name = dev->name().isEmpty() ? dev->id() : dev->name();

		// Parented to the stream, not the device, and their objectName is "Scan element N" —
		// so they are iterated and matched by id() rather than looked up.
		for(component::iio::IIOScanElement *el :
		    stream->findChildren<component::iio::IIOScanElement *>(QString(), Qt::FindDirectChildrenOnly)) {
			if(el->isOutput()) {
				continue;
			}
			Chan ch;
			ch.iioId = el->id();
			ch.channelId = entry.name + ":" + ch.iioId;
			ch.element = el;
			// An input and an output channel may share an id, so the direction is part
			// of the lookup. The codec hangs off the Channel, not the scan element.
			if(component::Channel *c = component::channelById(dev, ch.iioId, false)) {
				ch.codec = c->findChild<component::iio::IIOSampleCodec *>(QString(),
											  Qt::FindDirectChildrenOnly);
			}
			if(!ch.codec) {
				// Decoding needs the channel's declared format. Without it the
				// samples would be guesswork, so the channel is not offered.
				qWarning() << "DcSource: no sample codec for" << ch.channelId << "- skipping";
				continue;
			}
			entry.channels.append(ch);
		}

		if(entry.channels.isEmpty()) {
			continue;
		}

		// Ascending scan index, because that is the order buildStreamFormat() appends
		// enabled channels to StreamFormat::channels in. Everything downstream relies on a
		// position in `open` being a position in that list.
		std::sort(entry.channels.begin(), entry.channels.end(),
			  [](const Chan &a, const Chan &b) { return a.element->index() < b.element->index(); });

		for(const Chan &ch : entry.channels) {
			// All off: a context can hold dozens of channels across several devices, and
			// opening every one of them on the first run would be a surprise.
			enableChannel(ch.channelId, false);
		}
		m_devices.append(entry);
	}
}

void DcSource::onStart()
{
	SourceBlock::onStart();

	const QList<QString> enabled = enabledChannels();

	for(Dev &dev : m_devices) {
		QList<Chan> want;
		QList<int> mask;
		for(const Chan &ch : dev.channels) {
			if(enabled.contains(ch.channelId)) {
				want.append(ch);
				mask.append(static_cast<int>(ch.element->index()));
			}
		}
		if(want.isEmpty()) {
			continue;
		}

		// The mask is the whole truth, not an addition: openAsync() disables every scan
		// element not in it. Which is also why a channel toggled mid-run does nothing until
		// the next run.
		const auto resp = QCoro::waitFor(dev.stream->openAsync({mask, m_bufferSize}));
		if(!resp) {
			// Reported, not thrown: a throw here would make the engine fault-stop and tear
			// down every source, so one unavailable device would take the whole run with
			// it. The other devices keep streaming.
			qWarning() << "DcSource: open failed for" << dev.name << ":" << resp.error().errorString();
			continue;
		}
		dev.open = want;
	}

	readSampleRates();
}

void DcSource::readSampleRates()
{
	QHash<QString, double> rates;

	for(const Dev &dev : m_devices) {
		if(dev.open.isEmpty()) {
			continue;
		}
		component::Attribute *attr = component::attributeByName(dev.device, QString::fromLatin1(kRateAttr));
		if(!attr || !attr->readCapability()) {
			continue;
		}
		// Read deliberately: building the tree does not populate the attribute cache, so
		// without this the rate would read back empty and every stream would publish an
		// unknown timeline.
		if(!QCoro::waitFor(attr->readCapability()->readAsync())) {
			continue;
		}
		bool ok = false;
		const double rate = attr->cachedValue().trimmed().toDouble(&ok);
		if(!ok) {
			continue;
		}
		for(const Chan &ch : dev.open) {
			rates.insert(ch.channelId, rate);
		}
	}

	QMutexLocker lk(&m_rateMutex);
	m_sampleRates = rates;
}

void DcSource::acquire(scopy::acq::DataStore *store)
{
	if(!store) {
		return;
	}

	for(const Dev &dev : m_devices) {
		if(m_stopRequested) {
			return;
		}
		if(dev.open.isEmpty()) {
			continue;
		}

		const auto resp = QCoro::waitFor(dev.stream->refillAsync());

		// Re-checked after the wait: waitFor pumps this thread's event loop, so a close can
		// land mid-refill and the buffer the format describes may already be gone.
		if(m_stopRequested) {
			return;
		}
		if(!resp) {
			qWarning() << "DcSource: refill failed for" << dev.name << ":" << resp.error().errorString();
			continue;
		}

		decodeAndWrite(dev, store);
	}
}

void DcSource::decodeAndWrite(const Dev &dev, scopy::acq::DataStore *store)
{
	const component::StreamFormat &f = dev.stream->readFormat();

	// Positional, not by scan index: buildStreamFormat() appends one ChannelFormat per
	// *enabled* element in ascending-index order, which is exactly the order of dev.open.
	const int n = std::min(dev.open.size(), f.channels.size());

	for(int c = 0; c < n; ++c) {
		const Chan &ch = dev.open.at(c);
		const component::ChannelFormat &cf = f.channels.at(c);
		const char *p = static_cast<const char *>(f.data) + cf.offset;

		QVector<float> samples(static_cast<int>(f.sampleCount));
		for(size_t s = 0; s < f.sampleCount; ++s, p += cf.stride) {
			// Through the codec rather than a cast: it applies byte order, shift and
			// sign-extension from the channel's own format string. StreamView would be
			// the obvious shortcut but it memcpy's and ignores StreamFormat::order, so a
			// big-endian channel (the AD4130's be:U24/24>>0) would decode byte-swapped.
			// 64 bits wide and zeroed: convert writes the format's own width, which
			// is 8 bytes for a 64-bit scan element such as an IMU timestamp.
			int64_t raw = 0;
			ch.codec->convert(&raw, p);
			samples[static_cast<int>(s)] =
				static_cast<float>(signExtend(raw, ch.codec->dataFormat()) * cf.scale + cf.offsetPhys);
		}

		store->write(scopy::acq::DataKey::raw(id(), ch.channelId), std::move(samples));
	}
}

void DcSource::onStop()
{
	SourceBlock::onStop();
	closeAll();
}

void DcSource::closeAll()
{
	for(Dev &dev : m_devices) {
		if(dev.open.isEmpty()) {
			continue;
		}
		// Cleared before the wait, for the same reason IIOInputStream::closeAsync clears its
		// own state first: the wait pumps the event loop, and a refill resuming in there must
		// not find this device still marked open.
		dev.open.clear();
		QCoro::waitFor(dev.stream->closeAsync());
	}
}

std::optional<scopy::acq::StreamInfo> DcSource::streamInfo(const scopy::acq::DataKey &key) const
{
	// Base first: it matches the full key string rather than parsing it, so another block's
	// identically-named channel cannot pick up this one's description.
	std::optional<scopy::acq::StreamInfo> info = SourceBlock::streamInfo(key);
	if(!info) {
		return std::nullopt;
	}

	QMutexLocker lk(&m_rateMutex);
	info->sampleRate = m_sampleRates.value(key.channelId(), 0.0);
	return info;
}

QWidget *DcSource::createSettingsWidget(QWidget *parent)
{
	auto *own = new QWidget;
	auto *lay = new QVBoxLayout(own);
	lay->setContentsMargins(0, 0, 0, 0);
	lay->setSpacing(6);

	// One section per device, and no scroll area around them: the menu stack this page goes
	// into already scrolls, and a nested one would cap the page height and cut the last
	// device off. The rail row stays a plain uniform row — the per-device structure is here.
	for(const Dev &dev : m_devices) {
		auto *section = new MenuSectionCollapseWidget(dev.name, MenuCollapseSection::MHCW_ARROW,
							      MenuCollapseSection::MHW_BASEWIDGET, own);
		section->setCollapsed(true);

		for(const Chan &ch : dev.channels) {
			// Labelled with the IIO name, not the composite id: the device is already the
			// section title.
			auto *cb = new QCheckBox(ch.iioId, section);
			cb->setChecked(isChannelEnabled(ch.channelId));
			const QString channelId = ch.channelId;
			connect(cb, &QCheckBox::toggled, this,
				[this, channelId](bool en) { enableChannel(channelId, en); });
			section->contentLayout()->addWidget(cb);
		}

		// Whatever the device publishes, built from the Attribute metadata — which is what
		// keeps a new part from needing a hand-written settings page.
		for(IIOWidget *w : IIOWidgetBuilder(nullptr).componentContainer(dev.device).buildAll()) {
			section->contentLayout()->addWidget(w);
		}

		lay->addWidget(section);
	}

	lay->addStretch();
	return withBaseSettings(own, parent);
}

// Discovery is the constructor's, and isAvailable() answers from it, so this is the constructor
// alone — the same shape every other source registers with.
static const bool s_dcRegistered = AcqSourceRegistry::instance().add(
	[](scopy::component::Context *ctx, QObject *parent) -> scopy::acq::SourceBlock * {
		return new DcSource(ctx, QStringLiteral("dc"), parent);
	});

#include "moc_dcsource.cpp"
