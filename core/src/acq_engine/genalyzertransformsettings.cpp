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

#include "genalyzertransformsettings.h"

#include "datakey.h"
#include "genalyzerfftprocessor.h"

#include <gui/style.h>
#include <gui/widgets/menuonoffswitch.h>
#include <gui/widgets/menusectionwidget.h>

#include <QAbstractButton>
#include <QComboBox>
#include <QDoubleValidator>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpacerItem>
#include <QSpinBox>
#include <QVBoxLayout>

using namespace scopy;
using namespace scopy::acq;

namespace {
// A labelled row, matching GenalyzerSettings' layout so the two panels line up.
QWidget *labelledRow(const QString &text, QWidget *field, QWidget *parent)
{
	auto *w = new QWidget(parent);
	auto *lay = new QHBoxLayout(w);
	lay->setContentsMargins(0, 0, 0, 0);

	auto *label = new QLabel(text, w);
	Style::setStyle(label, style::properties::label::subtle);

	lay->addWidget(label);
	lay->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	lay->addWidget(field);
	return w;
}
} // namespace

GenalyzerTransformSettings::GenalyzerTransformSettings(GenalyzerFFTProcessor *proc, QWidget *parent)
	: QWidget(parent)
	, m_proc(proc)
{
	setupUI();
	syncFromProcessor();
}

void GenalyzerTransformSettings::setupUI()
{
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

	auto *mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);

	auto *section = new MenuSectionWidget(this);
	Style::setStyle(section, style::properties::widget::border);
	section->contentLayout()->setSpacing(10);

	// --- Window ---------------------------------------------------------
	m_windowCombo = new QComboBox(section);
	m_windowCombo->addItem(QStringLiteral("Hann"), static_cast<int>(GnWindowHann));
	m_windowCombo->addItem(QStringLiteral("Blackman-Harris"), static_cast<int>(GnWindowBlackmanHarris));
	m_windowCombo->addItem(QStringLiteral("Rectangular"), static_cast<int>(GnWindowNoWindow));
	section->contentLayout()->addWidget(labelledRow(QStringLiteral("Window:"), m_windowCombo, section));

	// --- Real-mode dBFS reference ---------------------------------------
	m_scaleCombo = new QComboBox(section);
	m_scaleCombo->addItem(QStringLiteral("Sine full-scale (0 dBFS)"), static_cast<int>(GnRfftScaleDbfsSin));
	m_scaleCombo->addItem(QStringLiteral("DC full-scale (-3 dBFS)"), static_cast<int>(GnRfftScaleDbfsDc));
	m_scaleCombo->addItem(QStringLiteral("Native (-6 dBFS)"), static_cast<int>(GnRfftScaleNative));
	m_scaleContainer = labelledRow(QStringLiteral("dBFS reference:"), m_scaleCombo, section);
	section->contentLayout()->addWidget(m_scaleContainer);

	// --- Sample rate ----------------------------------------------------
	m_sampleRateEdit = new QLineEdit(section);
	// Positive only: the frequency axis is built from fs, and a negative or zero
	// rate produces a degenerate axis rather than a diagnosable error.
	m_sampleRateEdit->setValidator(new QDoubleValidator(1.0, 1e12, 6, this));
	section->contentLayout()->addWidget(
		labelledRow(QStringLiteral("Sample rate (Hz):"), m_sampleRateEdit, section));

	// --- Averaging ------------------------------------------------------
	m_avgSwitch = new MenuOnOffSwitch(QStringLiteral("Averaging"), section, false);
	section->contentLayout()->addWidget(m_avgSwitch);

	m_avgSpinbox = new QSpinBox(section);
	// Lower bound 2: navg of 1 is "off", which the switch expresses.
	m_avgSpinbox->setRange(2, 1000);
	m_avgSpinbox->setValue(2);
	m_avgSizeContainer = labelledRow(QStringLiteral("Frames:"), m_avgSpinbox, section);
	section->contentLayout()->addWidget(m_avgSizeContainer);

	// --- Derived read-outs ----------------------------------------------
	m_nfftValue = new QLabel(section);
	Style::setStyle(m_nfftValue, style::properties::label::subtle);
	section->contentLayout()->addWidget(labelledRow(QStringLiteral("NFFT:"), m_nfftValue, section));

	m_rbwValue = new QLabel(section);
	Style::setStyle(m_rbwValue, style::properties::label::subtle);
	section->contentLayout()->addWidget(labelledRow(QStringLiteral("Bin width (Hz):"), m_rbwValue, section));

	mainLayout->addWidget(section);

	// --- Wiring ---------------------------------------------------------
	connect(m_windowCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
		if(m_proc)
			m_proc->setWindow(static_cast<GnWindow>(m_windowCombo->currentData().toInt()));
	});

	connect(m_scaleCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
		if(m_proc)
			m_proc->setRfftScale(static_cast<GnRfftScale>(m_scaleCombo->currentData().toInt()));
	});

	// editingFinished, not textChanged: a rate typed digit by digit would push
	// "2", "24", "240"… each of which rebuilds the axis and drops both gn configs.
	connect(m_sampleRateEdit, &QLineEdit::editingFinished, this, [this]() {
		if(!m_proc)
			return;
		bool ok = false;
		const double fs = m_sampleRateEdit->text().toDouble(&ok);
		if(ok && fs > 0.0)
			m_proc->setSampleRate(fs);
		else
			m_sampleRateEdit->setText(QString::number(m_proc->sampleRate()));
	});

	connect(m_avgSwitch->onOffswitch(), &QAbstractButton::toggled, this, [this](bool on) {
		m_avgSizeContainer->setVisible(on);
		if(m_proc)
			m_proc->setAveraging(on ? m_avgSpinbox->value() : 1);
	});

	connect(m_avgSpinbox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int v) {
		if(m_proc && m_avgSwitch->onOffswitch()->isChecked())
			m_proc->setAveraging(v);
	});

	if(m_proc) {
		// Queued: these come off the acquisition worker thread.
		connect(
			m_proc, &GenalyzerFFTProcessor::nfftChanged, this, [this](int) { updateDerived(); },
			Qt::QueuedConnection);
		connect(m_proc, &GenalyzerFFTProcessor::sampleRateChanged, this,
			&GenalyzerTransformSettings::syncFromProcessor, Qt::QueuedConnection);
		connect(
			m_proc, &GenalyzerFFTProcessor::averagingChanged, this, [this](int) { updateDerived(); },
			Qt::QueuedConnection);
		// The mode is the input count, so retargeting the block changes the bin
		// count and whether the dBFS reference applies at all. Direct: inputs are
		// only ever set from the GUI thread.
		connect(m_proc, &GenalyzerFFTProcessor::inputsChanged, this,
			[this](const QList<DataKey> &) { updateDerived(); });
	}
}

void GenalyzerTransformSettings::syncFromProcessor()
{
	if(!m_proc)
		return;

	m_windowCombo->blockSignals(true);
	m_scaleCombo->blockSignals(true);
	m_sampleRateEdit->blockSignals(true);
	m_avgSwitch->onOffswitch()->blockSignals(true);
	m_avgSpinbox->blockSignals(true);

	m_windowCombo->setCurrentIndex(m_windowCombo->findData(static_cast<int>(m_proc->window())));
	m_scaleCombo->setCurrentIndex(m_scaleCombo->findData(static_cast<int>(m_proc->rfftScale())));
	m_sampleRateEdit->setText(QString::number(m_proc->sampleRate()));

	const int navg = m_proc->averaging();
	m_avgSwitch->onOffswitch()->setChecked(navg > 1);
	if(navg > 1)
		m_avgSpinbox->setValue(navg);
	m_avgSizeContainer->setVisible(navg > 1);

	m_windowCombo->blockSignals(false);
	m_scaleCombo->blockSignals(false);
	m_sampleRateEdit->blockSignals(false);
	m_avgSwitch->onOffswitch()->blockSignals(false);
	m_avgSpinbox->blockSignals(false);

	updateDerived();
}

void GenalyzerTransformSettings::updateDerived()
{
	if(!m_proc)
		return;

	const int nfft = m_proc->nfft();
	const double fs = m_proc->sampleRate();
	const auto mode = m_proc->mode();

	// Complex mode spans [-fs/2, fs/2) over nfft bins, real mode [0, fs/2] over
	// nfft/2+1; the bin width is fs/nfft either way. With no inputs picked there
	// is no transform and so no bin count — nfft is still shown, because it is
	// what the next cycle would run with once the reader picks a stream.
	if(mode == GenalyzerFFTProcessor::FFTMode::None) {
		m_nfftValue->setText(QStringLiteral("%1 (no inputs)").arg(nfft));
	} else {
		const int bins = (mode == GenalyzerFFTProcessor::FFTMode::Real) ? nfft / 2 + 1 : nfft;
		m_nfftValue->setText(QStringLiteral("%1 (%2 bins)").arg(nfft).arg(bins));
	}
	m_rbwValue->setText(nfft > 0 ? QString::number(fs / nfft, 'g', 6) : QStringLiteral("—"));

	// gn_fft takes no scaling argument, so the reference only means something in
	// real mode. Hide rather than disable: a greyed control invites the question
	// of what would enable it, and the answer is "watch one key instead of two".
	m_scaleContainer->setVisible(mode == GenalyzerFFTProcessor::FFTMode::Real);
}

#include "moc_genalyzertransformsettings.cpp"
