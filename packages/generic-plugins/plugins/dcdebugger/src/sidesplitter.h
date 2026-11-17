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

#ifndef SIDESPLITTER_H
#define SIDESPLITTER_H

#include <QSplitter>

namespace scopy {
class MapStackedWidget;
}

namespace scopy::dcdebugger {

// Instrument body as a splitter: [left stack (channels)] [central] [right stack (menus)], like the
// ADC instrument. The handles resize the side stacks or hide them (dragged to the edge); a hidden
// stack comes back at its last width. Showing a page of the right stack opens it.
class SideSplitter : public QSplitter
{
	Q_OBJECT
public:
	// No left stack for leftWidth == 0.
	SideSplitter(int leftWidth, int rightWidth, QWidget *parent = nullptr);

	// Between the stacks; once.
	void setCentralWidget(QWidget *central);

	MapStackedWidget *leftStack() const { return m_left; }
	MapStackedWidget *rightStack() const { return m_right; }
	bool isRightOpen() const;

public Q_SLOTS:
	void setLeftOpen(bool open);
	void setRightOpen(bool open);

Q_SIGNALS:
	// Also when the right stack is dragged closed / open.
	void rightOpenChanged(bool open);

private:
	void setOpen(QWidget *side, int &width, bool open);
	void onMoved();

	MapStackedWidget *m_left = nullptr;
	MapStackedWidget *m_right;
	QWidget *m_central = nullptr;
	int m_leftWidth;
	int m_rightWidth;
	bool m_rightOpen = true;
};

} // namespace scopy::dcdebugger

#endif // SIDESPLITTER_H
