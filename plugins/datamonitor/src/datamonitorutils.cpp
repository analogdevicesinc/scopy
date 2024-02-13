#include "datamonitorutils.hpp"

using namespace scopy;
using namespace datamonitor;

DataMonitorUtils::DataMonitorUtils(QObject *parent)
	: QObject{parent}
{}

double DataMonitorUtils::getYAxisDefaultMinValue() { return 0; }

double DataMonitorUtils::getYAxisDefaultMaxValue() { return 10; }
