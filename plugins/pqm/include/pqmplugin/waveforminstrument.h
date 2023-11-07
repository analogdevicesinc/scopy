#ifndef WAVEFORMINSTRUMENT_H
#define WAVEFORMINSTRUMENT_H

#include "scopy-pqmplugin_export.h"

#include <QWidget>
#include <gui/plotwidget.h>
#include <gui/tooltemplate.h>
#include <gui/widgets/menucontrolbutton.h>
#include <gui/widgets/toolbuttons.h>

#define SAMPLE_RATE 1024
namespace scopy::pqm {
class SCOPY_PQMPLUGIN_EXPORT WaveformInstrument : public QWidget
{
	Q_OBJECT
public:
	WaveformInstrument(QWidget *parent = nullptr);
	~WaveformInstrument();

	void initData();
public Q_SLOTS:
	void onBufferDataAvailable(QMap<QString, std::vector<double>> data);
Q_SIGNALS:
	void enableTool(bool en, QString toolName = "waveform");
	void runTme(bool en);

private:
	void initPlot(PlotWidget *plot, QString unitType, int yMin = -650, int yMax = 650);
	void setupChannels(PlotWidget *plot, QMap<QString, QString> chnls);

	ToolTemplate *m_tool;
	PlotWidget *m_voltagePlot;
	PlotWidget *m_currentPlot;
	RunBtn *m_runBtn;
	SingleShotBtn *m_singleBtn;

	QMap<QString, std::vector<double>> m_yValues;
	std::vector<double> m_xTime;
	const QMap<QString, QMap<QString, QString>> m_chnls = {
		{"voltage", {{"Ua", "ua"}, {"Ub", "ub"}, {"Uc", "uc"}}},
		{"current", {{"Ia", "ia"}, {"Ib", "ib"}, {"Ic", "ic"}, {"In", "in"}}}};
};
} // namespace scopy::pqm

#endif // WAVEFORMINSTRUMENT_H
