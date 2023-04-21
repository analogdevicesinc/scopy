#include "register/bitfield/bitfieldmodel.hpp"
#include "register/registermodel.hpp"
#include "xmlfilemanager.hpp"

#include <QDomDocument>
#include <qfile.h>
#include <qdebug.h>
#include <iio.h>
#include <QHBoxLayout>

XmlFileManager::XmlFileManager(struct iio_device *dev, QString filePath):
	dev(dev),
	filePath(filePath)
{}

QList<QString> *XmlFileManager::getAllAddresses()
{
	QList<QString> *addressList = new QList<QString>();
	QDomDocument xmlBOM;
	QFile f(filePath);
	if (!f.open(QIODevice::ReadOnly ))
	{
		// Error while loading file
		qDebug() << "Error while loading file";
	}
	// Set data into the QDomDocument before processing
	xmlBOM.setContent(&f);
	QDomElement root=xmlBOM.documentElement();
	QDomElement reg = root.firstChildElement("Register");
	while(!reg.isNull())
	{
		QDomElement address = reg.firstChildElement("Address");
		addressList->push_back(address.firstChild().toText().data());
		reg = reg.nextSibling().toElement();
	}
	f.close();

	return addressList;
}

QMap<uint32_t, RegisterModel*>* XmlFileManager::getAllRegisters()
{
	QMap<uint32_t, RegisterModel*> *regList = new QMap<uint32_t, RegisterModel*>();
	QDomDocument xmlBOM;
	QFile f(filePath);
	if (!f.open(QIODevice::ReadOnly ))
	{
		// Error while loading file
		qDebug() << "Error while loading file";
	}
	// Set data into the QDomDocument before processing
	xmlBOM.setContent(&f);
	QDomElement root=xmlBOM.documentElement();
	QDomElement reg = root.firstChildElement("Register");

	while(!reg.isNull())
	{
		bool ok;
		int address = reg.firstChildElement("Address").firstChild().toText().data().toInt(&ok,16);
		regList->insert(address, getRegister(reg));
		reg = reg.nextSibling().toElement();
	}
	f.close();

	return regList;
}

QVector<BitFieldModel*> *XmlFileManager::getBitFields(QString regAddr)
{
	QDomDocument xmlBOM;
	QFile f(filePath);
	if (!f.open(QIODevice::ReadOnly ))
	{
		// Error while loading file
		qDebug() << "Error while loading file";
	}
	// Set data into the QDomDocument before processing
	xmlBOM.setContent(&f);
	QDomElement root=xmlBOM.documentElement();
	QDomElement reg = root.firstChildElement("Register");
	while(!reg.isNull())
	{
		QDomElement address = reg.firstChildElement("Address");
		if (address.firstChild().toText().data() == regAddr) {
			return getBitFieldsOfRegister(reg);
		}
		reg = reg.nextSibling().toElement();
	}
	f.close();

	return nullptr;
}

RegisterModel *XmlFileManager::getRegister(QDomElement reg)
{
	if (!reg.isNull()) {
		bool ok;
		return new RegisterModel( reg.firstChildElement("Name").firstChild().toText().data(),
					  reg.firstChildElement("Address").firstChild().toText().data().toInt(&ok,16),
					  reg.firstChildElement("Description").firstChild().toText().data(),
					  reg.firstChildElement("Exists").toText().data().toUpper() == "TRUE",
					  reg.firstChildElement("Width").firstChild().toText().data().toInt(),
					  reg.firstChildElement("Notes").firstChild().toText().data(),
					  getBitFieldsOfRegister(reg)
					  );
	}
	return nullptr;
}

QVector<BitFieldModel*> *XmlFileManager::getBitFieldsOfRegister(QDomElement reg)
{
	if (!reg.isNull()) {
		QVector<BitFieldModel*>  *bitFieldsList = new QVector<BitFieldModel*>();
		QDomElement bitFields = reg.firstChildElement("BitFields");
		QDomElement bf;

		bf = bitFields.firstChild().toElement();
		int regWidth = reg.firstChildElement("Width").firstChild().toText().data().toInt();
		int numberOfBits = 0;
		while(numberOfBits < regWidth) {
			if (!bf.isNull()) {
				int bitFieldRegOffset = bf.firstChildElement("RegOffset").firstChild().toText().data().toInt();
				if ( bitFieldRegOffset == numberOfBits){
					BitFieldModel *bitField = getBitField(bf);
					bitFieldsList->push_back(bitField);
					bf = bf.nextSibling().toElement();
					numberOfBits += bitField->getWidth();
				} else {
					BitFieldModel *bitField = new BitFieldModel("Bit " +QString::number(bitFieldRegOffset -1) + ":" + QString::number(numberOfBits), bitFieldRegOffset - numberOfBits, numberOfBits , "Reserved" , nullptr);
					bitFieldsList->push_back(bitField);
					numberOfBits+= bitFieldRegOffset - numberOfBits;
				}
			} else { //for all remaining ?
				BitFieldModel *bitField = new BitFieldModel("Bit " + QString::number(regWidth -1) + ":" + QString::number(numberOfBits) , regWidth - numberOfBits, numberOfBits, "Reserved", nullptr);
				bitFieldsList->push_back(bitField);
				numberOfBits += regWidth - numberOfBits;
			}
//			bf = bf.nextSibling().toElement();

		}
		return bitFieldsList;
	}
	return nullptr;
}

BitFieldModel *XmlFileManager::getBitField(QDomElement bitField)
{
	if (!bitField.isNull()) {
		QMap<QString,QString> *options = new QMap<QString, QString>();
		QDomElement bitFieldOptions = bitField.firstChildElement("Options");
		QDomElement bfOption = bitFieldOptions.firstChildElement("Option");
		while (!bfOption.isNull()) {
			options->insert(bfOption.firstChildElement("Description").firstChild().toText().data(),
					bfOption.firstChildElement("Value").firstChild().toText().data()
					);
			bfOption = bfOption.nextSibling().toElement();
		}
		return new BitFieldModel(
					bitField.firstChildElement("Name").firstChild().toText().data(),
					bitField.firstChildElement("Access").firstChild().toText().data(),
					bitField.firstChildElement("DefaultValue").firstChild().toText().data().toInt(),
					bitField.firstChildElement("Description").firstChild().toText().data(),
					bitField.firstChildElement("Visibility").firstChild().toText().data(),
					bitField.firstChildElement("Width").firstChild().toText().data().toInt(),
					bitField.firstChildElement("Notes").firstChild().toText().data(),
					bitField.firstChildElement("BitOffset").firstChild().toText().data().toInt(),
					bitField.firstChildElement("RegOffset").firstChild().toText().data().toInt(),
					bitField.firstChildElement("SliceWidth").firstChild().toText().data().toInt(),
					options
					);
	}
	return nullptr;
}
