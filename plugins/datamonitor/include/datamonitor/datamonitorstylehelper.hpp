#ifndef DATAMONITORSTYLEHELPER_HPP
#define DATAMONITORSTYLEHELPER_HPP

#include <QObject>
#include <datamonitorview.hpp>

#include "datamonitorsettings.hpp"
#include "datamonitortool.hpp"
#include "monitorplot.hpp"

namespace scopy {
namespace datamonitor {

class DataMonitorStyleHelper : public QObject
{
	Q_OBJECT
public:
	explicit DataMonitorStyleHelper(QObject *parent = nullptr);

	static void DataMonitorViewStyle(DataMonitorView *dataMonitorView, QColor color);
	static void DataMonitorSettingsStyle(DataMonitorSettings *dataMonitorSettings);
	static void DataMonitorToolStyle(DataMonitorTool *tool);
	static QString RemoveButtonStyle();

signals:
};
} // namespace datamonitor
} // namespace scopy
#endif // DATAMONITORSTYLEHELPER_HPP
