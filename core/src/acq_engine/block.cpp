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

#include "block.h"

#include "acquisitionengine.h"

#include <QCheckBox>
#include <QVBoxLayout>
#include <QWidget>

namespace scopy {
namespace acq {

Block::Block(const QString &name, QObject *parent)
	: QObject(parent)
	, m_name(name)
{}

void Block::setEnabled(bool en)
{
	if(m_enabled.exchange(en, std::memory_order_relaxed) != en)
		Q_EMIT enabledChanged(en);
}

void Block::report(AcquisitionError::Severity sev, const QString &msg) const
{
	auto *engine = qobject_cast<AcquisitionEngine *>(parent());
	if(!engine || sev < engine->minReportSeverity())
		return;
	Q_EMIT engine->error(static_cast<int>(sev), m_name, msg);
}

void Block::reportOnce(AcquisitionError::Severity sev, const QString &msg) const
{
	// Before the severity gate, so the remembered message does not depend on the
	// threshold: raising the threshold mid-run would otherwise let the next
	// occurrence through as if it were new.
	if(m_lastReportOnce == msg)
		return;
	m_lastReportOnce = msg;
	report(sev, msg);
}

bool Block::wantsReport(AcquisitionError::Severity sev) const
{
	auto *engine = qobject_cast<AcquisitionEngine *>(parent());
	return engine && sev >= engine->minReportSeverity();
}

QWidget *Block::settingsWidget(QWidget *parent)
{
	if(m_settingsWidget.isNull())
		m_settingsWidget = createSettingsWidget(parent);
	return m_settingsWidget;
}

void Block::setSettingsWidget(QWidget *w)
{
	if(!w || !m_settingsWidget.isNull())
		return;
	m_settingsWidget = w;
}

QWidget *Block::createSettingsWidget(QWidget *parent)
{
	auto *cb = new QCheckBox(QStringLiteral("Enabled"), parent);
	cb->setChecked(isEnabled());
	connect(cb, &QCheckBox::toggled, this, [this](bool en) { setEnabled(en); });
	connect(this, &Block::enabledChanged, cb, &QCheckBox::setChecked);
	return cb;
}

DataStore *Block::engineStore() const
{
	auto *engine = qobject_cast<AcquisitionEngine *>(parent());
	return engine ? engine->store() : nullptr;
}

QWidget *Block::withBaseSettings(QWidget *own, QWidget *parent)
{
	auto *w = new QWidget(parent);
	auto *lay = new QVBoxLayout(w);
	lay->setContentsMargins(0, 0, 0, 0);
	lay->setSpacing(4);
	lay->addWidget(Block::createSettingsWidget(w));
	if(own) {
		own->setParent(w);
		lay->addWidget(own);
	}
	return w;
}

} // namespace acq
} // namespace scopy

#include "moc_block.cpp"
