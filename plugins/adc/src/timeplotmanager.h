#ifndef TIMEPLOTMANAGER_H
#define TIMEPLOTMANAGER_H
#include "scopy-adc_export.h"
#include <QWidget>
#include <interfaces.h>
#include <channelcomponent.h>
#include <measurementpanel.h>

namespace scopy {
namespace adc {
class SCOPY_ADC_EXPORT TimePlotManager : public QWidget, public MeasurementPanelInterface, public MetaComponent
{
	Q_OBJECT
public:
	TimePlotManager(QString name = "TimePlotManager", QWidget *parent = nullptr);
	~TimePlotManager();

	uint32_t addPlot(QString name);
	void removePlot(uint32_t uuid);

	void addChannel(ChannelComponent *, uint32_t uuid = 0);
	void moveChannel(ChannelComponent *, uint32_t uuid = 0);
	void removeChannel(ChannelComponent *);

	// TimePlotComponent* plot(QString name);
	TimePlotComponent *plot(uint32_t uuid);

	QList<TimePlotComponent *> plots() const;
	MeasurementsPanel *measurePanel() const override;
	StatsPanel *statsPanel() const override;

	QWidget *createMenu(QWidget *parent);

public Q_SLOTS:
	void replot();
	void enableMeasurementPanel(bool) override;
	void enableStatsPanel(bool) override;

	void setXInterval(double xMin, double xMax);
	void selectChannel(ChannelComponent *c);

private:
	uint32_t m_plotIdx;
	QVBoxLayout *m_lay;
	QList<TimePlotComponent *> m_plots;
	QList<TimePlotComponentChannel *> m_channels;
	MeasurementsPanel *m_measurePanel;
	StatsPanel *m_statsPanel;
	// PlotSettings *m_plotSettings;
};
} // namespace adc
} // namespace scopy
#endif // TIMEPLOTMANAGER_H
