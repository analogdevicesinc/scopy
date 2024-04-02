#include "datamonitorutils.hpp"

using namespace scopy;
using namespace datamonitor;

DataMonitorUtils::DataMonitorUtils(QObject *parent)
	: QObject{parent}
{}

double DataMonitorUtils::getAxisDefaultMinValue() { return 0; }

double DataMonitorUtils::getAxisDefaultMaxValue() { return 10; }

double DataMonitorUtils::getXAxisDefaultViewPortSize() { return 10; }

QString DataMonitorUtils::getToolSettingsId() { return "DataMonitorSettings"; }

double DataMonitorUtils::getReadIntervalDefaul() { return 1; }

int DataMonitorUtils::getDefaultPrecision() { return 3; }

QString DataMonitorUtils::getLoggingDateTimeFormat() { return "dd/MM/yyyy hh:mm:ss"; }
