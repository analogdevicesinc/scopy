#ifndef PLOTMANAGER_H
#define PLOTMANAGER_H
#include "scopy-adc_export.h"
#include <QWidget>
#include "toolcomponent.h"
#include "measurementpanel.h"
#include "channelcomponent.h"

namespace scopy {
namespace adc {

class PlotManagerCombobox;
class SCOPY_ADC_EXPORT PlotManager : public QWidget, public MeasurementPanelInterface, public MetaComponent
{
	Q_OBJECT
public:
	PlotManager(QString name = "PlotManager", QWidget *parent = nullptr);
	~PlotManager();

	virtual uint32_t addPlot(QString name) = 0;
	virtual void removePlot(uint32_t uuid) = 0;

	virtual void addChannel(ChannelComponent *);
	virtual void moveChannel(ChannelComponent *, uint32_t uuid = 0);
	virtual void removeChannel(ChannelComponent *);

	       // TimePlotComponent* plot(QString name);
	PlotComponent *plot(uint32_t uuid);

	QList<PlotComponent *> plots() const;
	MeasurementsPanel *measurePanel() const override;
	StatsPanel *statsPanel() const override;

	QWidget *createMenu(QWidget *parent);
	QWidget *plotCombo(ChannelComponent *c);

public Q_SLOTS:
	void replot();
	void enableMeasurementPanel(bool) override;
	void enableStatsPanel(bool) override;

	void setXInterval(double xMin, double xMax);
	void selectChannel(ChannelComponent *c);
Q_SIGNALS:
	void plotRemoved(uint32_t);

protected:
	uint32_t m_plotIdx;
	QVBoxLayout *m_lay;
	QList<PlotComponent *> m_plots;
	QList<PlotComponentChannel *> m_channels;
	MeasurementsPanel *m_measurePanel;
	StatsPanel *m_statsPanel;
	QMap<ChannelComponent *, PlotManagerCombobox *> m_channelPlotcomboMap;
	// PlotSettings *m_plotSettings;

};
}}
#endif // PLOTMANAGER_H
