/*
 * Copyright (c) 2023 Analog Devices Inc.
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
 */


#include "buffermenuview.h"
#include <QHBoxLayout>

using namespace scopy;

swiot::BufferMenuView::BufferMenuView(QWidget *parent):
	GenericMenu(parent)
      ,m_swiotAdvMenu(nullptr)
{}

swiot::BufferMenuView::~BufferMenuView()
{}

void swiot::BufferMenuView::init(QString title, QString function, QColor* color)
{
	initInteractiveMenu();
	setMenuHeader(title,color,false);
	createHeaderWidget(title);

	m_advanceSettingsSection = new gui::SubsectionSeparator("SETTINGS", false);
	m_advanceSettingsSection->getLabel()->setStyleSheet("color:gray;");
	m_advanceSettingsSection->layout()->setSpacing(10);
	m_advanceSettingsSection->getContentWidget()->layout()->setSpacing(10);

	insertSection(m_advanceSettingsSection);
	m_swiotAdvMenu = swiot::BufferMenuBuilder::newAdvMenu(m_advanceSettingsSection->getContentWidget(), function);
	QVector<QBoxLayout *> layers = m_swiotAdvMenu->getMenuLayers();
	for (int i = 0; i < layers.size(); i++) {
		m_advanceSettingsSection->getContentWidget()->layout()->addItem(layers[i]);
	}

}

void swiot::BufferMenuView::initAdvMenu(QMap<QString, QMap<QString, QStringList>> values)
{
	m_swiotAdvMenu->setAttrValues(values);
	m_swiotAdvMenu->init();
}

void swiot::BufferMenuView::createHeaderWidget(const QString title)
{
	QWidget *headerWidget = new QWidget();
	headerWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
	headerLayout->setContentsMargins(0,0,0,0);
	headerLayout->setSpacing(0);
	m_btnInfoStatus = new scopy::LinkedButton();
	m_btnInfoStatus->installEventFilter(this);
	m_btnInfoStatus->setObjectName(QString::fromUtf8("btnHelp"));
	m_btnInfoStatus->setCheckable(false);
	m_btnInfoStatus->setText(QString());
	m_btnInfoStatus->setToolTip("For each output channel, a buffered\n"
				  "input channel is generated which is\n"
				  "displayed on the plot");
	headerLayout->insertWidget(headerLayout->count(), new QLabel(title));
	headerLayout->insertWidget(headerLayout->count(), m_btnInfoStatus);
	addNewHeaderWidget(headerWidget);
}

bool swiot::BufferMenuView::eventFilter(QObject *obj, QEvent *event)
{
	if (obj == (QObject*)m_btnInfoStatus) {
		if (event->type() == QEvent::Enter)
		{
			m_btnInfoStatus->setToolTip(m_swiotAdvMenu->getInfoMessage());
		}
		return false;
	} else {
		return QWidget::eventFilter(obj, event);
	}
}

swiot::BufferMenu* swiot::BufferMenuView::getAdvMenu()
{
	return m_swiotAdvMenu;
}
