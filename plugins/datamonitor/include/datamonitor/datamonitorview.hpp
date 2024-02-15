#ifndef DATAMONITORVIEW_HPP
#define DATAMONITORVIEW_HPP

#include "datamonitormodel.hpp"

#include <QLabel>
#include <QLineEdit>
#include <QWidget>
#include <lcdNumber.hpp>

#include "monitorplot.hpp"
#include "scopy-datamonitor_export.h"

namespace scopy {
namespace datamonitor {

class SCOPY_DATAMONITOR_EXPORT DataMonitorView : public QFrame
{
	friend class DataMonitorStyleHelper;
	Q_OBJECT
public:
	explicit DataMonitorView(QFrame *parent = nullptr);

	MonitorPlot *monitorPlot() const;
	QString getTitle();

	void togglePeakHolder(bool toggle);
	void togglePlot(bool toggle);

	void configureMonitor(DataMonitorModel *dataMonitorModel);
	void setMeasureUnit(UnitOfMeasurement *newMeasureUnit);

	void toggleSelected();

	bool isSelected() const;

Q_SIGNALS:
	void widgetClicked();
	void titleChanged(QString newTitle);

private:
	QWidget *m_header;
	QLabel *m_measuringUnit;
	QLineEdit *m_title;
	MonitorPlot *m_monitorPlot;

	UnitOfMeasurement *m_measureUnit;

	bool eventFilter(QObject *watched, QEvent *event) override;
	bool m_isSelected = false;
};
} // namespace datamonitor
} // namespace scopy
#endif // DATAMONITORVIEW_HPP
