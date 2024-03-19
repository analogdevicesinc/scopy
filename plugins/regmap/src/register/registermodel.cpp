#include "registermodel.hpp"

#include "bitfield/bitfieldmodel.hpp"

using namespace scopy::regmap;

RegisterModel::RegisterModel(QString name, uint32_t address, QString description, bool exists, int width, QString notes,
			     QVector<BitFieldModel *> *bitFields, RegisterMapTemplate *registerMapTemplate)
	: name(name)
	, address(address)
	, description(description)
	, exists(exists)
	, width(width)
	, notes(notes)
	, bitFields(bitFields)
	, m_registerMapTemaplate(registerMapTemplate)
{}

QString RegisterModel::getName() const { return name; }

uint32_t RegisterModel::getAddress() const { return address; }

QString RegisterModel::getDescription() const { return description; }

bool RegisterModel::getExists() const { return exists; }

int RegisterModel::getWidth() const { return width; }

QString RegisterModel::getNotes() const { return notes; }

QVector<BitFieldModel *> *RegisterModel::getBitFields() const { return bitFields; }

RegisterMapTemplate *RegisterModel::registerMapTemaplate() const { return m_registerMapTemaplate; }

void RegisterModel::setRegisterMapTemaplate(RegisterMapTemplate *newRegisterMapTemaplate)
{
	m_registerMapTemaplate = newRegisterMapTemaplate;
}
