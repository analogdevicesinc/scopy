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
	void updateValue(double time, double value);
	void updateMinValue(double value);
	void updateMaxValue(double value);
	void updatePrecision(int precision);

	void togglePeakHolder(bool toggle);
	void togglePlot(bool toggle);

	void configureMonitor(DataMonitorModel *dataMonitorModel);
	void setMeasureUnit(UnitOfMeasurement *newMeasureUnit);
	void resetView();

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
	QWidget *m_valueWidget;
	LcdNumber *m_value;
	QLabel *m_valueUMLabel;
	QWidget *m_minValueWidget;
	LcdNumber *m_minValue;
	QLabel *m_minValueUMLabel;
	QWidget *m_maxValueWidget;
	LcdNumber *m_maxValue;
	QLabel *m_maxValueUMLabel;

	UnitOfMeasurement *m_measureUnit;
	QString m_mainMonitor = "";
	QMetaObject::Connection valueUpdateConnection;
	QMetaObject::Connection minValueUpdateConnection;
	QMetaObject::Connection maxValueUpdateConnection;

	bool eventFilter(QObject *watched, QEvent *event) override;
	bool m_isSelected = false;
};
} // namespace datamonitor
} // namespace scopy
#endif // DATAMONITORVIEW_HPP
