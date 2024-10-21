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

#include "semiexclusivebuttongroup.h"

using namespace scopy;

SemiExclusiveButtonGroup::SemiExclusiveButtonGroup(QObject *parent)
	: QButtonGroup(parent)
{
	m_exclusive = true;
	m_lastBtn = nullptr;
	QButtonGroup::setExclusive(false);
	connect(this, SIGNAL(buttonClicked(QAbstractButton *)), SLOT(buttonClicked(QAbstractButton *)));
}

void SemiExclusiveButtonGroup::setExclusive(bool bExclusive) { m_exclusive = bExclusive; }

bool SemiExclusiveButtonGroup::exclusive() const { return m_exclusive; }

QAbstractButton *SemiExclusiveButtonGroup::getSelectedButton()
{
	QList<QAbstractButton *> buttonlist = buttons();
	for(auto iBtn = buttonlist.begin(); iBtn != buttonlist.end(); ++iBtn) {
		QAbstractButton *pBtn = *iBtn;
		if(pBtn->isChecked()) {
			return pBtn;
		}
	}
	return nullptr;
}

QAbstractButton *SemiExclusiveButtonGroup::getLastButton()
{
	if(m_lastBtn == nullptr) {
		m_lastBtn = buttons()[0];
	}
	return m_lastBtn;
}

void SemiExclusiveButtonGroup::buttonClicked(QAbstractButton *button)
{
	QList<QAbstractButton *> buttonlist = buttons();
	m_lastBtn = button;
	if(m_exclusive) {
		// just uncheck all other buttons regardless of the state of the clicked button

		for(auto iBtn = buttonlist.begin(); iBtn != buttonlist.end(); ++iBtn) {
			QAbstractButton *pBtn = *iBtn;
			if(pBtn && pBtn != button && pBtn->isCheckable())
				pBtn->setChecked(false);
		}
	}
	Q_EMIT buttonSelected(getSelectedButton());
}
