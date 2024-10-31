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

#include "browsemenu.h"
#include "style.h"

#include <QPushButton>
#include <stylehelper.h>

using namespace scopy;

BrowseMenu::BrowseMenu(QWidget *parent)
	: QWidget(parent)
	, m_collapsed(false)
{
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	QVBoxLayout *lay = new QVBoxLayout(this);
	lay->setMargin(0);
	lay->setSpacing(0);
	lay->setAlignment(Qt::AlignTop);
	setLayout(lay);

	QWidget *menuHeader = createHeader(this);

	m_content = new QWidget(this);
	m_content->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
	m_contentLay = new QVBoxLayout(m_content);
	m_contentLay->setMargin(0);
	m_contentLay->setSpacing(0);
	m_content->setLayout(m_contentLay);

	m_spacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Preferred);
	m_contentLay->addSpacerItem(m_spacer);

	m_toolMenu = new ToolMenu(m_content);
	QButtonGroup *btnGroup = m_toolMenu->btnGroup();
	m_toolMenu->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

	QPushButton *homeBtn = new QPushButton(tr("Home"), m_content);
	Style::setStyle(homeBtn, style::properties::button::toolButton);
	homeBtn->setIcon(
		QIcon(":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) + "/icons/tool_home.svg"));
	homeBtn->setFixedHeight(Style::getDimension(json::global::unit_4));
	homeBtn->setCheckable(true);
	homeBtn->setIconSize(QSize(32, 32));
	homeBtn->setStyleSheet("text-align: left");
	connect(homeBtn, &QPushButton::clicked, this, [=]() { Q_EMIT requestTool("home"); });

	QWidget *saveLoadWidget = new QWidget(m_content);
	saveLoadWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	saveLoadWidget->setLayout(new QHBoxLayout(saveLoadWidget));
	saveLoadWidget->layout()->setSpacing(0);
	saveLoadWidget->layout()->setMargin(0);

	QPushButton *saveBtn = createBtn(
		"Save", ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) + "/icons/save.svg",
		saveLoadWidget);
	saveBtn->setCheckable(false);
	connect(saveBtn, &QPushButton::clicked, this, &BrowseMenu::requestSave);

	QPushButton *loadBtn = createBtn(
		"Load", ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) + "/icons/load.svg",
		saveLoadWidget);
	loadBtn->setCheckable(true);
	connect(loadBtn, &QPushButton::clicked, this, &BrowseMenu::requestLoad);

	saveLoadWidget->layout()->addWidget(saveBtn);
	saveLoadWidget->layout()->addWidget(loadBtn);

	QPushButton *preferencesBtn = createBtn("Preferences",
						":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
							"/icons/preferences.svg",
						m_content);
	connect(preferencesBtn, &QPushButton::clicked, this, [=]() { Q_EMIT requestTool("preferences"); });
	preferencesBtn->setCheckable(true);

	QPushButton *aboutBtn = createBtn(
		"About", ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) + "/icons/info.svg",
		m_content);
	connect(aboutBtn, &QPushButton::clicked, this, [=]() { Q_EMIT requestTool("about"); });
	aboutBtn->setCheckable(true);

	QLabel *logo = createScopyLogo(m_content);

	btnGroup->addButton(homeBtn);
	btnGroup->addButton(preferencesBtn);
	btnGroup->addButton(aboutBtn);

	add(createHLine(m_content), "headerLine", MA_TOPLAST);
	add(homeBtn, "homeBtn", MA_TOPLAST);
	add(createHLine(m_content), "toolMenuLine1", MA_TOPLAST);
	add(m_toolMenu, "toolMenu", MA_TOPLAST);

	add(createHLine(m_content), "toolMenuLine2", MA_BOTTOMLAST);
	add(saveLoadWidget, "saveLoad", MA_BOTTOMLAST);
	add(preferencesBtn, "preferencesBtn", MA_BOTTOMLAST);
	add(aboutBtn, "aboutBtn", MA_BOTTOMLAST);
	add(logo, "logo", MA_BOTTOMLAST);

	lay->addWidget(menuHeader);
	lay->addWidget(m_content);
}

BrowseMenu::~BrowseMenu() {}

ToolMenu *BrowseMenu::toolMenu() const { return m_toolMenu; }

void BrowseMenu::add(QWidget *w, QString name, MenuAlignment position)
{
	int spacerIndex = m_contentLay->indexOf(m_spacer);
	switch(position) {

	case MA_TOPLAST:
		m_contentLay->insertWidget(spacerIndex, w);
		break;
	case MA_BOTTOMLAST:
		m_contentLay->insertWidget(-1, w);
		break;
	}
}

void BrowseMenu::toggleCollapsed()
{
	m_collapsed = !m_collapsed;
	m_btnCollapse->setHidden(m_collapsed);
	m_content->setHidden(m_collapsed);
	Q_EMIT collapsed(m_collapsed);
}

QPushButton *BrowseMenu::createBtn(QString name, QString iconPath, QWidget *parent)
{
	QPushButton *btn = new QPushButton(parent);
	btn->setIcon(QIcon(iconPath));
	btn->setText(tr(name.toStdString().c_str()));
	Style::setStyle(btn, style::properties::button::toolButton);
	return btn;
}

QFrame *BrowseMenu::createHLine(QWidget *parent)
{
	QFrame *line = new QFrame(parent);
	line->setFrameShape(QFrame::HLine);
	line->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	Style::setStyle(line, style::properties::widget::bottomBorder);
	return line;
}

QWidget *BrowseMenu::createHeader(QWidget *parent)
{
	QWidget *menuHeader = new QWidget(parent);
	menuHeader->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	QHBoxLayout *headerLay = new QHBoxLayout(menuHeader);
	headerLay->setSpacing(0);
	headerLay->setMargin(0);
	QPushButton *btnCollapseMini = new QPushButton(menuHeader);
	Style::setStyle(btnCollapseMini, style::properties::widget::toolMenu);
	Style::setStyle(btnCollapseMini, style::properties::button::toolButton);
	btnCollapseMini->setCheckable(true);
	btnCollapseMini->setFixedSize(Style::getDimension(json::global::unit_4),
				      Style::getDimension(json::global::unit_4));
	headerLay->addWidget(btnCollapseMini);

	m_btnCollapse = new QPushButton(menuHeader);
	Style::setStyle(m_btnCollapse, style::properties::widget::toolMenuLogo);
	Style::setStyle(m_btnCollapse, style::properties::widget::notInteractive);

	headerLay->addWidget(m_btnCollapse);
	connect(btnCollapseMini, &QPushButton::clicked, this, &BrowseMenu::toggleCollapsed);

	return menuHeader;
}

QLabel *BrowseMenu::createScopyLogo(QWidget *parent)
{
	QLabel *logo = new QLabel(m_content);
	Style::setStyle(logo, style::properties::widget::logo);
	return logo;
}

#include "moc_browsemenu.cpp"
