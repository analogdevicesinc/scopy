#include "registermapvalues.hpp"
#include "regreadwrite.hpp"
#include "readwrite/iregisterreadstrategy.hpp"
#include "readwrite/iregisterwritestrategy.hpp"

RegisterMapValues::RegisterMapValues(QObject *parent)
	: QObject{parent}
{
	registerReadValues = new QMap<uint32_t, uint32_t>();

	m_readConnection = QObject::connect(this, &RegisterMapValues::requestRead, this, &RegisterMapValues::getValueOfRegister);
	writeConnection = QObject::connect(this, &RegisterMapValues::requestWrite, this, &RegisterMapValues::readDone);

}

RegisterMapValues::~RegisterMapValues()
{
	delete registerReadValues;
}

QMap<uint32_t, uint32_t> *RegisterMapValues::getRegisterReadValues() const
{
	return registerReadValues;
}

bool RegisterMapValues::hasValue(uint32_t address)
{
	return registerReadValues->contains(address);
}

void RegisterMapValues::readDone(uint32_t address, uint32_t value)
{
	registerReadValues->insert(address, value);
	Q_EMIT registerValueChanged(address, value);
}

uint32_t RegisterMapValues::getValueOfRegister(uint32_t address)
{
	return registerReadValues->value(address);
}

void RegisterMapValues::setReadStrategy(IRegisterReadStrategy *readStrategy)
{
	this->readStrategy = readStrategy;
	QObject::disconnect(m_readConnection);
	QObject::connect(this, &RegisterMapValues::requestRead, readStrategy, &IRegisterReadStrategy::read);
	QObject::connect(readStrategy, &IRegisterReadStrategy::readDone, this, &RegisterMapValues::readDone);

}

void RegisterMapValues::setWriteStrategy(IRegisterWriteStrategy *writeStrategy)
{
	this->writeStrategy = writeStrategy;
	QObject::disconnect(writeConnection);
	QObject::connect(this, &RegisterMapValues::requestWrite, writeStrategy, &IRegisterWriteStrategy::write);
	if (readStrategy) {
		QObject::connect(writeStrategy, &IRegisterWriteStrategy::writeSuccess, readStrategy, &IRegisterReadStrategy::read);
	}
}

IRegisterReadStrategy *RegisterMapValues::getReadStrategy() const
{
	return readStrategy;
}

IRegisterWriteStrategy *RegisterMapValues::getWriteStrategy() const
{
	return writeStrategy;
}
