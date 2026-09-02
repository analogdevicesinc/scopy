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

#include "acqchannel.h"

#include "acqaxis.h"
#include "acqplot.h"

#include <core/acq_engine/AcquisitionEngine.h>
#include <core/acq_engine/DataStore.h>

#include <gui/instrumenttemplate.h>
#include <gui/plotaxis.h>
#include <gui/plotwidget.h>
#include <gui/style.h>
#include <gui/widgets/menulineedit.h>
#include <gui/widgets/menusectionwidget.h>

#include <qwt_axis.h>

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

using namespace scopy;
using namespace scopy::adc;

namespace {

// Display name for a stream whose producer declared none. The key string is a poor
// label but an honest one — it is what the DataStore viewer shows, so the two agree.
QString labelFor(const scopy::acq::StreamInfo &info, const scopy::acq::DataKey &key)
{
	return info.label.isEmpty() ? key.toString() : info.label;
}

// The stream X is read from: the caller's pick, else the producer's recommendation, else
// the engine's ramp. Never empty — see the member's initialiser for why that matters.
scopy::acq::DataKey resolveXKey(const AcqChannel::Args &args)
{
	if(!args.xKey.key.isEmpty()) {
		return args.xKey;
	}
	if(!args.info.xKey.key.isEmpty()) {
		return args.info.xKey;
	}
	return scopy::acq::AcquisitionEngine::indexRampKey();
}

} // namespace

AcqChannel::AcqChannel(const Args &args)
	: QObject(args.parent)
	, m_store(args.store)
	, m_engine(args.engine)
	, m_key(args.key)
	// The caller's explicit choice first, the producer's recommendation second, the
	// engine's index ramp last. StreamInfo is a recommendation everywhere else in this
	// code, and it is one here too.
	//
	// Resolved here and only here, which is what lets an empty DataKey keep exactly the
	// one meaning core gives it in StreamInfo::xKey — *the producer named no X*. From
	// this point on m_xKey is a real stream in every case, so nothing downstream tests
	// it for emptiness to discover what it is; the X axis's Mode says that.
	, m_xKey(resolveXKey(args))
	, m_info(args.info)
	, m_name(labelFor(args.info, args.key))
	// getChannelColor() wraps its index into the palette, so an unresolved -1 would
	// silently land on the last entry. The manager assigns a real slot before
	// constructing us; clamping here only covers a caller that did not.
	, m_color(Style::getChannelColor(qMax(0, args.info.colorIndex)))
	, m_uid(args.uid)
	, m_claimant(QStringLiteral("acqch-%1").arg(args.uid))
	// The producer's rate if it declared one. 1.0 rather than 0.0 as the fallback
	// because every consumer of it divides: a "time (s)" axis then reads in samples,
	// which is wrong-but-readable, where a zero divisor is neither.
	, m_sampleRate(args.info.sampleRate > 0.0 ? args.info.sampleRate : 1.0)
{
	// No depth claim here: plotSize and bufferSize belong to the manager, which calls
	// reclaimDepth() right after attaching us.
}

scopy::acq::DataStore *AcqChannel::store() const { return m_store.data(); }

AcqChannel::~AcqChannel()
{
	// Virtual dispatch is already gone by the time ~AcqChannel runs, so this cannot
	// reach the subclass's detachFrom(). Each subclass destructor calls detach()
	// itself; this is the backstop for the base-owned state only.
	m_owner = nullptr;
	if(!m_store.isNull()) {
		// Sweeps every key this claimant holds in one call, so it stays correct even
		// if key retargeting is ever added.
		m_store->releaseClaimant(m_claimant);
	}
}

QString AcqChannel::menuId() const { return QStringLiteral("acqch:%1").arg(m_uid); }

PlotChannel *AcqChannel::plotChannel() const { return nullptr; }

PlotAxis *AcqChannel::ownXAxis(AcqPlot *plot)
{
	Q_UNUSED(plot)
	return nullptr;
}

PlotAxis *AcqChannel::ownYAxis(AcqPlot *plot)
{
	Q_UNUSED(plot)
	return nullptr;
}

QWidget *AcqChannel::createKindSettings(QWidget *parent)
{
	Q_UNUSED(parent)
	return nullptr;
}

void AcqChannel::onEnabledChanged(bool en) { Q_UNUSED(en) }
void AcqChannel::onColorChanged(const QColor &c) { Q_UNUSED(c) }
void AcqChannel::onNameChanged(const QString &n) { Q_UNUSED(n) }
void AcqChannel::onSampleRateChanged(double sr) { Q_UNUSED(sr) }

void AcqChannel::acquireAxes()
{
	if(!m_owner || m_xAxis || m_yAxis) {
		return;
	}

	const bool pooled = m_owner->supportsPerChannelAxes();

	PlotAxis *xPlotAxis = nullptr;
	PlotAxis *yPlotAxis = nullptr;
	// Two reasons a side can decline the pool: the plot kind owns its axes (a
	// waterfall), or this kind draws on an axis it names itself (a logic track on the
	// plot's shared digital axis). Either way the AcqAxis still exists and is
	// source-fixed, so the section is visible with the constraint stated rather than
	// missing for one kind.
	const bool xPooled = pooled && wantsPooledXAxis();
	const bool yPooled = pooled && wantsPooledYAxis();

	if(xPooled) {
		xPlotAxis = m_owner->acquireAxis(QwtAxis::XBottom);
	} else {
		xPlotAxis = ownXAxis(m_owner);
	}
	if(yPooled) {
		yPlotAxis = m_owner->acquireAxis(QwtAxis::YLeft);
	} else {
		yPlotAxis = ownYAxis(m_owner);
	}

	// The plot widget's built-in pair is the fallback for a side that declined the pool
	// and named nothing — which is the waterfall case.
	PlotWidget *w = m_owner->plot();
	if(!xPlotAxis && w) {
		xPlotAxis = w->xAxis();
	}
	if(!yPlotAxis && w) {
		yPlotAxis = w->yAxis();
	}

	m_xAxis = new AcqAxis(xPlotAxis,
			      m_xKey == scopy::acq::AcquisitionEngine::indexRampKey()
				      ? AcqAxis::Source::sampleIndex()
				      : AcqAxis::Source::stream(m_xKey, QString()),
			      m_store.data(), m_engine.data(), this);
	m_yAxis = new AcqAxis(yPlotAxis, AcqAxis::Source::stream(m_key, m_info.unit), m_store.data(),
			      m_engine.data(), this);

	if(!xPooled) {
		m_xAxis->setSourceFixed(true, tr("This plot kind owns its horizontal axis"));
	}
	if(!yPooled) {
		m_yAxis->setSourceFixed(true, tr("This plot kind owns its vertical axis"));
	}

	// A retarget moves which keys this channel reads, so the depth claim has to move
	// with it. Releasing the dropped key is the manager's job — it holds the store and
	// the geometry — so this only reports; it does not call releaseDepth.
	connect(m_xAxis, &AcqAxis::sourceChanged, this,
		[this](scopy::acq::DataKey oldKey, scopy::acq::DataKey newKey) {
			m_xKey = newKey;
			Q_EMIT depthNeedsReclaim(oldKey);
		});
	// The Y axis's source is a scaling choice, not this channel's key — key() is fixed
	// for life. A retarget there still changes what is read, so it reclaims too.
	connect(m_yAxis, &AcqAxis::sourceChanged, this,
		[this](scopy::acq::DataKey oldKey, scopy::acq::DataKey newKey) {
			Q_UNUSED(newKey)
			Q_EMIT depthNeedsReclaim(oldKey);
		});
}

void AcqChannel::releaseAxes()
{
	// Capture the borrowed PlotAxis before destroying the wrapper: the AcqAxis
	// destructor deregisters its channel from its autoscaler, so it has to run before
	// the axis goes back in the pool and is handed to somebody else.
	PlotAxis *xBorrowed = m_xAxis ? m_xAxis->plotAxis() : nullptr;
	PlotAxis *yBorrowed = m_yAxis ? m_yAxis->plotAxis() : nullptr;
	const bool xWasPooled = m_owner && m_owner->supportsPerChannelAxes() && wantsPooledXAxis();
	const bool yWasPooled = m_owner && m_owner->supportsPerChannelAxes() && wantsPooledYAxis();

	delete m_xAxis;
	m_xAxis = nullptr;
	delete m_yAxis;
	m_yAxis = nullptr;

	// Only the pool's own axes go back. releaseAxis() warns about a foreign one, and a
	// plot widget's built-in pair or a shared digital axis is exactly that.
	if(m_owner) {
		if(xWasPooled) {
			m_owner->releaseAxis(xBorrowed);
		}
		if(yWasPooled) {
			m_owner->releaseAxis(yBorrowed);
		}
	}
}

void AcqChannel::attach(AcqPlot *plot)
{
	if(!plot) {
		return;
	}
	m_owner = plot;
	// Before attachTo: a kind builds its PlotChannel against xAxis()/yAxis().
	acquireAxes();
	attachTo(plot);

	// After attachTo, because plotChannel() only becomes non-null once the kind has
	// built its curve. Null for a kind that has none, which is what suppresses the
	// AUTOSCALE switch — see acqaxis.h.
	PlotChannel *pc = plotChannel();
	if(m_xAxis) {
		m_xAxis->setAutoscaleChannel(pc);
	}
	if(m_yAxis) {
		m_yAxis->setAutoscaleChannel(pc);
	}

	// After attachTo, so the visual it just created starts in the right state rather
	// than being created shown and hidden a moment later.
	onEnabledChanged(m_enabled);
}

void AcqChannel::detach()
{
	// Before detachFrom(), which is what destroys the kind's PlotChannel: the axes hold
	// that pointer as their autoscaler target, and ~AcqAxis unregisters it with
	// PlotAutoscaler::removeChannels() -> disconnect(channel, ...), which dereferences
	// it. Clearing the target first is the only ordering where neither side reads a
	// destroyed object — dropping the axes wholesale before detachFrom() would instead
	// leave the curve drawing against a PlotAxis already back in the pool.
	if(m_xAxis) {
		m_xAxis->setAutoscaleChannel(nullptr);
	}
	if(m_yAxis) {
		m_yAxis->setAutoscaleChannel(nullptr);
	}

	detachFrom();
	// After detachFrom, so nothing is still drawing against an axis that has gone back
	// to the pool.
	releaseAxes();
	m_owner = nullptr;
}

void AcqChannel::refreshAxisSourceChoices()
{
	if(m_xAxis) {
		m_xAxis->refreshSourceChoices();
	}
	if(m_yAxis) {
		m_yAxis->refreshSourceChoices();
	}
}

void AcqChannel::onStarted()
{
	// Nothing to do beyond the hook. The autoscalers are started by
	// AcqAxis::setAutoscale, not by run state, so a reader who toggles AUTOSCALE while
	// stopped still gets a scaled axis on the data already in the store.
}

void AcqChannel::onStopped()
{
	// One final pass, so the axes end up scaled to the last frame rather than to
	// whichever frame the autoscaler's timer happened to be on. Same call as
	// GRTimeChannelComponent::onStop(); a no-op while autoscale is off.
	if(m_xAxis) {
		m_xAxis->autoscaleOnce();
	}
	if(m_yAxis) {
		m_yAxis->autoscaleOnce();
	}
}

void AcqChannel::pull(int plotSize)
{
	if(!m_enabled || m_store.isNull()) {
		return;
	}
	readData(m_store.data(), plotSize);
}

void AcqChannel::reclaimDepth(int plotSize)
{
	if(m_store.isNull()) {
		return;
	}
	const DepthNeed d = depthNeeded(plotSize);
	const auto claim = [this, d](const scopy::acq::DataKey &k) {
		if(d.inSamples) {
			m_store->claimSamples(k, m_claimant, d.amount);
		} else {
			m_store->claimChunks(k, m_claimant, d.amount);
		}
	};

	claim(m_key);

	// The X key needs the same depth, or readData truncates to qMin(x.size, y.size) and
	// the curve draws SHORT rather than wrong — a silent failure. An X key left at the
	// store's default capacity of 1 clips a multi-chunk Y window to a single buffer.
	//
	// A plain "different from Y" test now: m_xKey is never empty, so the ramp gets a
	// claim like any other X stream. That claim resolves to one chunk, since the ramp
	// arrives as a single chunk at least plotSize long.
	if(m_xKey.key != m_key.key) {
		claim(m_xKey);
	}
}

void AcqChannel::setEnabled(bool en)
{
	if(m_enabled == en) {
		return;
	}
	// A channel whose key is gone has nothing to draw, so it stays off until the key
	// returns — but remember what was asked for, or re-enabling the key would also
	// silently re-enable a channel the reader had switched off.
	if(en && !m_keyPresent) {
		m_enabledBeforeVanish = true;
		return;
	}
	m_enabled = en;
	m_enabledBeforeVanish = en;
	onEnabledChanged(en);
	Q_EMIT enabledChanged(en);
}

void AcqChannel::setKeyPresent(bool present)
{
	if(m_keyPresent == present) {
		return;
	}
	m_keyPresent = present;

	if(!present) {
		// Remember and switch off. Not deleted: the key usually comes back on the next
		// run, and deleting would take this channel's settings page with it.
		m_enabledBeforeVanish = m_enabled;
		if(m_enabled) {
			m_enabled = false;
			onEnabledChanged(false);
			Q_EMIT enabledChanged(false);
		}
	} else if(m_enabledBeforeVanish && !m_enabled) {
		m_enabled = true;
		onEnabledChanged(true);
		Q_EMIT enabledChanged(true);
	}

	Q_EMIT keyPresentChanged(present);
}

void AcqChannel::setName(const QString &n)
{
	if(m_name == n) {
		return;
	}
	m_name = n;
	onNameChanged(n);
	Q_EMIT nameChanged(n);
}

void AcqChannel::setColor(const QColor &c)
{
	if(m_color == c) {
		return;
	}
	m_color = c;
	onColorChanged(c);
	Q_EMIT colorChanged(c);
}

void AcqChannel::setFallbackSampleRate(double sr)
{
	// The producer's own statement wins: it knows the timeline its samples are on,
	// and the view's fallback is a guess from the pipeline shape.
	if(m_info.sampleRate > 0.0 || sr <= 0.0 || qFuzzyCompare(m_sampleRate, sr)) {
		return;
	}
	m_sampleRate = sr;
	onSampleRateChanged(sr);
}

QWidget *AcqChannel::createSettingsPage(InstrumentTemplate *it, QWidget *parent)
{
	if(!it) {
		return nullptr;
	}

	QWidget *page = new QWidget(parent);
	QVBoxLayout *lay = new QVBoxLayout(page);
	lay->setContentsMargins(0, 0, 0, 0);

	// SO_VIEW throughout: none of this reaches the engine. A channel names a key and
	// draws it; whatever produced the key is a block with its own page.
	MenuSectionCollapseWidget *chSection = it->createMenuSection(QStringLiteral("CHANNEL"), SO_VIEW, page);

	MenuLineEdit *nameEdit = new MenuLineEdit(chSection);
	nameEdit->edit()->setText(m_name);
	connect(nameEdit->edit(), &QLineEdit::editingFinished, this,
		[this, nameEdit]() { setName(nameEdit->edit()->text()); });
	chSection->add(nameEdit);

	// The key is fixed for the channel's life, so this is a readout rather than a
	// picker. Retargeting means a new channel.
	QLabel *keyLabel = new QLabel(m_key.toString(), chSection);
	keyLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
	chSection->add(keyLabel);

	// What the producer said about the stream, where it said anything. A readout for
	// the same reason the key is: these are the engine's facts, and the control that
	// changes them belongs on the producing block's own page.
	QStringList facts;
	if(!m_info.unit.isEmpty()) {
		facts << QStringLiteral("unit: %1").arg(m_info.unit);
	}
	if(m_sampleRate > 1.0) {
		facts << QStringLiteral("rate: %1 Sa/s").arg(m_sampleRate, 0, 'g', 6);
	}
	// The X source as it stands now, not as the producer declared it — the axis may have
	// been retargeted since, and a readout that disagrees with the X AXIS section below
	// would be worse than none.
	// Named by mode rather than by key for the two index-based readings: "sample index"
	// and "time (s)" are both the engine's ramp, and printing its key here would say
	// nothing the reader can act on.
	if(m_xAxis && m_xAxis->isTime()) {
		facts << QStringLiteral("X: time (s)");
	} else if(m_xAxis && m_xAxis->isSampleIndex()) {
		facts << QStringLiteral("X: sample index");
	} else if(!m_xKey.key.isEmpty()) {
		facts << QStringLiteral("X: %1").arg(m_xKey.toString());
	}
	if(!facts.isEmpty()) {
		QLabel *infoLabel = new QLabel(facts.join(QStringLiteral("  ·  ")), chSection);
		infoLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
		chSection->add(infoLabel);
	}

	lay->addWidget(chSection);

	// The kind's own knobs, under the generic ones — the same generic-then-specific
	// stacking Block::withBaseSettings() uses.
	if(QWidget *own = createKindSettings(page)) {
		MenuSectionCollapseWidget *kindSection = it->createMenuSection(kindName(), SO_VIEW, page);
		kindSection->add(own);
		lay->addWidget(kindSection);
	}

	// Y before X, because a reader looks for the amplitude scale first. Both sections
	// are the same widget from the same class — see acqaxis.h; the title is the only
	// thing that differs. Guarded on the pointer rather than assumed non-null: the page
	// can in principle be built before attach(), and a missing section is better than a
	// crash.
	if(m_yAxis) {
		if(QWidget *ySection = m_yAxis->createSettings(it, tr("Y AXIS"), page)) {
			lay->addWidget(ySection);
		}
	}
	if(m_xAxis) {
		if(QWidget *xSection = m_xAxis->createSettings(it, tr("X AXIS"), page)) {
			lay->addWidget(xSection);
		}
	}

	QPushButton *delBtn = new QPushButton(tr("Delete channel"), page);
	Style::setStyle(delBtn, style::properties::button::borderButton);
	// Queued: the handler destroys this channel, and with it this page and this button,
	// so a direct connection would return into freed memory.
	connect(delBtn, &QAbstractButton::clicked, this, [this]() { Q_EMIT removeRequested(); },
		Qt::QueuedConnection);
	lay->addWidget(delBtn);

	lay->addStretch();
	return page;
}

#include "moc_acqchannel.cpp"
