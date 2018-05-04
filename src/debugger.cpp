/*
 * Copyright 2017 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "debugger.h"
#include "ui_debugger.h"
#include <QDebug>
#include <QFileDialog>


using namespace adiscope;
using namespace std;

Debugger::Debugger(struct iio_context *ctx, Filter *filt,
                   QPushButton *runButton, QJSEngine *engine,
                   ToolLauncher *parent) :
	Tool(ctx, runButton, nullptr, "Debug", parent),
	ui(new Ui::Debugger), filter(filt),
	eng(engine)
{
	ui->setupUi(this);
	debug.setIioContext(ctx);

	reg = new RegisterWidget(ui->widget, &debug);

	ui->DevicecomboBox->addItems(debug.getDeviceList());

	/*Set register view widget to retain size policy*/
	QSizePolicy sp_retainSize = ui->widget->sizePolicy();
	sp_retainSize.setRetainSizeWhenHidden(true);
	ui->widget->setSizePolicy(sp_retainSize);

	this->updateChannelComboBox(ui->DevicecomboBox->currentText());
	this->updateAttributeComboBox(ui->ChannelComboBox->currentText());
	this->updateValueWidget(ui->AttributeComboBox->currentText());
	this->updateFilename(ui->AttributeComboBox->currentIndex());
	on_ReadButton_clicked();

	QObject::connect(ui->DevicecomboBox, &QComboBox::currentTextChanged,this,
	                 &Debugger::updateChannelComboBox);
	QObject::connect(ui->ChannelComboBox, &QComboBox::currentTextChanged, this,
	                 &Debugger::updateAttributeComboBox);
	QObject::connect(ui->AttributeComboBox, SIGNAL(currentIndexChanged(int)), this,
	                 SLOT(updateFilename(int)));
	QObject::connect(ui->AttributeComboBox, &QComboBox::currentTextChanged, this,
	                 &Debugger::updateValueWidget);
	QObject::connect(ui->DevicecomboBox, &QComboBox::currentTextChanged, this,
	                 &Debugger::updateSources);

	QObject::connect(ui->addressSpinBox, SIGNAL(valueChanged(int)), this,
	                 SLOT(updateRegMap()));

	QObject::connect(ui->valueSpinBox, SIGNAL(valueChanged(int)), this->reg,
	                 SLOT(setValue(int)));
	QObject::connect(this->reg, SIGNAL(valueChanged(int)), ui->valueSpinBox,
	                 SLOT(setValue(int)));

	QObject::connect(ui->DevicecomboBox, &QComboBox::currentTextChanged, this,
	                 &Debugger::updateRegMap);
	QObject::connect(ui->sourceComboBox, &QComboBox::currentTextChanged, this,
	                 &Debugger::updateRegMap);


	on_detailedRegMapCheckBox_stateChanged(0);
}

Debugger::~Debugger()
{

	delete ui;
}

void Debugger::updateChannelComboBox(QString devName)
{
	QStringList channels;

	ui->ChannelComboBox->blockSignals(true);
	ui->addressSpinBox->blockSignals(true);
	ui->ChannelComboBox->clear();
	ui->addressSpinBox->setValue(0);
	ui->ChannelComboBox->blockSignals(false);
	ui->addressSpinBox->blockSignals(false);

	debug.scanChannels(devName);
	channels = debug.getChannelList();

	if (channels.isEmpty()) {
		channels.append(QString("None"));
	}

	std::sort(channels.begin(), channels.end(), std::less_equal<QString>());
	ui->ChannelComboBox->addItems(channels);

	updateAttributeComboBox(ui->ChannelComboBox->currentText());
}

void Debugger::updateAttributeComboBox(QString channel)
{
	QStringList attributes;

	ui->AttributeComboBox->blockSignals(true);
	ui->AttributeComboBox->clear();
	ui->AttributeComboBox->blockSignals(false);

	debug.scanChannelAttributes(ui->DevicecomboBox->currentText(), channel);
	attributes = debug.getAttributeList();

	if (attributes.isEmpty()) {
		attributes.append(QString("None"));
	}

	ui->AttributeComboBox->addItems(attributes);

	updateFilename(ui->AttributeComboBox->currentIndex());
	updateValueWidget(ui->AttributeComboBox->currentText());
}

void Debugger::updateFilename(int index)
{
	QStringList filename = debug.getFileName();

	if (!filename.isEmpty()) {
		ui->filenameLineEdit->setText(filename[index]);
		//        ui->filenameLineEdit->setFixedWidth(700);
	} else {
		ui->filenameLineEdit->clear();
	}
}

void Debugger::updateValueWidget(QString attribute)
{
	bool available = false;
	QString dev, ch;
	QStringList valueList;

	QVector<QString> availableValues = debug.getAttributeVector();
	ui->valueStackedWidget->setEnabled(true);

	for (QString t : availableValues) {
		if (t.contains(attribute.toLatin1(), Qt::CaseInsensitive)) {
			available = true;

			/*read device, channel and get values to update widget*/
			dev = ui->DevicecomboBox->currentText();
			ch = ui->ChannelComboBox->currentText();
			valueList = debug.getAvailableValues(dev, ch, attribute);
			ui->valueComboBox->clear();
			ui->valueComboBox->addItems(valueList);
			ui->valueComboBox->setEnabled(true);
			ui->valueStackedWidget->setCurrentIndex(1);
			break;
		}
	}

	if (!available) {
		ui->valueStackedWidget->setCurrentIndex(0);
		ui->valueLineEdit->clear();
	}

	on_ReadButton_clicked();
}

void Debugger::on_ReadButton_clicked()
{
	QString dev;
	QString channel;
	QString attribute;
	QString value;
	int index;

	dev = ui->DevicecomboBox->currentText();
	channel = ui->ChannelComboBox->currentText();
	attribute = ui->AttributeComboBox->currentText();



	if (channel.contains("Global", Qt::CaseInsensitive)) {
		channel.clear();
	}

	if (!attribute.contains("None", Qt::CaseInsensitive)) {
		if (!attribute.isNull()) {
			value = debug.readAttribute(dev, channel, attribute);

			if (ui->valueStackedWidget->currentIndex() == 0) {
				ui->valueLineEdit->setText(value);
			} else {
				index = ui->valueComboBox->findText(value, Qt::MatchCaseSensitive);
				ui->valueComboBox->setCurrentIndex(index);
			}
		} else {
			ui->valueStackedWidget->setCurrentIndex(0);
		}
	}
}

void Debugger::on_WriteButton_clicked()
{
	QString dev;
	QString channel;
	QString attribute;
	QString value;

	dev = ui->DevicecomboBox->currentText();
	channel = ui->ChannelComboBox->currentText();
	attribute = ui->AttributeComboBox->currentText();

	if (channel.contains("Global", Qt::CaseInsensitive)) {
		channel.clear();
	}

	if (ui->valueStackedWidget->currentIndex() == 0) {
		value = ui->valueLineEdit->text();
	} else {
		value = ui->valueComboBox->currentText();
	}

	if (!attribute.isNull()) {
		debug.writeAttribute(dev, channel, attribute, value);
	}
}

void Debugger::updateSources()
{
	reg->verifyAvailableSources(ui->DevicecomboBox->currentText());
	QStringList list = reg->getSources();

	ui->sourceComboBox->blockSignals(true);
	ui->sourceComboBox->clear();
	ui->sourceComboBox->addItems(list);
	ui->sourceComboBox->blockSignals(false);

	Q_EMIT ui->sourceComboBox->currentTextChanged(
	        ui->sourceComboBox->currentText());
}

void Debugger::updateRegMap()
{
	QString device = ui->DevicecomboBox->currentText();
	QString source = ui->sourceComboBox->currentText();
	int address = ui->addressSpinBox->value();

	ui->addressSpinBox->blockSignals(true); //block signals from Address spinbox

	reg->createRegMap(&device, &address, &source);
	ui->addressSpinBox->setValue(address);

	if (!ui->sourceComboBox->currentText().isEmpty()) {
		ui->addressSpinBox->setMaximum(reg->getLastAddress());
	}

	if (ui->enableAutoReadCheckBox->isChecked()) {
		on_readRegPushButton_clicked();
	}

	ui->descriptionLineEdit->setText(reg->getDescription());
	ui->defaultValueLabel->setText(QString("0x%1").arg(reg->getDefaultValue(), 0,
	                               16));

	ui->addressSpinBox->blockSignals(false); //activate signals from Address spinbox
}

void Debugger::on_readRegPushButton_clicked()
{
	QString device = ui->DevicecomboBox->currentText();
	uint32_t value = 0;

	/*Read register*/
	ui->addressSpinBox->blockSignals(true); //block signals from Address spinbox
	value = reg->readRegister(&device, ui->addressSpinBox->value());

	ui->valueSpinBox->blockSignals(true);
	ui->valueSpinBox->setValue(value);
	ui->valueSpinBox->blockSignals(false);

	ui->valueHexLabel->setText(QString("0x%1").arg(value, 0, 16));
	ui->addressSpinBox->blockSignals(false); //activate signals from Address spinbox
}

void Debugger::on_writeRegPushButton_clicked()
{
	QString device = ui->DevicecomboBox->currentText();
	uint32_t address = ui->addressSpinBox->value();
	uint32_t value = ui->valueSpinBox->value();

	reg->writeRegister(&device, address, ui->valueSpinBox->value());

}

void adiscope::Debugger::on_detailedRegMapCheckBox_stateChanged(int arg1)
{
	if (!arg1) {
		ui->widget->hide();
	} else {
		ui->widget->show();
	}
}

void adiscope::Debugger::on_newWindowButton_clicked()
{
	Q_EMIT newDebuggerInstance();
}

void adiscope::Debugger::on_loadButton_clicked()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Save File"),
			   "/home",
			   tr("JavaScript (*.js)"));
	scriptFile.setFileName(fileName);
	ui->scriptLocation->setText(fileName);
}

void adiscope::Debugger::on_runButton_clicked()
{
	QJSValue result;

	if (!scriptFile.open(QIODevice::ReadOnly))
		qDebug() << "File not available";

	QTextStream stream(&scriptFile);
	QString contents = stream.readAll();
	scriptFile.close();

	QStringList l = contents.split('\n');

	if(l[0].contains('#'))
		l.removeFirst();
	contents = l.join('\n');

	result = eng->evaluate(contents, scriptFile.fileName());

	if (result.isError()) {
		qDebug()
				<< "Uncaught exception at line"
				<< result.property("lineNumber").toInt()
				<< ":" << result.toString();
	} else {
		qDebug()<<" - Success";
	}
}
