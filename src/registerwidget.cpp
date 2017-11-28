#include "registerwidget.h"
#include "ui_registerwidget.h"

using namespace adiscope;

RegisterWidget::RegisterWidget(QWidget *parent,  Debug  *debug) :
	QWidget(parent),
	ui(new Ui::RegisterWidget),
	address(0)
{
	ui->setupUi(this);

	regMap.setIioContext(debug->getIioContext());


	QObject::connect(this, &RegisterWidget::valueChanged, this,
	                 &RegisterWidget::updateBitfields);
}

RegisterWidget::~RegisterWidget()
{
	delete ui;
}

uint32_t RegisterWidget::getLastAddress(void) const
{
	return regMap.getLastAddress();
}

void RegisterWidget::createRegMap(const QString *device, int *address,
                                  const QString *source)
{
	QString filename;
	QString addr;
	bool goHigh = false;

	if (*address >= this->address) {
		goHigh = true;
	}

	for (auto iterator = bitfieldsVector.rbegin();
	     iterator != bitfieldsVector.rend(); ++iterator) {
		ui->horizontalLayout->removeWidget(*iterator);
		delete(*iterator);
	}

	ui->horizontalLayout->update();

	bitfieldsVector.clear();

	regMap.deviceXmlFileSelection(device, &filename, *source);
	regMap.deviceXmlFileLoad(&filename);

	if (!filename.isEmpty()) {
		if (regNode != 0) {
			regNode = nullptr;
		}

		while (regNode == 0) {
			addr = QString("0x%1").arg(*address, 0, 16);
			regNode = regMap.getRegisterNode(addr);

			if (regNode == 0) {
				if (goHigh) {
					(*address)++;
				} else {
					(*address)--;
				}

				addr = QString("0x%1").arg(*address, 0, 16);
			}
		}

		bool status;
		this->address =
		        regNode->firstChildElement("Address").text().split("0x")[1].toUInt(&status, 16);

		/*get register information from the node*/
		name = regNode->firstChildElement("Name").text();
		width = regNode->firstChildElement("Width").text().toInt();
		description = regNode->firstChildElement("Description").text();
		notes = regNode->firstChildElement("Notes").text();

		QDomElement bitfieldList = regNode->firstChildElement("BitFields");
		QDomElement bitfieldElement = bitfieldList.firstChildElement("BitField");

		while (bitfieldList.lastChildElement() != bitfieldElement) {
			bitfieldsVector.append(new BitfieldWidget(this, &bitfieldElement));
			bitfieldElement = bitfieldElement.nextSiblingElement();
		}

		if (!bitfieldElement.isNull()) {
			bitfieldsVector.append(new BitfieldWidget(this, &bitfieldElement));
		}

		checkRegisterMap();

		defaultValue = 0;

		for (auto iterator = bitfieldsVector.rbegin();
		     iterator != bitfieldsVector.rend(); ++iterator) {

			defaultValue += (*iterator)->getDefaultValue() << (*iterator)->getRegOffset();
			connect(*iterator, SIGNAL(valueChanged(uint32_t, uint32_t)), this,
			        SLOT(setValue(uint32_t,uint32_t)));
			ui->horizontalLayout->addWidget(*iterator);
		}
	}
}

bool lessThan(const BitfieldWidget *b1, const BitfieldWidget *b2)
{
	if (b1->getRegOffset() <= b2->getRegOffset()) {
		return true;
	} else {
		return false;
	}
}

void RegisterWidget::checkRegisterMap(void)
{
	int size = 0;
	int regOffsets[32];
	int slice[32];
	int before;
	int count = 0;
	int gap;
	int bit = 0;

	/*sort vector using bit number*/
	qSort(bitfieldsVector.begin(), bitfieldsVector.end(), lessThan);

	for (auto iterator = bitfieldsVector.begin(); iterator != bitfieldsVector.end();
	     ++iterator) {
		size += (*iterator)->getSliceWidth();
		slice[count] = (*iterator)->getSliceWidth();
		regOffsets[count] = (*iterator)->getRegOffset();
		count++;
	}

	bit = size;

	/*If bits are missing at the start of register*/
	before = regOffsets[0];

	if (before != 0) {
		for (int i = 0; i < before; i++) {
			bitfieldsVector.insert(i, new BitfieldWidget(this, i));
			size++;
		}

		size = 0;
		count = 0;

		for (auto iterator = bitfieldsVector.begin(); iterator != bitfieldsVector.end();
		     ++iterator) {
			size += (*iterator)->getSliceWidth();
			slice[count] = (*iterator)->getSliceWidth();
			regOffsets[count] = (*iterator)->getRegOffset();
			count++;
		}

		bit = size;
	}

	if (size < width) {
		/*if bits are missing inside the register*/
		before = regOffsets[0];

		for (int i = 1; i < (count); ++i) {
			gap = (regOffsets[i] - before) - slice[i - 1];

			for (int j = 0; j < gap; j++) {
				bitfieldsVector.insert(i + j , new BitfieldWidget(this,
				                       i + j + slice[i - 1] - 1));
				size++;
			}

			before = regOffsets[i];
		}

		/*if bits are missing at the end of the register*/
		size = 0;
		count = 0;

		for (auto iterator = bitfieldsVector.begin(); iterator != bitfieldsVector.end();
		     ++iterator) {
			size += (*iterator)->getSliceWidth();
			slice[count] = (*iterator)->getSliceWidth();
			regOffsets[count] = (*iterator)->getRegOffset();
			count++;
		}

		bit = size;

		while (size < width) {
			bitfieldsVector.insert(count, new BitfieldWidget(this, bit++));
			count++;
			size++;
		}
	}
}

void RegisterWidget::verifyAvailableSources(const QString device)
{
	QString filename;

	fileSources.clear(); //clear the fileSource list

	regMap.deviceXmlFileSelection(&device, &filename, QString("SPI"));

	if (!filename.isNull()) {
		fileSources.append(QString("SPI"));
	}

	regMap.deviceXmlFileSelection(&device, &filename, QString("AXI CORE"));

	if (!filename.isNull()) {
		fileSources.append(QString("AXI CORE"));
	}
}

QStringList RegisterWidget::getSources() const
{
	return fileSources;
}

uint32_t RegisterWidget::getValue() const
{
	return value;
}

void RegisterWidget::setValue(int var)
{
	this->blockSignals(true);
	value = var;
	updateBitfields();
	this->blockSignals(false);
}

void RegisterWidget::setValue(uint32_t var, uint32_t mask)
{
	value &= ~mask;
	value |=var;

	Q_EMIT valueChanged((int)value);
}

uint32_t RegisterWidget::readRegister(const QString *device,
                                      const uint32_t address)
{
	/*Read register*/
	value = regMap.readRegister(device, address);

	updateBitfields();

	return value;
}

void RegisterWidget::writeRegister(const QString *device,
                                   const uint32_t address, uint32_t regVal)
{
	regMap.writeRegister(device, address, regVal);
	value = regVal;
}

void RegisterWidget::updateBitfields()
{
	uint32_t temp = value;

	/*Update bitfield widgets*/
	for (auto iterator = bitfieldsVector.begin(); iterator != bitfieldsVector.end();
	     ++iterator) {
		(*iterator)->updateValue(&temp);
	}
}

QString RegisterWidget::getDescription() const
{
	return description;
}


uint32_t RegisterWidget::getDefaultValue() const
{
	return defaultValue;
}
