#include "datamonitor/readstrategy/testreadstrategy.hpp"

using namespace scopy;
using namespace datamonitor;

TestReadStrategy::TestReadStrategy() {}

void TestReadStrategy::read()
{
	testDataValue += 1;
	testDataTime += 1;
	Q_EMIT readDone(testDataTime, testDataValue);
}
