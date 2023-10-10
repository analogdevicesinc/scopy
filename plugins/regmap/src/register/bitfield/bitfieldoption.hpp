#ifndef BITFIELDOPTION_HPP
#define BITFIELDOPTION_HPP

#include <QObject>

class BitFieldOption
{
public:
	BitFieldOption(QString value, QString description);

	QString getDescription() const;

	QString getValue() const;

private:
	QString value;
	QString description;
};

#endif // BITFIELDOPTION_HPP
