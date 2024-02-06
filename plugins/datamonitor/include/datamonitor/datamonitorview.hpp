#ifndef DATAMONITORVIEW_HPP
#define DATAMONITORVIEW_HPP

#include <QWidget>

namespace scopy {
namespace datamonitor {

class MonitorPlot;

class DataMonitorView : public QWidget
{
	Q_OBJECT
public:
	explicit DataMonitorView(QWidget *parent = nullptr);

	MonitorPlot *monitorPlot() const;

signals:

private:
	MonitorPlot *m_monitorPlot;
};
} // namespace datamonitor
} // namespace scopy
#endif // DATAMONITORVIEW_HPP
