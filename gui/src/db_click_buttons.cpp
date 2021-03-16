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
#include "dynamic_widget.hpp"

#include "ui_db_click_buttons.h"

#include <QGridLayout>

#include <scopy/gui/db_click_buttons.hpp>

using namespace scopy::gui;

/*
 * The DbClickButtons class is intended to be a UI menu that contains a
 * configurable number of buttons. What is different about it is that the first
 * click landed on a button will only select it but not toggled it. Only after a
 * button has been selected it can be toggled.
 */

DbClickButtons::DbClickButtons(QWidget* parent, int maxRowBtnCount)
	: QWidget(parent)
	, m_ui(new Ui::DbClickButtons)
	, m_btnStates()
	, m_selectedBtn(-1)
	, m_maxRowBtnCnt(maxRowBtnCount > 0 ? maxRowBtnCount : 1)
{
	m_ui->setupUi(this);

	m_colorCodes << "#F44336"
		     << "#2096F3"
		     << "#8BC34A"
		     << "#FF5721"
		     << "#607D8B";

	m_btnList = findChildren<QPushButton*>();

	int n = 0;
	for (const auto& i : m_btnList) {
		m_btnStates.push_back(false);
		i->setProperty("id", QVariant(n++));
		connect(i, SIGNAL(clicked()), this, SLOT(onButtonClicked()));
	}
}

DbClickButtons::~DbClickButtons() { delete m_ui; }

int DbClickButtons::selectedButton() const { return m_selectedBtn; }

void DbClickButtons::setSelectedButton(int btnId)
{
	for (int i = 0; i < m_btnList.size(); i++) {
		bool selected;
		if (i == btnId) {
			selected = true;
		} else {
			selected = false;
		}

		DynamicWidget::setDynamicProperty(m_btnList[i], "is_selected", selected);
		m_btnList[i]->update();

		if (selected) {
			m_selectedBtn = btnId;
			Q_EMIT buttonSelected(btnId);
		}
	}
}

bool DbClickButtons::buttonChecked(int btnId) const { return m_btnStates[btnId]; }

void DbClickButtons::setButtonChecked(int btnId, bool checked)
{
	m_btnStates[btnId] = checked;
	DynamicWidget::setDynamicProperty(m_btnList[btnId], "is_checked", checked);
	m_btnList[btnId]->update();

	Q_EMIT buttonToggled(btnId, checked);
}

int DbClickButtons::buttonCount() const { return m_btnList.size(); }

void DbClickButtons::setButtonCount(int count)
{
	if (count != m_btnList.size()) {
		for (int i = 0; i < m_btnList.size(); i++) {
			delete m_btnList[i];
		}
		m_selectedBtn = -1;
		m_btnList.clear();

		QGridLayout* gLayout = static_cast<QGridLayout*>(layout());
		for (int i = 0; i < count; i++) {
			QPushButton* btn = new QPushButton(this);
			btn->setText(QString::number(i));
			btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
			btn->setProperty("id", QVariant(i));
			btn->setProperty("is_checked", QVariant(false));
			btn->setProperty("is_selected", QVariant(false));
			QString color = m_colorCodes[i % m_colorCodes.size()];
			QString stylesheet = QString("QPushButton"
						     "{\n  border: 2px solid %1;\n}\n"
						     "\nQPushButton[is_checked=true] {\n  "
						     "background-color: %1;\n}\n"
						     "\nQPushButton[is_selected=true] {\n	"
						     "border: 4px solid white;\n}\n")
						     .arg(color);
			btn->setStyleSheet(stylesheet);
			connect(btn, SIGNAL(clicked()), this, SLOT(onButtonClicked()));

			int row = i / m_maxRowBtnCnt;
			int col = i % m_maxRowBtnCnt;
			gLayout->addWidget(btn, row, col);
			m_btnList.push_back(btn);
			m_btnStates.push_back(false);
		}
	}
}

void DbClickButtons::onButtonClicked()
{
	QPushButton* btn = static_cast<QPushButton*>(QObject::sender());
	int btnId = btn->property("id").toInt();

	if (m_selectedBtn < 0) {
		setSelectedButton(btnId);
	} else if (btn == m_btnList[m_selectedBtn]) {
		toggleButton(btnId);
	} else {
		setSelectedButton(btnId);
	}
}

void DbClickButtons::toggleButton(int btnId) { setButtonChecked(btnId, !m_btnStates[btnId]); }
