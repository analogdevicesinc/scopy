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

#include "bittogglegrid.h"

#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <algorithm>

#include <style.h>

using namespace scopy;
using namespace scopy::dcdebugger;

namespace {
constexpr int PER_ROW = 16;
} // namespace

BitToggleGrid::BitToggleGrid(QWidget *parent)
	: QWidget(parent)
	, m_grid(new QGridLayout(this))
{
	m_grid->setContentsMargins(0, 0, 0, 0);
	m_grid->setHorizontalSpacing(4);
	m_grid->setVerticalSpacing(2);
	// Keep the buttons packed top-left: the spare column and row take the slack.
	m_grid->setColumnStretch(PER_ROW, 1);
	rebuild();
}

void BitToggleGrid::setWidth(int bits)
{
	bits = std::clamp(bits, 1, 32);
	if(bits == m_bits) {
		return;
	}
	m_bits = bits;
	m_value &= bits >= 32 ? 0xFFFFFFFFu : ((1u << bits) - 1);
	rebuild();
}

void BitToggleGrid::setValue(uint32_t value)
{
	m_value = value & (m_bits >= 32 ? 0xFFFFFFFFu : ((1u << m_bits) - 1));
	refreshButtons();
}

void BitToggleGrid::rebuild()
{
	while(QLayoutItem *item = m_grid->takeAt(0)) {
		delete item->widget();
		delete item;
	}
	m_buttons.clear();
	m_buttons.resize(m_bits);
	// Row pairs: labels, then buttons; MSB in the top-left like a datasheet.
	for(int bit = m_bits - 1; bit >= 0; --bit) {
		const int pos = m_bits - 1 - bit;
		const int row = (pos / PER_ROW) * 2;
		const int col = pos % PER_ROW;
		auto *label = new QLabel(QString::number(bit), this);
		label->setAlignment(Qt::AlignHCenter);
		Style::setStyle(label, style::properties::label::subtle);
		auto *btn = new QPushButton("0", this);
		btn->setCheckable(true);
		btn->setFixedWidth(28);
		btn->setToolTip(QStringLiteral("bit %1").arg(bit));
		Style::setStyle(btn, style::properties::button::basicButton);
		connect(btn, &QPushButton::toggled, this, [this, bit](bool on) {
			const uint32_t b = 1u << bit;
			m_value = on ? (m_value | b) : (m_value & ~b);
			m_buttons[bit]->setText(on ? "1" : "0");
			Q_EMIT valueEdited(m_value);
		});
		m_grid->addWidget(label, row, col);
		m_grid->addWidget(btn, row + 1, col);
		m_buttons[bit] = btn;
	}
	const int rows = ((m_bits + PER_ROW - 1) / PER_ROW) * 2;
	for(int r = 0; r <= rows; ++r) {
		m_grid->setRowStretch(r, r == rows ? 1 : 0);
	}
	refreshButtons();
}

void BitToggleGrid::refreshButtons()
{
	for(int bit = 0; bit < m_buttons.size(); ++bit) {
		QPushButton *btn = m_buttons[bit];
		const bool on = (m_value >> bit) & 1u;
		QSignalBlocker block(btn);
		btn->setChecked(on);
		btn->setText(on ? "1" : "0");
	}
}

#include "moc_bittogglegrid.cpp"
