#ifndef HOCKEYPUCKTEMPSENSOR_H
#define HOCKEYPUCKTEMPSENSOR_H

#include "gui/tooltemplate.h"

#include "scopy-hockeypucktempsensorplugin_export.h"
#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <gui/lcdNumber.hpp>
#include <gui/plotwidget.h>
#include <gui/plotchannel.h>
#include <gui/plotautoscaler.h>
#include <deque>
#include <iio.h>

namespace scopy::HockeyPuckTempSensor {
using namespace scopy::gui;
#define HPTS_NR_SAMPLES 10
#define HPTS_COLOR StyleHelper::getColor("CH7")
#define HPTS_HIST 20

// #define TESTING
#ifdef TESTING
#define COMPATIBLE_DESCRIPTION "PREEMPT"
#define HPTS_DEV_NAME "xadc"
#define HPTS_CH_NAME "temp0"
#define HPTS_RAW "raw"
#define HPTS_INPUT_ATTR_NAME "temperature_input"
#define HPTS_OUTPUT_ATTR_NAME "temperature_output"
#define HPTS_OUT_UNIT_ATTR_NAME "convert_output_unit"

#else

#define COMPATIBLE_DESCRIPTION "Hockey Puck IIO Server"
#define HPTS_DEV_NAME "ad7124-4"
#define HPTS_CH_NAME "voltage0-voltage0"
#define HPTS_RAW "raw"
#define HPTS_INPUT_ATTR_NAME "temperature_input"
#define HPTS_OUTPUT_ATTR_NAME "temperature_output"
#define HPTS_OUT_UNIT_ATTR_NAME "convert_output_unit"

#endif

class SCOPY_HOCKEYPUCKTEMPSENSORPLUGIN_EXPORT HockeyPuckTempSensor : public QWidget
{
	Q_OBJECT
public:
	HockeyPuckTempSensor(iio_context *ctx, QWidget *parent = nullptr);
	~HockeyPuckTempSensor();
public Q_SLOTS:
	void run(bool);
private Q_SLOTS:
	void refresh();
	void setMin(double val);
	void setMax(double val);
	void updateUnit(QString txt);
	float readData();

private:
	void setupUiElements();

	ToolTemplate *m_tool;
	QTimer *tim;
	QWidget *tempWidget;
	QLabel *name;
	PlotWidget *plot;
	QComboBox *unitOfMeasurement;
	LcdNumber *sevenseg;
	PlotAutoscaler *pa;
	PlotChannel *ch;

	std::deque<float> deque;
	std::vector<float> datax;
	std::vector<float> datay;
	bool changedUnit;
	QString unit;

	void applyStylesheet(QString lcdColor);
	void initData();
	iio_context *m_ctx;
	iio_device *m_dev;
	iio_channel *m_ch;
};
} // namespace scopy::HockeyPuckTempSensor
#endif // HOCKEYPUCKTEMPSENSOR_H
