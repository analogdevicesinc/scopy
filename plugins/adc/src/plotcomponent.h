#ifndef PLOTCOMPONENT_H
#define PLOTCOMPONENT_H

#include "measurementpanel.h"
#include "scopy-adcplugin_export.h"
#include "toolcomponent.h"

#include <QFuture>
#include <QTimer>
#include <QVBoxLayout>
#include <QtConcurrent>

#include <plotwidget.h>
#include "plotinfo.h"

namespace scopy {
namespace adc {

class SCOPY_ADCPLUGIN_EXPORT PlotComponent : public QWidget, public ToolComponent
{
	Q_OBJECT
public:
	PlotComponent(QString name = "PlotComponent", QWidget *parent = nullptr);
	~PlotComponent();

	virtual PlotWidget *plot();
public Q_SLOTS:
	virtual void replot();
	void enableMeasurementPanel(bool);
	void enableStatsPanel(bool);

	// virtual double sampleRate()

public:
	void onStart();
	void onStop();
	void onInit();
	void onDeinit();

	MeasurementsPanel *measurePanel() const;
	StatsPanel *statsPanel() const;

private:
	QVBoxLayout *m_lay;
	PlotWidget *m_plot;
	TimePlotInfo *m_info;

	MeasurementsPanel *m_measurePanel;
	StatsPanel *m_statsPanel;

};

} // namespace adc
} // namespace scopy

#endif // PLOTCOMPONENT_H
