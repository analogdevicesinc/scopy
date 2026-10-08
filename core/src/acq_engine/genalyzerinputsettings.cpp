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

#include "genalyzerinputsettings.h"

#include "acquisitionengine.h"
#include "datakeycombo.h"
#include "datastore.h"
#include "genalyzerfftprocessor.h"

#include <gui/style.h>
#include <gui/widgets/menusectionwidget.h>

#include <QComboBox>
#include <QCoreApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QSignalBlocker>
#include <QSpacerItem>
#include <QVBoxLayout>

using namespace scopy;
using namespace scopy::acq;

namespace {

using FFTMode = GenalyzerFFTProcessor::FFTMode;

// Matches GenalyzerTransformSettings' layout so the block panels line up.
QWidget *labelledRow(const QString &text, QWidget *field, QWidget *parent, QLabel **labelOut = nullptr)
{
	auto *w = new QWidget(parent);
	auto *lay = new QHBoxLayout(w);
	lay->setContentsMargins(0, 0, 0, 0);

	auto *label = new QLabel(text, w);
	Style::setStyle(label, style::properties::label::subtle);

	lay->addWidget(label);
	lay->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	lay->addWidget(field);
	if(labelOut) {
		*labelOut = label;
	}
	return w;
}

// Selects `key`, inserting it as a visibly stale entry when the key set no longer offers it.
// populateKeyCombo() preserves whatever the box had, but a selection whose stream has gone would
// fall back to index 0 and silently retarget the FFT at a different channel.
void selectKey(QComboBox *box, const DataKey &key)
{
	if(!box || key.key.isEmpty()) {
		return;
	}
	QSignalBlocker b(box);
	if(box->findData(key.key) < 0) {
		box->insertItem(0, QCoreApplication::translate("GenalyzerInputSettings", "%1 (absent)").arg(key.key),
				key.key);
	}
	box->setCurrentIndex(box->findData(key.key));
}

} // namespace

GenalyzerInputSettings::GenalyzerInputSettings(GenalyzerFFTProcessor *proc, DataStore *store, AcquisitionEngine *engine,
					       QWidget *parent)
	: QWidget(parent)
	, m_proc(proc)
	, m_store(store)
	, m_engine(engine)
{
	setupUI();

	// This widget is owned by the menu page's layout while the block and the store belong to
	// the engine, so a teardown can destroy either of them first.
	if(m_proc) {
		connect(m_proc, &GenalyzerFFTProcessor::inputsChanged, this,
			[this](const QList<DataKey> &) { syncFromProcessor(); });
		// Disabling the block is what makes a retarget safe, so the pickers have to follow it.
		connect(m_proc, &GenalyzerFFTProcessor::enabledChanged, this, [this](bool) { updateState(); });
		connect(m_proc, &QObject::destroyed, this, [this]() {
			m_proc = nullptr;
			updateState();
		});
	}
	if(m_store) {
		connect(m_store, &QObject::destroyed, this, [this]() { m_store = nullptr; });
		// Queued: keysChanged comes off the worker thread, and refreshKeys() repopulates combos.
		// A stream appearing or going is exactly when the pickers are stale.
		connect(
			m_store, &DataStore::keysChanged, this, [this](const QList<DataKey> &) { refreshKeys(); },
			Qt::QueuedConnection);
	}
	if(m_engine) {
		// Queued for the same reason: stopped() is emitted from the worker as it winds down.
		connect(
			m_engine, &AcquisitionEngine::started, this, [this]() { updateState(); }, Qt::QueuedConnection);
		connect(
			m_engine, &AcquisitionEngine::stopped, this, [this]() { updateState(); }, Qt::QueuedConnection);
		connect(
			m_engine, &AcquisitionEngine::forceStopped, this, [this]() { updateState(); },
			Qt::QueuedConnection);
		// New blocks mean new declared keys, which is a key set change the store never sees
		// because nothing has been written to those keys yet.
		connect(
			m_engine, &AcquisitionEngine::blocksChanged, this, [this]() { refreshKeys(); },
			Qt::QueuedConnection);
		connect(m_engine, &QObject::destroyed, this, [this]() { m_engine = nullptr; });
	}

	syncFromProcessor();
}

void GenalyzerInputSettings::setupUI()
{
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

	auto *mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);

	auto *section = new MenuSectionWidget(this);
	Style::setStyle(section, style::properties::widget::border);
	section->contentLayout()->setSpacing(10);

	// --- Mode -----------------------------------------------------------
	//
	// Stored as the enum rather than a key count: "Complex I/Q" with only the I
	// half picked is a real state of this widget, and a count could not tell it
	// apart from a deliberate real transform.
	m_modeCombo = new QComboBox(section);
	m_modeCombo->addItem(tr("None"), static_cast<int>(FFTMode::None));
	m_modeCombo->addItem(tr("Real (float)"), static_cast<int>(FFTMode::Real));
	m_modeCombo->addItem(tr("Complex (I/Q)"), static_cast<int>(FFTMode::Complex));
	section->contentLayout()->addWidget(labelledRow(tr("Mode:"), m_modeCombo, section));

	// --- Input keys -----------------------------------------------------
	//
	// No sample-index entry: the ramp is an abscissa, and transforming it is not
	// a measurement of anything.
	m_firstKey = new QComboBox(section);
	populateKeyCombo(m_firstKey, m_store, m_engine, /*withSampleIndex=*/false);
	m_firstRow = labelledRow(tr("Input:"), m_firstKey, section, &m_firstLabel);
	section->contentLayout()->addWidget(m_firstRow);

	m_secondKey = new QComboBox(section);
	populateKeyCombo(m_secondKey, m_store, m_engine, /*withSampleIndex=*/false);
	m_secondRow = labelledRow(tr("Q (imag):"), m_secondKey, section);
	section->contentLayout()->addWidget(m_secondRow);

	m_hint = new QLabel(section);
	m_hint->setWordWrap(true);
	Style::setStyle(m_hint, style::properties::label::subtle);
	section->contentLayout()->addWidget(m_hint);

	mainLayout->addWidget(section);

	// --- Wiring ---------------------------------------------------------
	connect(m_modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
		if(m_building) {
			return;
		}
		// Both combos were filled with the same list and both sit at index 0, so a
		// freshly revealed Q picker would name the I stream. Stepping it on once is a
		// default, not a rule: the reader can point them at the same stream if they
		// want, it just should not be what picking "Complex" silently does.
		if(static_cast<FFTMode>(m_modeCombo->currentData().toInt()) == FFTMode::Complex &&
		   m_secondKey->currentIndex() == m_firstKey->currentIndex() && m_secondKey->count() > 1) {
			QSignalBlocker b(m_secondKey);
			m_secondKey->setCurrentIndex((m_firstKey->currentIndex() + 1) % m_secondKey->count());
		}
		// Rows first: switching to complex reveals the Q picker, and pushInputs()
		// has to see whatever it is already pointing at.
		updateState();
		pushInputs();
	});
	connect(m_firstKey, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
		if(!m_building) {
			pushInputs();
		}
	});
	connect(m_secondKey, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
		if(!m_building) {
			pushInputs();
		}
	});
}

void GenalyzerInputSettings::syncFromProcessor()
{
	const QList<DataKey> keys = m_proc ? m_proc->inputs() : QList<DataKey>();

	m_building = true;

	// Only from a non-empty list. The block's mode is its key count, so an empty
	// list is equally "None" and "Complex, one half still to pick" — and taking it
	// as None would throw away a mode the reader just chose, every time the pick is
	// still incomplete.
	if(!keys.isEmpty()) {
		const FFTMode mode = m_proc->mode();
		m_modeCombo->setCurrentIndex(m_modeCombo->findData(static_cast<int>(mode)));
	}

	if(keys.size() > 0) {
		selectKey(m_firstKey, keys.at(0));
	}
	if(keys.size() > 1) {
		selectKey(m_secondKey, keys.at(1));
	}

	m_building = false;

	updateState();
}

void GenalyzerInputSettings::refreshKeys()
{
	// In place rather than a rebuild: populateKeyCombo preserves each selection, so a key
	// appearing mid-edit does not disturb the reader.
	m_building = true;
	populateKeyCombo(m_firstKey, m_store, m_engine, /*withSampleIndex=*/false);
	populateKeyCombo(m_secondKey, m_store, m_engine, /*withSampleIndex=*/false);

	// Re-asserted from the block: a selection whose stream has gone is the one case
	// populateKeyCombo cannot preserve.
	const QList<DataKey> keys = m_proc ? m_proc->inputs() : QList<DataKey>();
	if(keys.size() > 0) {
		selectKey(m_firstKey, keys.at(0));
	}
	if(keys.size() > 1) {
		selectKey(m_secondKey, keys.at(1));
	}
	m_building = false;

	updateState();
}

void GenalyzerInputSettings::pushInputs()
{
	if(!m_proc) {
		return;
	}

	const auto mode = static_cast<FFTMode>(m_modeCombo->currentData().toInt());
	const DataKey first = keyFromCombo(m_firstKey);
	const DataKey second = keyFromCombo(m_secondKey);

	QList<DataKey> keys;
	switch(mode) {
	case FFTMode::Real:
		if(!first.key.isEmpty()) {
			keys << first;
		}
		break;
	case FFTMode::Complex:
		// Both halves or neither. A complex pair with one key missing is not a
		// real transform of the other — it would measure a different thing and
		// label it the same way, so the block idles until the pair is complete.
		if(!first.key.isEmpty() && !second.key.isEmpty()) {
			keys << first << second;
		}
		break;
	case FFTMode::None:
		break;
	}

	m_proc->setInputs(keys);
	// setInputs() is a no-op for an unchanged list and so emits nothing; the hint
	// still has to follow an incomplete selection.
	updateState();
}

void GenalyzerInputSettings::updateState()
{
	const auto mode = static_cast<FFTMode>(m_modeCombo->currentData().toInt());
	const bool complex = (mode == FFTMode::Complex);
	const bool real = (mode == FFTMode::Real);

	m_firstRow->setVisible(complex || real);
	m_secondRow->setVisible(complex);
	// "Input" for one stream, "I (real)" for the half of a pair — the same picker
	// means a different thing in each mode, and the label is where that shows.
	m_firstLabel->setText(complex ? tr("I (real):") : tr("Input:"));

	// The engine reads watchedKeys() on the worker thread and skips that read only
	// for a disabled block, so retargeting an enabled block mid-run is the one
	// thing this widget must not allow. Greyed rather than hidden: the reader needs
	// to see what it is pointed at, and the hint says how to change it.
	const bool locked = m_engine && m_engine->isRunning() && m_proc && m_proc->isEnabled();
	m_modeCombo->setEnabled(!locked);
	m_firstKey->setEnabled(!locked);
	m_secondKey->setEnabled(!locked);

	if(locked) {
		m_hint->setText(tr("Running — stop the acquisition or disable this block to change its inputs."));
		return;
	}

	const bool incomplete = (real && keyFromCombo(m_firstKey).key.isEmpty()) ||
		(complex && (keyFromCombo(m_firstKey).key.isEmpty() || keyFromCombo(m_secondKey).key.isEmpty()));

	if(mode == FFTMode::None) {
		m_hint->setText(tr("No inputs — pick one stream for a real FFT, or two for a complex I/Q pair."));
	} else if(incomplete) {
		m_hint->setText(tr("Waiting for a complete selection — nothing is transformed until then."));
	} else if(complex) {
		m_hint->setText(
			tr("Complex FFT over %1 bins, spanning -fs/2 to fs/2.").arg(m_proc ? m_proc->nfft() : 0));
	} else {
		m_hint->setText(
			tr("Real FFT over %1 bins, spanning 0 to fs/2.").arg(m_proc ? m_proc->nfft() / 2 + 1 : 0));
	}
}

#include "moc_genalyzerinputsettings.cpp"
