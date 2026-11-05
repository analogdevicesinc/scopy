/*
 * Copyright (c) 2026 Analog Devices Inc.
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

#ifndef BITTOGGLEGRID_H
#define BITTOGGLEGRID_H

#include "scopy-dcdebugger_export.h"

#include <QList>
#include <QWidget>

class QGridLayout;
class QPushButton;

namespace scopy::dcdebugger {

// One checkable button per bit, MSB first, 16 per row, the bit number above each. Clicking a
// bit flips it in value() and emits valueEdited(); nothing is written by itself.
class SCOPY_DCDEBUGGER_EXPORT BitToggleGrid : public QWidget
{
	Q_OBJECT
public:
	explicit BitToggleGrid(QWidget *parent = nullptr);

	void setWidth(int bits);
	int width() const { return m_bits; }
	void setValue(uint32_t value);
	uint32_t value() const { return m_value; }
	QPushButton *bitButton(int bit) const { return bit >= 0 && bit < m_buttons.size() ? m_buttons[bit] : nullptr; }

Q_SIGNALS:
	void valueEdited(uint32_t value);

private:
	void rebuild();
	void refreshButtons();

	QGridLayout *m_grid;
	QList<QPushButton *> m_buttons; // index = bit
	int m_bits = 32;
	uint32_t m_value = 0;
};

} // namespace scopy::dcdebugger

#endif // BITTOGGLEGRID_H
