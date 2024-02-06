#ifndef DATAMONITORSTYLEHELPER_HPP
#define DATAMONITORSTYLEHELPER_HPP

#include <QObject>

#include "datamonitortool.hpp"

namespace scopy {
namespace datamonitor {

class DataMonitorStyleHelper : public QObject
{
	Q_OBJECT
public:
	explicit DataMonitorStyleHelper(QObject *parent = nullptr);

	static void DataMonitorToolStyle(DataMonitorTool *tool);
	static QString RemoveButtonStyle();

signals:
};
} // namespace datamonitor
} // namespace scopy
#endif // DATAMONITORSTYLEHELPER_HPP
