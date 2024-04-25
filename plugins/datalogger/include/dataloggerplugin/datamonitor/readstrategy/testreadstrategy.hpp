#ifndef TESTREADSTRATEGY_HPP
#define TESTREADSTRATEGY_HPP

#include "ireadstrategy.hpp"
#include "../../scopy-dataloggerplugin_export.h"

namespace scopy {
namespace datamonitor {

class SCOPY_DATALOGGERPLUGIN_EXPORT TestReadStrategy : public IReadStrategy
{
public:
	TestReadStrategy();

	// IReadStrategy interface
public:
	void read();
	double testDataValue = 0;
	double testDataTime = 0;
};
} // namespace datamonitor
} // namespace scopy
#endif // TESTREADSTRATEGY_HPP
