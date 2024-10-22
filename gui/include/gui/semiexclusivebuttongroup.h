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

#ifndef SEMIEXCLUSIVEBUTTONGROUP_H
#define SEMIEXCLUSIVEBUTTONGROUP_H

#include "scopy-gui_export.h"

#include <QAbstractButton>
#include <QButtonGroup>

namespace scopy {
class SCOPY_GUI_EXPORT SemiExclusiveButtonGroup : public QButtonGroup
{
	Q_OBJECT

public:
	explicit SemiExclusiveButtonGroup(QObject *parent = nullptr);

	void setExclusive(bool bExclusive);
	bool exclusive() const;

	QAbstractButton *getSelectedButton();
	QAbstractButton *getLastButton();

Q_SIGNALS:
	void buttonSelected(QAbstractButton *btn);

protected Q_SLOTS:
	void buttonClicked(QAbstractButton *button);

protected:
	bool m_exclusive;
	QAbstractButton *m_lastBtn;
};
} // namespace scopy
#endif // SEMIEXCLUSIVEBUTTONGROUP_H
