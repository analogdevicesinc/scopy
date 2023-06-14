#include "search.hpp"

#include "register/registermodel.hpp"
#include "register/bitfield/bitfieldmodel.hpp"

using namespace scopy;
using namespace regmap;

Search::Search(QObject *parent)
	: QObject{parent}
{
}

QList<uint32_t> Search::searchForRegisters(QMap<uint32_t, RegisterModel *> *registers, QString searchParam)
{
	if (searchParam.isEmpty()) {
		return registers->keys();
	}

	QList<uint32_t> result;
	QMap<uint32_t, RegisterModel*>::iterator mapIterator;
	for (mapIterator = registers->begin(); mapIterator != registers->end(); ++mapIterator) {
		QString address = QString::number(mapIterator.key(),16);
		if (address.contains(searchParam) || mapIterator.value()->getName().contains(searchParam) ){
			result.push_back(mapIterator.key());
		} else {
			for (int i = 0 ; i < mapIterator.value()->getBitFields()->size(); ++i){
				if (mapIterator.value()->getBitFields()->at(i)->getName().toLower().contains(searchParam) ||
						mapIterator.value()->getBitFields()->at(i)->getDescription().toLower().contains(searchParam) ) {

					result.push_back(mapIterator.key());
					break;
				}
			}
		}
	}
	return result;
}
