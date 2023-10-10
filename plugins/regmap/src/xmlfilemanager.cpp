#include "xmlfilemanager.hpp"

#include "jsonformatedelement.hpp"
#include "register/bitfield/bitfieldmodel.hpp"
#include "register/registermodel.hpp"
#include "utils.hpp"

#include <QDomDocument>
#include <QHBoxLayout>
#include <qdebug.h>
#include <qfile.h>

#include <register/bitfield/bitfieldoption.hpp>

using namespace scopy;
using namespace regmap;

XmlFileManager::XmlFileManager(struct iio_device *dev, QString filePath)
	: dev(dev)
	, filePath(filePath)
{}

QList<QString> *XmlFileManager::getAllAddresses()
{
	QList<QString> *addressList = new QList<QString>();
	QDomDocument xmlBOM;
	QFile f(filePath);
	if(!f.open(QIODevice::ReadOnly)) {
		// Error while loading file
		qDebug() << "Error while loading file";
	}
	// Set data into the QDomDocument before processing
	xmlBOM.setContent(&f);
	QDomElement root = xmlBOM.documentElement();
	QDomElement reg = root.firstChildElement("Register");
	while(!reg.isNull()) {
		QDomElement address = reg.firstChildElement("Address");
		addressList->push_back(address.firstChild().toText().data());
		reg = reg.nextSibling().toElement();
	}
	f.close();

	return addressList;
}

QMap<uint32_t, RegisterModel *> *XmlFileManager::getAllRegisters()
{
	QMap<uint32_t, RegisterModel *> *regList = new QMap<uint32_t, RegisterModel *>();
	QDomDocument xmlBOM;
	QFile f(filePath);
	if(!f.open(QIODevice::ReadOnly)) {
		// Error while loading file
		qDebug() << "Error while loading file";
	}
	// Set data into the QDomDocument before processing
	xmlBOM.setContent(&f);
	QDomElement root = xmlBOM.documentElement();
	QDomElement reg = root.firstChildElement("Register");

	while(!reg.isNull()) {
		bool ok;
		int address = reg.firstChildElement("Address").firstChild().toText().data().toInt(&ok, 16);
		regList->insert(address, getRegister(reg));
		reg = reg.nextSibling().toElement();
	}
	f.close();

	return regList;
}

QVector<BitFieldModel *> *XmlFileManager::getBitFields(QString regAddr)
{
	QDomDocument xmlBOM;
	QFile f(filePath);
	if(!f.open(QIODevice::ReadOnly)) {
		// Error while loading file
		qDebug() << "Error while loading file";
	}
	// Set data into the QDomDocument before processing
	xmlBOM.setContent(&f);
	QDomElement root = xmlBOM.documentElement();
	QDomElement reg = root.firstChildElement("Register");
	while(!reg.isNull()) {
		QDomElement address = reg.firstChildElement("Address");
		if(address.firstChild().toText().data() == regAddr) {
			return getBitFieldsOfRegister(reg, true);
		}
		reg = reg.nextSibling().toElement();
	}
	f.close();

	return nullptr;
}

RegisterModel *XmlFileManager::getRegister(QDomElement reg)
{
	if(!reg.isNull()) {
		bool ok;

		QString name = "";
		QString description = "";
		bool isAxiCompatible = false;
		QStringList xmlName = filePath.split(QLatin1Char('/'));

		JsonFormatedElement *jsonFormatedElement =
			scopy::regmap::Utils::getJsonTemplate(xmlName.at(xmlName.length() - 1));
		if(jsonFormatedElement && jsonFormatedElement->getUseRegisterNameAsDescription()) {
			name = reg.firstChildElement("Description").firstChild().toText().data();
			description = reg.firstChildElement("Name").firstChild().toText().data();
			isAxiCompatible = jsonFormatedElement->getIsAxiCompatible();
		} else {
			name = reg.firstChildElement("Name").firstChild().toText().data();
			description = reg.firstChildElement("Description").firstChild().toText().data();
		}

		return new RegisterModel(
			name, reg.firstChildElement("Address").firstChild().toText().data().toInt(&ok, 16), description,
			reg.firstChildElement("Exists").toText().data().toUpper() == "TRUE",
			reg.firstChildElement("Width").firstChild().toText().data().toInt(),
			reg.firstChildElement("Notes").firstChild().toText().data(),
			getBitFieldsOfRegister(reg, isAxiCompatible));
	}
	return nullptr;
}

bool compareRegisters(const BitFieldModel *bf1, const BitFieldModel *bf2)
{
	return bf1->getRegOffset() < bf2->getRegOffset();
}

QVector<BitFieldModel *> *XmlFileManager::getBitFieldsOfRegister(QDomElement reg, bool reverseBitOrder)
{
	if(!reg.isNull()) {

		QVector<BitFieldModel *> *bitFieldsList = new QVector<BitFieldModel *>();
		QDomElement bitFields = reg.firstChildElement("BitFields");
		QDomElement bf;
		bf = bitFields.firstChild().toElement();
		int regWidth = reg.firstChildElement("Width").firstChild().toText().data().toInt();
		int numberOfBits = 0;
		while(!bf.isNull()) {
			BitFieldModel *bitField = getBitField(bf);
			bitFieldsList->push_back(bitField);
			numberOfBits += bitField->getWidth();
			bf = bf.nextSibling().toElement();
		}
		// after getting all available bitfields sort the list and add all the missing bits as reserved
		if(!bitFieldsList->empty()) {
			QVector<BitFieldModel *> *bitFieldsListWithReserved = new QVector<BitFieldModel *>();
			if(regWidth > numberOfBits) {
				// sort bitfield list
				std::sort(bitFieldsList->begin(), bitFieldsList->end(), compareRegisters);
				bitFieldsListWithReserved->push_back(bitFieldsList->at(0));
				for(int i = 1; i < bitFieldsList->length(); i++) {
					int bf1 = bitFieldsList->at(i - 1)->getRegOffset() +
						bitFieldsList->at(i - 1)->getWidth();
					int bf2 = bitFieldsList->at(i)->getRegOffset();
					if(bf1 != bf2) {
						BitFieldModel *bitField = new BitFieldModel(
							"Bit " + QString::number(bf2 - 1) + ":" + QString::number(bf1),
							bf2 - bf1, bf1, "Reserved", nullptr);
						bitFieldsListWithReserved->push_back(bitField);
						numberOfBits += bf2 - bf1;
					}
					bitFieldsListWithReserved->push_back(bitFieldsList->at(i));
				}

				if(regWidth > numberOfBits) {
					BitFieldModel *bitField = new BitFieldModel(
						"Bit " + QString::number(regWidth - 1) + ":" +
							QString::number(numberOfBits),
						regWidth - numberOfBits, numberOfBits, "Reserved", nullptr);
					bitFieldsListWithReserved->push_back(bitField);
				}
				return bitFieldsListWithReserved;
			}
		}
		return bitFieldsList;
	}
	return nullptr;
}

BitFieldModel *XmlFileManager::getBitField(QDomElement bitField)
{
	if(!bitField.isNull()) {
		QVector<BitFieldOption *> *options = new QVector<BitFieldOption *>();
		QDomElement bitFieldOptions = bitField.firstChildElement("Options");
		QDomElement bfOption = bitFieldOptions.firstChildElement("Option");
		while(!bfOption.isNull()) {
			bool ok;
			options->push_back(new BitFieldOption(
				QString::number(bfOption.firstChildElement("Value").firstChild().toText().data().toInt(
					&ok, 16)),
				bfOption.firstChildElement("Description").firstChild().toText().data()));
			bfOption = bfOption.nextSibling().toElement();
		}

		QString name = "";
		QString description = "";
		QStringList xmlName = filePath.split(QLatin1Char('/'));

		JsonFormatedElement *jsonFormatedElement =
			scopy::regmap::Utils::getJsonTemplate(xmlName.at(xmlName.length() - 1));
		if(jsonFormatedElement && jsonFormatedElement->getUseBifieldNameAsDescription()) {
			name = bitField.firstChildElement("Description").firstChild().toText().data();
			description = bitField.firstChildElement("Name").firstChild().toText().data();
		} else {
			name = bitField.firstChildElement("Name").firstChild().toText().data();
			description = bitField.firstChildElement("Description").firstChild().toText().data();
		}

		return new BitFieldModel(
			name, bitField.firstChildElement("Access").firstChild().toText().data(),
			bitField.firstChildElement("DefaultValue").firstChild().toText().data().toInt(), description,
			bitField.firstChildElement("Visibility").firstChild().toText().data(),
			bitField.firstChildElement("Width").firstChild().toText().data().toInt(),
			bitField.firstChildElement("Notes").firstChild().toText().data(),
			bitField.firstChildElement("BitOffset").firstChild().toText().data().toInt(),
			bitField.firstChildElement("RegOffset").firstChild().toText().data().toInt(),
			bitField.firstChildElement("SliceWidth").firstChild().toText().data().toInt(), options);
	}
	return nullptr;
}
