#ifndef DATAMONITORUTILS_HPP
#define DATAMONITORUTILS_HPP

#include <QObject>
#include "scopy-datamonitor_export.h"

namespace scopy {
namespace datamonitor {
class SCOPY_DATAMONITOR_EXPORT DataMonitorUtils : public QObject
{
	Q_OBJECT
public:
	explicit DataMonitorUtils(QObject *parent = nullptr);

	static double getAxisDefaultMinValue();
	static double getAxisDefaultMaxValue();
	static double getXAxisDefaultViewPortSize();
};
} // namespace datamonitor
} // namespace scopy
#endif // DATAMONITORUTILS_HPP
