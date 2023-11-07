#ifndef RMSINSTRUMENT_H
#define RMSINSTRUMENT_H
#include <QWidget>
#include <gui/widgets/measurementlabel.h>
#include <measurementpanel.h>
#include <scopy-pqmplugin_export.h>
#include <gui/polarplotwidget.h>
#include <gui/widgets/menucontrolbutton.h>
#include <gui/widgets/toolbuttons.h>

#define DEVICE_NAME "pqm"

namespace scopy::pqm {
class SCOPY_PQMPLUGIN_EXPORT RmsInstrument : public QWidget
{
	Q_OBJECT
public:
	RmsInstrument(QWidget *parent = nullptr);
	~RmsInstrument();

Q_SIGNALS:
	void enableTool(bool en, QString toolName = "rms");
	void runTme(bool en);
public Q_SLOTS:
	void onAttrAvailable(QMap<QString, QMap<QString, QString>> data);

private:
	void initPlot(PolarPlotWidget *plot);
	void updateLabels();
	void updatePlot(PolarPlotWidget *plot, QString type);
	void createLabels(MeasurementsPanel *mPanel, QStringList chnls, QStringList labels, QString color = "");
	void setupChannels(PolarPlotWidget *plot, QMap<QString, QString> channels, int thickness = 5);
	void setupMenuControlBtn(MenuControlButton *btn, QString name);

	RunBtn *m_runBtn;
	SingleShotBtn *m_singleBtn;
	PolarPlotWidget *m_voltagePlot;
	PolarPlotWidget *m_currentPlot;
	QWidget *m_voltageLabelWidget;
	QWidget *m_currentLabelWidget;
	QMap<QString, QList<MeasurementLabel *>> m_labels;
	QMap<QString, QMap<QString, QString>> m_attributes;
	const QMap<QString, QMap<QString, QString>> m_chnls = {
		{"voltage", {{"Ua", "ua"}, {"Ub", "ub"}, {"Uc", "uc"}}},
		{"current", {{"Ia", "ia"}, {"Ib", "ib"}, {"Ic", "ic"}, {"In", "in"}}}};
	const QMap<QString, QString> m_attrDictionary = {{"RMS", "rms"},
							 {"Angle", "angle"},
							 {"Deviation under", "deviation_under"},
							 {"Deviation over", "deviation_over"},
							 {"Pinst", "pinst"},
							 {"Pst", "pst"},
							 {"Plt", "plt"},
							 {"U2", "u2"},
							 {"U0", "u0"},
							 {"Sneg V", "sneg_voltage"},
							 {"Spos V", "spos_voltage"},
							 {"Szro V", "szro_voltage"},
							 {"I2", "u2"},
							 {"I0", "u0"},
							 {"Sneg I", "sneg_current"},
							 {"Spos I", "spos_current"},
							 {"Szro I", "szro_current"}};
};
} // namespace scopy::pqm
#endif // RMSINSTRUMENT_H
