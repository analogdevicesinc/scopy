#ifndef HARMONICSINSTRUMENT_H
#define HARMONICSINSTRUMENT_H

#include <gui/widgets/menucombo.h>
#include <scopy-pqmplugin_export.h>
#include <gui/plotwidget.h>
#include <gui/tooltemplate.h>
#include <gui/flexgridlayout.hpp>
#include <QTableWidget>
#include <QWidget>
#include <gui/widgets/measurementlabel.h>
#include <gui/widgets/menucontrolbutton.h>
#include <gui/widgets/toolbuttons.h>

#define HARMONICS_MAX_DEGREE 50
#define MAX_CHNLS 6

namespace scopy::pqm {
class SCOPY_PQMPLUGIN_EXPORT HarmonicsInstrument : public QWidget
{
	Q_OBJECT
public:
	HarmonicsInstrument(QWidget *parent = nullptr);
	~HarmonicsInstrument();

public Q_SLOTS:
	void onAttrAvailable(QMap<QString, QMap<QString, QString>> attr);
Q_SIGNALS:
	void enableTool(bool en, QString toolName = "harmonics");
	void runTme(bool en);
private Q_SLOTS:
	void updateTable();
	void onActiveChnlChannged(QString chnlId);

private:
	QWidget *createSettingsMenu();
	void initData();
	void initTable();
	void initPlot();
	void setupPlotChannels();
	void createThdWidget(ToolTemplate *tool);

	QString m_harmonicsType;
	RunBtn *m_runBtn;
	SingleShotBtn *m_singleBtn;
	QTableWidget *m_table;
	PlotWidget *m_plot;
	std::vector<double> m_xTime;
	QMap<QString, std::vector<double>> m_yValues;
	QMap<QString, MeasurementLabel *> m_labels;
	QMap<QString, PlotChannel *> m_plotChnls;
	// keys - used for UI labels; values - context channels name
	const QMap<QString, QString> m_chnls = {{"Ia", "ia"}, {"Ib", "ib"}, {"Ic", "ic"},
						{"Ua", "ua"}, {"Ub", "ub"}, {"Uc", "uc"}};
};
} // namespace scopy::pqm
#endif // HARMONICSINSTRUMENT_H
