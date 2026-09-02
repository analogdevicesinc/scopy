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

#include "acqaxis.h"

#include <core/acq_engine/AcquisitionEngine.h>
#include <core/acq_engine/DataStore.h>

#include <gui/instrumenttemplate.h>
#include <gui/plotautoscaler.h>
#include <gui/plotaxis.h>
#include <gui/plotchannel.h>
#include <gui/widgets/menucombo.h>
#include <gui/widgets/menulineedit.h>
#include <gui/widgets/menuonoffswitch.h>
#include <gui/widgets/menuplotaxisrangecontrol.h>
#include <gui/widgets/menusectionwidget.h>

#include <qwt_axis.h>

#include <QComboBox>
#include <QLineEdit>
#include <QSet>
#include <QSignalBlocker>
#include <QVBoxLayout>
#include <QVariant>

using namespace scopy;
using namespace scopy::adc;

// The combo's per-item payload is the DataKey as a string. Sample index no longer needs
// a sentinel — it *is* a key now, the engine's ramp — so the only sentinel left is time,
// which really is not a stream.
const QString &AcqAxis::timeComboData()
{
	static const QString s = QStringLiteral("\x01time-seconds");
	return s;
}

const QString &AcqAxis::rampKeyString()
{
	static const QString s = scopy::acq::AcquisitionEngine::indexRampKey().toString();
	return s;
}

AcqAxis::Source AcqAxis::Source::sampleIndex()
{
	return Source{scopy::acq::AcquisitionEngine::indexRampKey(), QStringLiteral("samples"),
		      Mode::SampleIndex};
}

AcqAxis::Source AcqAxis::Source::time()
{
	return Source{scopy::acq::AcquisitionEngine::indexRampKey(), QStringLiteral("s"), Mode::Time};
}

AcqAxis::AcqAxis(PlotAxis *axis, const Source &src, scopy::acq::DataStore *store,
		 scopy::acq::AcquisitionEngine *engine, QObject *parent)
	: QObject(parent)
	, m_axis(axis)
	, m_store(store)
	, m_engine(engine)
	, m_source(src)
{
	applyUnit();

	// Forward the axis's own range changes — a reader's pan or zoom goes through
	// PlotAxis, not through this object, so this is the only way a consumer hears
	// about one.
	if(!m_axis.isNull()) {
		connect(m_axis.data(), &PlotAxis::minChanged, this,
			[this](double) { Q_EMIT intervalChanged(min(), max()); });
		connect(m_axis.data(), &PlotAxis::maxChanged, this,
			[this](double) { Q_EMIT intervalChanged(min(), max()); });
	}
}

AcqAxis::~AcqAxis()
{
	// The PlotAxis is deliberately not touched: it belongs to the AcqPlot's pool and
	// outlives every channel that borrows it. See the header.
	if(!m_autoscaler.isNull() && m_autoscaleTarget) {
		// PlotAutoscaler::autoscale() walks its channel list on a timer, so a channel
		// left registered past this point is a use-after-free within one timeout.
		m_autoscaler->removeChannels(m_autoscaleTarget);
	}
}

int AcqAxis::position() const { return m_axis.isNull() ? QwtAxis::YLeft : m_axis->position(); }

bool AcqAxis::isHorizontal() const { return m_axis.isNull() ? false : m_axis->isHorizontal(); }

QString AcqAxis::unitForKey(const scopy::acq::DataKey &k) const
{
	if(k.key.isEmpty()) {
		// No producer named an X. The channel resolves that to the ramp before an axis
		// ever sees it, so this is only reachable for an axis constructed with a
		// default-built Source.
		return QStringLiteral("samples");
	}
	if(!m_engine.isNull()) {
		if(const std::optional<scopy::acq::StreamInfo> si = m_engine->streamInfo(k)) {
			if(!si->unit.isEmpty()) {
				return si->unit;
			}
		}
	}
	// The key rather than nothing: an unlabelled axis says less than a badly labelled
	// one, and this at least names the stream the numbers came from.
	return k.toString();
}

void AcqAxis::applyUnit()
{
	if(m_axis.isNull()) {
		return;
	}
	m_axis->setUnits(m_source.unit);
	// PlotAxis's constructor hides units (gui/src/plotaxis.cpp:107), so setting one
	// without this labels nothing.
	m_axis->setUnitsVisible(!m_source.unit.isEmpty());
}

void AcqAxis::setSource(const Source &src)
{
	if(m_sourceFixed) {
		return;
	}
	// Mode as well as key: sample index and time are the same stream read two different
	// ways, so a key-only test would swallow the switch between them.
	if(src.key == m_source.key && src.mode == m_source.mode) {
		return;
	}

	const scopy::acq::DataKey oldKey = m_source.key;
	m_source.key = src.key;
	m_source.mode = src.mode;

	// The unit follows the source unless the reader typed one: the new stream is in
	// different units than the old one almost by definition, and keeping the previous
	// label would mislabel every value on the axis.
	if(!m_unitOverridden) {
		m_source.unit = src.unit.isEmpty() ? unitForKey(src.key) : src.unit;
		applyUnit();
	}

	// The interval belongs to the old stream's range; forget it so the first read off
	// the new source applies rather than being dropped as a repeat.
	m_reqMin = qQNaN();
	m_reqMax = qQNaN();

	Q_EMIT sourceChanged(oldKey, m_source.key);
}

void AcqAxis::setSourceFixed(bool fixed, const QString &reason)
{
	m_sourceFixed = fixed;
	m_fixedReason = reason;
	if(!m_sourceCombo.isNull()) {
		m_sourceCombo->setEnabled(!fixed);
		m_sourceCombo->setToolTip(fixed ? reason : QString());
	}
}

void AcqAxis::setAutoscaleChannel(PlotChannel *ch)
{
	if(m_autoscaleTarget == ch) {
		return;
	}
	if(!m_autoscaler.isNull() && m_autoscaleTarget) {
		m_autoscaler->removeChannels(m_autoscaleTarget);
	}
	m_autoscaleTarget = ch;
	if(!m_autoscaler.isNull() && m_autoscaleTarget && m_autoscaleActive) {
		m_autoscaler->addChannels(m_autoscaleTarget);
	}
}

void AcqAxis::setAutoscaleChannelActive(bool active)
{
	if(m_autoscaleActive == active) {
		return;
	}
	m_autoscaleActive = active;
	if(m_autoscaler.isNull() || !m_autoscaleTarget) {
		return;
	}
	// removeChannels() before addChannels() in the enabling case too: PlotAutoscaler
	// keeps a plain list and would otherwise hold the same channel twice after a
	// disable/enable cycle, scanning it once per copy.
	m_autoscaler->removeChannels(m_autoscaleTarget);
	if(active) {
		m_autoscaler->addChannels(m_autoscaleTarget);
	}
}

bool AcqAxis::autoscale() const { return m_autoscale; }

void AcqAxis::setAutoscale(bool on)
{
	if(m_autoscale == on) {
		return;
	}
	m_autoscale = on;

	// The manual spinboxes and the autoscaler write the same axis, so leaving both
	// live means the reader's typed value is overwritten a timeout later.
	if(!m_rangeCtrl.isNull()) {
		m_rangeCtrl->setEnabled(!on);
	}
	if(!m_autoBtn.isNull() && m_autoBtn->onOffswitch() && m_autoBtn->onOffswitch()->isChecked() != on) {
		QSignalBlocker b(m_autoBtn->onOffswitch());
		m_autoBtn->onOffswitch()->setChecked(on);
	}

	if(m_autoscaler.isNull()) {
		return;
	}
	// start() is not optional: PlotAutoscaler::onNewData and autoscale() both return
	// immediately while its timer is stopped (gui/src/plotautoscaler.cpp:59-63), so an
	// autoscaler that is never started silently does nothing.
	if(on) {
		m_autoscaler->start();
	} else {
		m_autoscaler->stop();
	}
}

void AcqAxis::autoscaleOnce()
{
	if(!m_autoscale || m_autoscaler.isNull()) {
		return;
	}
	m_autoscaler->autoscale();
}

double AcqAxis::min() const { return m_axis.isNull() ? 0.0 : m_axis->min(); }
double AcqAxis::max() const { return m_axis.isNull() ? 1.0 : m_axis->max(); }

void AcqAxis::setInterval(double min, double max)
{
	if(m_axis.isNull() || !(max > min)) {
		return;
	}
	m_reqMin = min;
	m_reqMax = max;
	m_axis->setInterval(min, max);
}

void AcqAxis::requestInterval(double min, double max)
{
	// The autoscaler owns the range while it is on; a read-driven write would fight it
	// every frame.
	if(m_autoscale || m_axis.isNull()) {
		return;
	}
	// A degenerate range comes out of an empty or single-sample stream. Qwt divides by
	// the span, so the previous range stays rather than the axis going unlabelled.
	if(!(max > min)) {
		return;
	}
	// Against the last request, not the axis's current interval, precisely so a
	// reader's zoom since then is not undone.
	if(qFuzzyCompare(min, m_reqMin) && qFuzzyCompare(max, m_reqMax)) {
		return;
	}
	m_reqMin = min;
	m_reqMax = max;
	m_axis->setInterval(min, max);
}

void AcqAxis::setUnit(const QString &u)
{
	if(m_source.unit == u) {
		return;
	}
	m_source.unit = u;
	// From here on a source change leaves the label alone: the reader has said what
	// this axis is in.
	m_unitOverridden = true;
	applyUnit();
}

void AcqAxis::refreshSourceChoices()
{
	if(m_sourceCombo.isNull()) {
		return;
	}
	QComboBox *combo = m_sourceCombo->combo();
	if(!combo) {
		return;
	}

	// Blocked and rebuilt wholesale: clear() emits currentIndexChanged, which would be
	// read as the reader retargeting the axis to whatever lands at index 0.
	QSignalBlocker blocker(combo);
	const QString previous = combo->currentData().toString();
	combo->clear();

	// Sample index appears twice under two different readings of the one ramp stream, and
	// is *not* listed again among the streams below: refreshSourceChoices() skips it
	// there, so the reader is not offered a third, raw entry that draws the same thing
	// with a confusing absolute range.
	combo->addItem(tr("sample index"), rampKeyString());
	combo->addItem(tr("time (s)"), timeComboData());

	if(!m_store.isNull()) {
		// Declared ∪ written: the two overlap but neither contains the other. A
		// declared-but-unwritten stream is selectable on purpose — claiming its depth
		// before the first cycle is what makes the first window full rather than one
		// chunk. Declarations come from the blocks; only the written set comes from
		// the store.
		QSet<scopy::acq::DataKey> all;
		if(!m_engine.isNull()) {
			const QList<scopy::acq::DataKey> declared = m_engine->declaredKeys();
			for(const scopy::acq::DataKey &k : declared) {
				all.insert(k);
			}
		}
		const QList<scopy::acq::DataKey> written = m_store->keys();
		for(const scopy::acq::DataKey &k : written) {
			all.insert(k);
		}

		QStringList names;
		names.reserve(all.size());
		for(const scopy::acq::DataKey &k : all) {
			const QString n = k.toString();
			// Already at the top, twice.
			if(n == rampKeyString()) {
				continue;
			}
			names << n;
		}
		// Sorted: a QSet iterates in hash order, which reshuffles the list every time
		// the key set changes and makes the picker unusable.
		names.sort();
		for(const QString &n : names) {
			combo->addItem(n, n);
		}
	}

	// The current source stays selected across a refresh even if its key has gone —
	// the channel is greyed rather than deleted in that case, and its axis should
	// still say what it was pointed at.
	const QString want = m_source.isTime() ? timeComboData() : m_source.key.toString();
	int idx = combo->findData(want);
	if(idx < 0 && !m_source.isIndexBased()) {
		combo->addItem(tr("%1 (absent)").arg(want), want);
		idx = combo->count() - 1;
	}
	if(idx < 0) {
		idx = combo->findData(previous);
	}
	combo->setCurrentIndex(qMax(0, idx));
}

QWidget *AcqAxis::createSettings(InstrumentTemplate *it, const QString &title, QWidget *parent)
{
	if(!it || m_axis.isNull()) {
		return nullptr;
	}

	// SO_VIEW: an axis is view state. Nothing here reaches the engine — retargeting the
	// source changes which stream is *read*, not what anything produces.
	MenuSectionCollapseWidget *section = it->createMenuSection(title, SO_VIEW, parent);

	// --- source ---------------------------------------------------------------
	m_sourceCombo = new MenuCombo(tr("Source"), section);
	refreshSourceChoices();
	connect(m_sourceCombo->combo(), &QComboBox::currentIndexChanged, this, [this](int) {
		if(m_sourceCombo.isNull() || !m_sourceCombo->combo()) {
			return;
		}
		const QString data = m_sourceCombo->combo()->currentData().toString();
		if(data == timeComboData()) {
			setSource(Source::time());
			return;
		}
		if(data == rampKeyString()) {
			setSource(Source::sampleIndex());
			return;
		}
		const scopy::acq::DataKey k = scopy::acq::DataKey(data);
		setSource(Source::stream(k, unitForKey(k)));
	});
	// Applies the fixed state to the combo that has only now been built.
	setSourceFixed(m_sourceFixed, m_fixedReason);
	section->add(m_sourceCombo);

	// --- autoscale + manual range ---------------------------------------------
	m_rangeCtrl = new scopy::gui::MenuPlotAxisRangeControl(m_axis.data(), section);

	if(m_autoscaleTarget) {
		// Only for a side that has a PlotChannel to measure. PlotAutoscaler takes a
		// PlotChannel*, so a digital track or a waterfall has nothing to give it, and a
		// switch that cannot do anything is worse than no switch.
		m_autoBtn = new MenuOnOffSwitch(tr("AUTOSCALE"), section, false);

		m_autoscaler = new scopy::gui::PlotAutoscaler(this);
		// The one place X and Y differ, and it is PlotAutoscaler's own distinction: it
		// scans the X array instead of the Y array.
		m_autoscaler->setXAxisMode(isHorizontal());
		if(m_autoscaleActive) {
			m_autoscaler->addChannels(m_autoscaleTarget);
		}
		connect(m_autoscaler.data(), &scopy::gui::PlotAutoscaler::newMin, m_rangeCtrl.data(),
			&scopy::gui::MenuPlotAxisRangeControl::setMin);
		connect(m_autoscaler.data(), &scopy::gui::PlotAutoscaler::newMax, m_rangeCtrl.data(),
			&scopy::gui::MenuPlotAxisRangeControl::setMax);

		connect(m_autoBtn->onOffswitch(), &QAbstractButton::toggled, this, [this](bool on) { setAutoscale(on); });
		QSignalBlocker b(m_autoBtn->onOffswitch());
		m_autoBtn->onOffswitch()->setChecked(m_autoscale);
		m_rangeCtrl->setEnabled(!m_autoscale);

		section->add(m_autoBtn);
	}
	section->add(m_rangeCtrl);

	// --- unit -----------------------------------------------------------------
	MenuLineEdit *unitEdit = new MenuLineEdit(section);
	unitEdit->edit()->setText(m_source.unit);
	unitEdit->edit()->setPlaceholderText(tr("unit"));
	connect(unitEdit->edit(), &QLineEdit::editingFinished, this,
		[this, unitEdit]() { setUnit(unitEdit->edit()->text()); });
	// A source change rewrites the label until the reader overrides it, and the field
	// has to show what the axis actually says.
	connect(this, &AcqAxis::sourceChanged, unitEdit, [this, unitEdit](scopy::acq::DataKey, scopy::acq::DataKey) {
		QSignalBlocker b(unitEdit->edit());
		unitEdit->edit()->setText(m_source.unit);
	});
	section->add(unitEdit);

	return section;
}

#include "moc_acqaxis.cpp"
