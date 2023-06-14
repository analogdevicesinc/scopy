#include "bitfield/bitfieldmodel.hpp"
#include "registermodel.hpp"

using namespace scopy::regmap;

RegisterModel::RegisterModel(QString name, uint32_t address, QString description, bool exists, int width, QString notes,
			     QVector<BitFieldModel *> *bitFields):
	name(name),
	address(address),
	description(description),
	exists(exists),
	width(width),
	notes(notes),
	bitFields(bitFields)
{}

QString RegisterModel::getName() const
{
	return name;
}

uint32_t RegisterModel::getAddress() const
{
	return address;
}

QString RegisterModel::getDescription() const
{
	return description;
}

bool RegisterModel::getExists() const
{
	return exists;
}

int RegisterModel::getWidth() const
{
	return width;
}

QString RegisterModel::getNotes() const
{
	return notes;
}

QVector<BitFieldModel *> *RegisterModel::getBitFields() const
{
	return bitFields;
}
