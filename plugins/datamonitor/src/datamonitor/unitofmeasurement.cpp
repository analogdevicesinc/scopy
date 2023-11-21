#include "unitofmeasurement.hpp"

using namespace scopy;
using namespace datamonitor;

UnitOfMeasurement::UnitOfMeasurement(QString name, QString symbol)
	: name(name)
	, symbol(symbol)
{}

QString UnitOfMeasurement::getName() const { return name; }

QString UnitOfMeasurement::getSymbol() const { return symbol; }

QString UnitOfMeasurement::getNameAndSymbol() { return QString(name + " ( " + symbol + " )"); }
