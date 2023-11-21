#ifndef UNITOFMEASUREMENT_HPP
#define UNITOFMEASUREMENT_HPP

#include <QObject>
namespace scopy {
namespace datamonitor {

class UnitOfMeasurement : public QObject
{
	Q_OBJECT
public:
	UnitOfMeasurement(QString name, QString symbol);

	QString getName() const;
	QString getSymbol() const;
	QString getNameAndSymbol();

private:
	QString name;
	QString symbol;
};
} // namespace datamonitor
} // namespace scopy
#endif // UNITOFMEASUREMENT_HPP
