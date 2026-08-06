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
#include <gui/widgets/menulineedit.h>
#include <gui/widgets/menusectionwidget.h>

#include <QLabel>
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
	m_store->claimDepth(m_key, m_claimant, m_repr->claimDepth(plotSize, bufferSize));
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

	lay->addStretch();
	return page;
}

#include "moc_acqchannel.cpp"
