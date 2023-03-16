/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef DB_CLICK_BUTTONS_HPP
#define DB_CLICK_BUTTONS_HPP

#include <QWidget>
#include <QPushButton>
#include <QList>
#include <QString>
#include "scopygui_export.h"

namespace Ui {
class DbClickButtons;
}

namespace SCOPYGUI_EXPORT adiscope {

class DbClickButtons: public QWidget
{
	Q_OBJECT

public:
	explicit DbClickButtons(QWidget *parent = 0, int maxRowBtnCount = 5);
	~DbClickButtons();

	int selectedButton() const;
	void setSelectedButton(int btnId);

	bool buttonChecked(int btnId) const;
	void setButtonChecked(int btnId, bool checked);

	int buttonCount() const;
	void setButtonCount(int count);

Q_SIGNALS:
	void buttonSelected(int);
	void buttonToggled(int, bool);

private Q_SLOTS:
	void onButtonClicked();

private:
	void toggleButton(int btnId);

private:
	Ui::DbClickButtons *ui;
	QList<QPushButton *> btn_list;
	QList<bool> btn_states;
	int selected_btn;
	int max_row_btn_cnt;
	QList<QString> color_codes;
};

} // namespace adiscope

#endif // DB_CLICK_BUTTONS_HPP
