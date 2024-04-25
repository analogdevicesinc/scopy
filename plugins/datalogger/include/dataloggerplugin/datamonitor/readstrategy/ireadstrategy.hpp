#ifndef IREGISTERREADSTRATEGY_HPP
#define IREGISTERREADSTRATEGY_HPP

#include <QObject>
#include "../../scopy-dataloggerplugin_export.h"

namespace scopy {
namespace datamonitor {
class SCOPY_DATALOGGERPLUGIN_EXPORT IReadStrategy : public QObject
{
	Q_OBJECT
public:
	virtual void read() = 0;

Q_SIGNALS:
	void readDone(double time, double value);
	void readError(const char *err);
};
} // namespace datamonitor
} // namespace scopy
#endif // IREGISTERREADSTRATEGY_HPP
