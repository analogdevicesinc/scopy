#ifndef UNITOFMEASUREMENT_HPP
#define UNITOFMEASUREMENT_HPP

#include <QObject>
#include "scopy-datamonitorplugin_export.h"

namespace scopy {
namespace datamonitor {

class SCOPY_DATAMONITORPLUGIN_EXPORT UnitOfMeasurement : public QObject
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
