#ifndef WAVEFORMINSTRUMENT_H
#define WAVEFORMINSTRUMENT_H

#include "scopy-pqm_export.h"

#include <QWidget>
#include <gui/spinbox_a.hpp>
#include <gui/plotwidget.h>
#include <gui/tooltemplate.h>
#include <gui/widgets/menucontrolbutton.h>
#include <gui/widgets/toolbuttons.h>
#include <pluginbase/resourcemanager.h>

namespace scopy::pqm {
class SCOPY_PQM_EXPORT WaveformInstrument : public QWidget, public ResourceUser
{
	Q_OBJECT
public:
	WaveformInstrument(QWidget *parent = nullptr);
	~WaveformInstrument();

public Q_SLOTS:
	void stop() override;
	void toggleWaveform(bool en);
	void onBufferDataAvailable(QMap<QString, QVector<double>> data);
Q_SIGNALS:
	void enableTool(bool en, QString toolName = "waveform");
	void runTme(bool en);

private:
	void initData();
	void initPlot(PlotWidget *plot, QString unitType, int yMin = -650, int yMax = 650);
	void setupChannels(PlotWidget *plot, QMap<QString, QString> chnls);
	QWidget *createSettMenu(QWidget *parent);

	void updateXData(int dataSize);
	void plotData(QVector<double> chnlData, QString chnlId);

	PlotWidget *m_voltagePlot;
	PlotWidget *m_currentPlot;
	RunBtn *m_runBtn;
	SingleShotBtn *m_singleBtn;
	GearBtn *m_settBtn;
	PositionSpinButton *m_timespanSpin;

	QMap<QString, PlotChannel *> m_plotChnls;
	QMap<QString, QVector<double>> m_yValues;
	QVector<double> m_xTime;

	const double m_plotSampleRate = 5120;
	const QMap<QString, QMap<QString, QString>> m_chnls = {
		{"voltage", {{"Ua", "ua"}, {"Ub", "ub"}, {"Uc", "uc"}}},
		{"current", {{"Ia", "ia"}, {"Ib", "ib"}, {"Ic", "ic"}, {"In", "in"}}}};
};
} // namespace scopy::pqm

#endif // WAVEFORMINSTRUMENT_H
