#include "bitfieldoption.hpp"

BitFieldOption::BitFieldOption(QString value, QString description)
	: value(value)
	, description(description)
{}

QString BitFieldOption::getDescription() const { return description; }

QString BitFieldOption::getValue() const { return value; }
