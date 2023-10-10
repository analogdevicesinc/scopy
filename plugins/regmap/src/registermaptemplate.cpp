#include "registermaptemplate.hpp"

#include "register/bitfield/bitfieldmodel.hpp"
#include "register/registermodel.hpp"
#include "search.hpp"
#include "xmlfilemanager.hpp"

#include <QMap>

using namespace scopy;
using namespace regmap;

RegisterMapTemplate::RegisterMapTemplate(QObject *parent)
	: QObject{parent}
{
	registerList = new QMap<uint32_t, RegisterModel *>();
}

RegisterMapTemplate::~RegisterMapTemplate() { delete registerList; }

QMap<uint32_t, RegisterModel *> *RegisterMapTemplate::getRegisterList() const { return registerList; }

RegisterModel *RegisterMapTemplate::getRegisterTemplate(uint32_t address)
{
	if(registerList->contains(address)) {
		return registerList->value(address);
	}
	return getDefaultTemplate(address);
}

void RegisterMapTemplate::setRegisterList(QMap<uint32_t, RegisterModel *> *newRegisterList)
{
	registerList = newRegisterList;
}

RegisterModel *RegisterMapTemplate::getDefaultTemplate(uint32_t address)
{
	QVector<BitFieldModel *> *bitFieldsList = new QVector<BitFieldModel *>();

	for(int i = 0; i < 8; ++i) {
		bitFieldsList->push_back(new BitFieldModel("Bit " + QString::number(i), 1, 8 - i, ""));
	}

	return new RegisterModel("Register Name", address, "Register Description", true, 8, "Reigster Notes",
				 bitFieldsList);
}
