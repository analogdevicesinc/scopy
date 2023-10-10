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

#include "debuggerinstrument.h"

#include "ui_debuggerinstrument.h"

#include <QDebug>
#include <QFileDialog>
#include <QJSEngine>

#include <gui/tool_view_builder.hpp>
#include <gui/utils.h>

using namespace scopy;
using namespace scopy::debugger;
using namespace std;

DebuggerInstrument::DebuggerInstrument(struct iio_context *ctx, QJSEngine *engine, QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::DebuggerInstrument)
	, eng(engine)
	, m_toolView(nullptr)
{
	ui->setupUi(this);
	m_debugController.setIioContext(ctx);

	reg = new RegisterWidget(ui->widget, &m_debugController);

	ui->DevicecomboBox->addItems(m_debugController.getDeviceList());

	/*Set register view widget to retain size policy*/
	QSizePolicy sp_retainSize = ui->widget->sizePolicy();
	sp_retainSize.setRetainSizeWhenHidden(true);
	ui->widget->setSizePolicy(sp_retainSize);

	this->updateChannelComboBox(ui->DevicecomboBox->currentText());
	this->updateAttributeComboBox(ui->ChannelComboBox->currentText());
	this->updateValueWidget(ui->AttributeComboBox->currentText());
	this->updateFilename(ui->AttributeComboBox->currentIndex());
	on_ReadButton_clicked();

	QObject::connect(ui->DevicecomboBox, &QComboBox::currentTextChanged, this,
			 &DebuggerInstrument::updateChannelComboBox);
	QObject::connect(ui->ChannelComboBox, &QComboBox::currentTextChanged, this,
			 &DebuggerInstrument::updateAttributeComboBox);
	QObject::connect(ui->AttributeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateFilename(int)));
	QObject::connect(ui->AttributeComboBox, &QComboBox::currentTextChanged, this,
			 &DebuggerInstrument::updateValueWidget);
	QObject::connect(ui->DevicecomboBox, &QComboBox::currentTextChanged, this, &DebuggerInstrument::updateSources);

	QObject::connect(ui->addressSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateRegMap()));

	QObject::connect(ui->valueRegisterSpinBox, SIGNAL(valueChanged(int)), this->reg, SLOT(setValue(int)));
	QObject::connect(this->reg, SIGNAL(valueChanged(int)), ui->valueRegisterSpinBox, SLOT(setValue(int)));

	QObject::connect(ui->DevicecomboBox, &QComboBox::currentTextChanged, this, &DebuggerInstrument::updateRegMap);
	QObject::connect(ui->sourceComboBox, &QComboBox::currentTextChanged, this, &DebuggerInstrument::updateRegMap);

	on_detailedRegMapCheckBox_stateChanged(0);
}

DebuggerInstrument::~DebuggerInstrument() { delete ui; }

void DebuggerInstrument::updateChannelComboBox(QString devName)
{
	QStringList channels;

	ui->ChannelComboBox->blockSignals(true);
	ui->addressSpinBox->blockSignals(true);
	ui->ChannelComboBox->clear();
	ui->addressSpinBox->setValue(0);
	ui->ChannelComboBox->blockSignals(false);
	ui->addressSpinBox->blockSignals(false);

	m_debugController.scanChannels(devName);
	channels = m_debugController.getChannelList();

	if(channels.isEmpty()) {
		channels.append(QString("None"));
	}

	std::sort(channels.begin(), channels.end(),
		  [](QString a, QString b) { return Util::compareNatural(a.toStdString(), b.toStdString()); });

	ui->ChannelComboBox->addItems(channels);

	updateAttributeComboBox(ui->ChannelComboBox->currentText());
}

void DebuggerInstrument::updateAttributeComboBox(QString channel)
{
	QStringList attributes;

	ui->AttributeComboBox->blockSignals(true);
	ui->AttributeComboBox->clear();
	ui->AttributeComboBox->blockSignals(false);

	m_debugController.scanChannelAttributes(ui->DevicecomboBox->currentText(), channel);
	attributes = m_debugController.getAttributeList();

	if(attributes.isEmpty()) {
		attributes.append(QString("None"));
	}

	ui->AttributeComboBox->addItems(attributes);

	updateFilename(ui->AttributeComboBox->currentIndex());
	updateValueWidget(ui->AttributeComboBox->currentText());
}

void DebuggerInstrument::updateFilename(int index)
{
	QStringList filename = m_debugController.getFileName();

	if(!filename.isEmpty()) {
		ui->filenameLineEdit->setText(filename[index]);
		//        ui->filenameLineEdit->setFixedWidth(700);
	} else {
		ui->filenameLineEdit->clear();
	}
}

void DebuggerInstrument::updateValueWidget(QString attribute)
{
	bool available = false;
	QString dev, ch;
	QStringList valueList;

	QVector<QString> availableValues = m_debugController.getAttributeVector();
	ui->valueStackedWidget->setEnabled(true);

	for(const QString &t : qAsConst(availableValues)) {
		if(!QString::compare(t, attribute, Qt::CaseInsensitive)) {
			available = true;

			/*read device, channel and get values to update widget*/
			dev = ui->DevicecomboBox->currentText();
			ch = ui->ChannelComboBox->currentText();
			valueList = m_debugController.getAvailableValues(dev, ch, attribute);

			/* Don't treat value range [min step max] as combobox items */
			if(valueList.at(0).startsWith("[")) {
				if(valueList.size() == 3) {
					QString min = valueList.at(0);
					min.remove(0, 1);
					min.squeeze();

					QString step = valueList.at(1);
					QString max = valueList.at(2);
					if(max.endsWith("]")) {
						max.chop(1);
					}
					ui->valueSpinBox->clear();
					ui->valueSpinBox->setRange(min.toDouble(), max.toDouble());
					ui->valueSpinBox->setEnabled(true);
					ui->valueStackedWidget->setCurrentWidget(ui->pageSpinBox);
				} else {
					// error in range structure
					break;
				}
			} else {
				ui->valueComboBox->clear();
				ui->valueComboBox->addItems(valueList);
				ui->valueComboBox->setEnabled(true);
				ui->valueStackedWidget->setCurrentWidget(ui->pageComboBox);
			}
			break;
		}
	}

	if(!available) {
		ui->valueStackedWidget->setCurrentWidget(ui->pageLineEdit);
		ui->valueLineEdit->clear();
	}

	on_ReadButton_clicked();
}

void DebuggerInstrument::updateReadValue(QString value)
{
	auto currentPageWidget = ui->valueStackedWidget->currentWidget();
	if(currentPageWidget == ui->pageLineEdit) {
		ui->valueLineEdit->setText(value);
	} else if(currentPageWidget == ui->pageComboBox) {
		int index = ui->valueComboBox->findText(value, Qt::MatchCaseSensitive);
		ui->valueComboBox->setCurrentIndex(index);
	} else if(currentPageWidget == ui->pageSpinBox) {
		QStringList valueAndUnit = value.split(" ");
		if(valueAndUnit.size() > 1) {
			ui->valueSpinBox->setSuffix(" " + valueAndUnit.at(1));
			value = valueAndUnit.at(0);
		} else {
			ui->valueSpinBox->setSuffix("");
		}
		ui->valueSpinBox->setValue(value.toDouble());
	}
}

void DebuggerInstrument::on_ReadButton_clicked()
{
	QString dev;
	QString channel;
	QString attribute;
	QString value;
	int index;

	dev = ui->DevicecomboBox->currentText();
	channel = ui->ChannelComboBox->currentText();
	attribute = ui->AttributeComboBox->currentText();

	if(channel.contains("Global", Qt::CaseInsensitive)) {
		channel.clear();
	}

	if(!attribute.contains("None", Qt::CaseInsensitive)) {
		if(!attribute.isNull()) {
			value = m_debugController.readAttribute(dev, channel, attribute);
			updateReadValue(value);
		} else {
			ui->valueStackedWidget->setCurrentWidget(ui->pageLineEdit);
		}
	}
}

void DebuggerInstrument::on_WriteButton_clicked()
{
	QString dev;
	QString channel;
	QString attribute;
	QString value;
	QString feedbackValue;

	dev = ui->DevicecomboBox->currentText();
	channel = ui->ChannelComboBox->currentText();
	attribute = ui->AttributeComboBox->currentText();

	if(channel.contains("Global", Qt::CaseInsensitive)) {
		channel.clear();
	}

	auto currentPageWidget = ui->valueStackedWidget->currentWidget();
	if(currentPageWidget == ui->pageLineEdit) {
		value = ui->valueLineEdit->text();
	} else if(currentPageWidget == ui->pageComboBox) {
		value = ui->valueComboBox->currentText();
	} else if(currentPageWidget == ui->pageSpinBox) {
		value = ui->valueSpinBox->text();
		QStringList valueAndUnit = value.split(' ');
		if(valueAndUnit.size() > 1) {
			value = valueAndUnit.at(1);
		}
		ui->valueSpinBox->setValue(value.toDouble());
	}

	if(!attribute.isNull()) {
		feedbackValue = m_debugController.writeAttribute(dev, channel, attribute, value);
		updateReadValue(feedbackValue);
	}
}

void DebuggerInstrument::updateSources()
{
	reg->verifyAvailableSources(ui->DevicecomboBox->currentText());
	QStringList list = reg->getSources();

	ui->sourceComboBox->blockSignals(true);
	ui->sourceComboBox->clear();
	ui->sourceComboBox->addItems(list);
	ui->sourceComboBox->blockSignals(false);

	Q_EMIT ui->sourceComboBox->currentTextChanged(ui->sourceComboBox->currentText());
}

void DebuggerInstrument::updateRegMap()
{
	QString device = ui->DevicecomboBox->currentText();
	QString source = ui->sourceComboBox->currentText();
	int address = ui->addressSpinBox->value();

	ui->addressSpinBox->blockSignals(true); // block signals from Address spinbox

	reg->createRegMap(&device, &address, &source);
	ui->addressSpinBox->setValue(address);

	if(!ui->sourceComboBox->currentText().isEmpty()) {
		ui->addressSpinBox->setMaximum(reg->getLastAddress());
	}

	if(ui->enableAutoReadCheckBox->isChecked()) {
		on_readRegPushButton_clicked();
	}

	ui->descriptionLineEdit->setText(reg->getDescription());
	ui->defaultValueLabel->setText(QString("0x%1").arg(reg->getDefaultValue(), 0, 16));

	ui->addressSpinBox->blockSignals(false); // activate signals from Address spinbox
}

void DebuggerInstrument::on_readRegPushButton_clicked()
{
	QString device = ui->DevicecomboBox->currentText();
	uint32_t value = 0;

	/*Read register*/
	ui->addressSpinBox->blockSignals(true); // block signals from Address spinbox
	value = reg->readRegister(&device, ui->addressSpinBox->value());

	ui->valueRegisterSpinBox->blockSignals(true);
	ui->valueRegisterSpinBox->setValue(value);
	ui->valueRegisterSpinBox->blockSignals(false);

	ui->valueHexLabel->setText(QString("0x%1").arg(value, 0, 16));
	ui->addressSpinBox->blockSignals(false); // activate signals from Address spinbox
}

void DebuggerInstrument::on_writeRegPushButton_clicked()
{
	QString device = ui->DevicecomboBox->currentText();
	uint32_t address = ui->addressSpinBox->value();

	reg->writeRegister(&device, address, ui->valueRegisterSpinBox->value());
}

void DebuggerInstrument::on_detailedRegMapCheckBox_stateChanged(int arg1)
{
	if(!arg1) {
		ui->widget->hide();
	} else {
		ui->widget->show();
	}
}
