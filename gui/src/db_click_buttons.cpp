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
#include "db_click_buttons.hpp"
#include "ui_db_click_buttons.h"
#include "dynamicWidget.h"

#include <QGridLayout>

using namespace adiscope;

/*
 * The DbClickButtons class is intended to be a UI menu that contains a
 * configurable number of buttons. What is different about it is that the first
 * click landed on a button will only select it but not toggled it. Only after a
 * button has been selected it can be toggled.
 */

DbClickButtons::DbClickButtons(QWidget *parent, int maxRowBtnCount) :
	QWidget(parent),
	ui(new Ui::DbClickButtons),
	btn_states(),
	selected_btn(-1),
	max_row_btn_cnt(maxRowBtnCount > 0 ? maxRowBtnCount : 1)
{
	ui->setupUi(this);

	color_codes << "#F44336" << "#2096F3" << "#8BC34A" << "#FF5721" <<
		"#607D8B";

	btn_list = findChildren<QPushButton *>();

	int n = 0;
	for (const auto& i : qAsConst(btn_list)) {
		btn_states.push_back(false);
		i->setProperty("id", QVariant(n++));
		connect(i, SIGNAL(clicked()),
			this, SLOT(onButtonClicked()));

	}
}

DbClickButtons::~DbClickButtons()
{
	delete ui;
}

int DbClickButtons::selectedButton() const
{
	return selected_btn;
}

void DbClickButtons::setSelectedButton(int btnId)
{
	for (int i = 0; i < btn_list.size(); i++) {
		bool selected;
		if (i == btnId) {
			selected = true;
		} else {
			selected = false;
		}

		setDynamicProperty(btn_list[i], "is_selected", selected);
		btn_list[i]->update();

		if (selected) {
			selected_btn = btnId;
			Q_EMIT buttonSelected(btnId);
		}
	}
}

bool DbClickButtons::buttonChecked(int btnId) const
{
	return btn_states[btnId];
}

void DbClickButtons::setButtonChecked(int btnId, bool checked)
{
	btn_states[btnId] = checked;
	setDynamicProperty(btn_list[btnId], "is_checked", checked);
	btn_list[btnId]->update();

	Q_EMIT buttonToggled(btnId, checked);
}

int DbClickButtons::buttonCount() const
{
	return btn_list.size();
}

void DbClickButtons::setButtonCount(int count)
{
	if (count != btn_list.size()) {
		for (int i = 0; i < btn_list.size(); i++) {
			delete btn_list[i];
		}
		selected_btn = -1;
		btn_list.clear();

		QGridLayout *gLayout = static_cast<QGridLayout *>(layout());
		for (int i = 0; i < count; i++) {
			QPushButton *btn = new QPushButton(this);
			btn->setText(QString::number(i));
			btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
			btn->setProperty("id", QVariant(i));
			btn->setProperty("is_checked", QVariant(false));
			btn->setProperty("is_selected", QVariant(false));
			QString color = color_codes[i % color_codes.size()];
			QString stylesheet = QString ("QPushButton"
				"{\n  border: 2px solid %1;\n}\n"
				"\nQPushButton[is_checked=true] {\n  "
				"background-color: %1;\n}\n"
				"\nQPushButton[is_selected=true] {\n	"
				"border: 4px solid white;\n}\n").arg(color);
			btn->setStyleSheet(stylesheet);
			connect(btn, SIGNAL(clicked()),
				this, SLOT(onButtonClicked()));

			int row = i / max_row_btn_cnt;
			int col = i % max_row_btn_cnt;
			gLayout->addWidget(btn, row, col);
			btn_list.push_back(btn);
			btn_states.push_back(false);
		}
	}
}

void DbClickButtons::onButtonClicked()
{
	QPushButton *btn = static_cast<QPushButton *>(QObject::sender());
	int btnId = btn->property("id").toInt();

	if (selected_btn < 0) {
		setSelectedButton(btnId);
	} else if (btn == btn_list[selected_btn]) {
		 toggleButton(btnId);
	} else {
		setSelectedButton(btnId);
	}
}

void DbClickButtons::toggleButton(int btnId)
{
	setButtonChecked(btnId, !btn_states[btnId]);
}
