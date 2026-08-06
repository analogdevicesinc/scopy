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

#include "acqplotrow.h"

#include <core/acq_engine/DataStore.h>

#include <gui/instrumenttemplate.h>
#include <gui/style.h>
#include <gui/widgets/menulineedit.h>
#include <gui/widgets/menusectionwidget.h>

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

using namespace scopy;
using namespace scopy::adc;

AcqChannel::AcqChannel(scopy::acq::DataStore *store, const scopy::acq::DataKey &key, const QString &name,
		       const QColor &color, std::unique_ptr<AcqChannelRepr> repr, int uid, QObject *parent)
	: QObject(parent)
	, m_store(store)
	, m_key(key)
	, m_name(name)
	, m_color(color)
	, m_uid(uid)
	, m_claimant(QStringLiteral("acqch-%1").arg(uid))
	, m_repr(std::move(repr))
{
	// No depth claim here: plotSize and bufferSize belong to the manager, which
	// calls reclaimDepth() right after attaching us.
	//
	// But a repr can change its own requirement later (a waterfall's row count), and
	// it knows neither plotSize nor bufferSize. Forward it as a signal so the manager
	// can re-claim with the numbers it owns. Capturing `this` is safe: the repr is a
	// member and cannot outlive us.
	if(m_repr) {
		m_repr->setReclaimNotifier([this]() { Q_EMIT depthNeedsReclaim(); });
	}
}

AcqChannel::~AcqChannel()
{
	detach();
	if(!m_store.isNull()) {
		// Sweeps every key this claimant holds in one call, so it stays correct even
		// if key retargeting is ever added.
		m_store->releaseClaimant(m_claimant);
	}
}

QString AcqChannel::menuId() const { return QStringLiteral("acqch:%1").arg(m_uid); }

void AcqChannel::attach(AcqPlotRow *row)
{
	if(!row || !m_repr) {
		return;
	}
	m_row = row;
	m_repr->attach(row, m_name, m_color);
	m_repr->setEnabled(m_enabled);
}

void AcqChannel::detach()
{
	if(m_repr) {
		m_repr->detach();
	}
	m_row = nullptr;
}

void AcqChannel::pull(int plotSize)
{
	if(!m_enabled || m_store.isNull() || !m_repr) {
		return;
	}
	m_repr->pull(m_store.data(), m_key, plotSize);
}

void AcqChannel::reset()
{
	if(m_repr) {
		m_repr->reset();
	}
}

void AcqChannel::reclaimDepth(int plotSize, std::size_t bufferSize)
{
	if(m_store.isNull() || !m_repr) {
		return;
	}
	const std::size_t depth = m_repr->claimDepth(plotSize, bufferSize);
	m_store->claimDepth(m_key, m_claimant, depth);

	// The repr's secondary keys — a curve's X stream — get the same depth. Not
	// cosmetic: CurveRepr::pull truncates to qMin(x.size, y.size), so an X key left at
	// the default capacity of 1 would clip a multi-chunk Y window down to one buffer.
	const QList<scopy::acq::DataKey> extra = m_repr->extraKeys();
	for(const scopy::acq::DataKey &k : extra) {
		if(k.key.isEmpty() || k == m_key) {
			continue;
		}
		m_store->claimDepth(k, m_claimant, depth);
	}

	// Release the keys we claimed last time and no longer read. The X key is
	// user-editable, so retargeting it would otherwise leave the old key pinned at this
	// channel's depth for the life of the instrument, with nothing reading it.
	for(const scopy::acq::DataKey &old : std::as_const(m_claimedExtra)) {
		if(old == m_key || extra.contains(old)) {
			continue;
		}
		m_store->releaseDepth(old, m_claimant);
	}
	m_claimedExtra = extra;
}

void AcqChannel::setEnabled(bool en)
{
	if(m_enabled == en) {
		return;
	}
	m_enabled = en;
	if(m_repr) {
		m_repr->setEnabled(en);
	}
	Q_EMIT enabledChanged(en);
}

void AcqChannel::setName(const QString &n)
{
	if(m_name == n) {
		return;
	}
	m_name = n;
	if(m_repr) {
		m_repr->setName(n);
	}
	Q_EMIT nameChanged(n);
}

void AcqChannel::setColor(const QColor &c)
{
	if(m_color == c) {
		return;
	}
	m_color = c;
	if(m_repr) {
		m_repr->setColor(c);
	}
	Q_EMIT colorChanged(c);
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

	lay->addWidget(chSection);

	// The repr's own knobs, under the generic ones — the same generic-then-specific
	// stacking Block::withBaseSettings() uses.
	if(m_repr) {
		if(QWidget *own = m_repr->createSettingsWidget(page)) {
			MenuSectionCollapseWidget *reprSection = it->createMenuSection(m_repr->kindName(), SO_VIEW, page);
			reprSection->add(own);
			lay->addWidget(reprSection);
		}
	}

	// Last, below the repr's own knobs and outside every section: it acts on the channel
	// as a whole, not on one aspect of it, and putting it inside a collapsible section
	// would let it hide.
	QPushButton *delBtn = new QPushButton(tr("Delete channel"), page);
	Style::setStyle(delBtn, style::properties::button::basicButton);
	// Emitted directly; it is the *manager's* connection to removeRequested that is
	// queued, so the channel and this button are both off the stack by the time either
	// is destroyed. Deferring here instead would not help: the lambda's `this` would be
	// the channel being deleted inside the emit.
	connect(delBtn, &QPushButton::clicked, this, [this]() { Q_EMIT removeRequested(); });
	lay->addWidget(delBtn);

	lay->addStretch();
	return page;
}

#include "moc_acqchannel.cpp"
