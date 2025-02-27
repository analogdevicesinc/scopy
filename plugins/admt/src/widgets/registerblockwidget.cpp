/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#include "widgets/registerblockwidget.h"
#include "style_properties.h"
#include <style.h>
#include <stylehelper.h>

using namespace scopy;
using namespace scopy::admt;

RegisterBlockWidget::RegisterBlockWidget(
    QString header, QString description, uint32_t address, uint32_t cnvPage,
    RegisterBlockWidget::ACCESS_PERMISSION accessPermission, QWidget *parent)
    : QWidget(parent), m_address(address), m_cnvPage(cnvPage),
      m_accessPermission(accessPermission) {
  QVBoxLayout *container = new QVBoxLayout(this);
  setLayout(container);
  container->setMargin(0);
  container->setSpacing(0);
  MenuSectionWidget *menuSectionWidget = new MenuSectionWidget(this);
  Style::setStyle(menuSectionWidget, style::properties::widget::basicComponent);
  QLabel *headerLabel = new QLabel(header, menuSectionWidget);
  Style::setStyle(headerLabel, style::properties::label::menuMedium);
  menuSectionWidget->setFixedHeight(180);
  menuSectionWidget->contentLayout()->setSpacing(
      Style::getDimension(json::global::unit_0_5));

  QLabel *descriptionLabel = new QLabel(description, menuSectionWidget);
  descriptionLabel->setWordWrap(true);
  descriptionLabel->setMinimumHeight(24);
  descriptionLabel->setAlignment(Qt::AlignTop);
  descriptionLabel->setSizePolicy(QSizePolicy::Preferred,
                                  QSizePolicy::MinimumExpanding);

  m_spinBox = new PaddedSpinBox(menuSectionWidget);
  Style::setStyle(m_spinBox, style::properties::admt::spinBox);
  m_spinBox->setButtonSymbols(m_spinBox->ButtonSymbols::NoButtons);

  m_value = 0x00;
  m_spinBox->setValue(m_value);

  QWidget *buttonsWidget = new QWidget(menuSectionWidget);
  QHBoxLayout *buttonsContainer = new QHBoxLayout(buttonsWidget);
  buttonsWidget->setLayout(buttonsContainer);

  buttonsContainer->setMargin(0);
  buttonsContainer->setSpacing(Style::getDimension(json::global::unit_0_5));
  switch (m_accessPermission) {
  case ACCESS_PERMISSION::READWRITE:
    addReadButton(buttonsWidget);
    addWriteButton(buttonsWidget);
    break;
  case ACCESS_PERMISSION::WRITE:
    addWriteButton(buttonsWidget);
    break;
  case ACCESS_PERMISSION::READ:
    addReadButton(buttonsWidget);
    m_spinBox->setReadOnly(true);
    break;
  }

  menuSectionWidget->contentLayout()->addWidget(headerLabel);
  menuSectionWidget->contentLayout()->addWidget(descriptionLabel);
  menuSectionWidget->contentLayout()->addWidget(m_spinBox);
  menuSectionWidget->contentLayout()->addWidget(buttonsWidget);

  container->addWidget(menuSectionWidget);
  container->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding,
                                           QSizePolicy::Preferred));

  connect(m_spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this,
          &RegisterBlockWidget::onValueChanged);
}

RegisterBlockWidget::~RegisterBlockWidget() {}

void RegisterBlockWidget::onValueChanged(int newValue) {
  m_value = static_cast<uint32_t>(newValue);
}

uint32_t RegisterBlockWidget::getValue() { return m_value; }

void RegisterBlockWidget::setValue(uint32_t value) {
  m_value = value;
  m_spinBox->setValue(m_value);
}

uint32_t RegisterBlockWidget::getAddress() { return m_address; }

uint32_t RegisterBlockWidget::getCnvPage() { return m_cnvPage; }

RegisterBlockWidget::ACCESS_PERMISSION
RegisterBlockWidget::getAccessPermission() {
  return m_accessPermission;
}

void RegisterBlockWidget::addReadButton(QWidget *parent) {
  m_readButton = new QPushButton("Read", parent);
  Style::setStyle(m_readButton, style::properties::button::basicButton);
  parent->layout()->addWidget(m_readButton);
}

QPushButton *RegisterBlockWidget::readButton() { return m_readButton; }

void RegisterBlockWidget::addWriteButton(QWidget *parent) {
  m_writeButton = new QPushButton("Write", parent);
  Style::setStyle(m_writeButton, style::properties::button::basicButton);
  parent->layout()->addWidget(m_writeButton);
}

QPushButton *RegisterBlockWidget::writeButton() { return m_writeButton; }

PaddedSpinBox::PaddedSpinBox(QWidget *parent) : QSpinBox(parent) {
  setDisplayIntegerBase(16);
  setMinimum(0);
  setMaximum(INT_MAX);
}

PaddedSpinBox::~PaddedSpinBox() {}

QString PaddedSpinBox::textFromValue(int value) const {
  return QString("0x%1").arg(value, 4, 16, QChar('0'));
}
#include "moc_registerblockwidget.cpp"
