#ifndef HARMONICCALIBRATION_H
#define HARMONICCALIBRATION_H

#include "scopy-admt_export.h"
#include "sismograph.hpp"

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QString>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QButtonGroup>
#include <QTimer>
#include <QSpacerItem>
#include <QVariant>
#include <QTabWidget>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QFileDialog>
#include <QGridLayout>
#include <QIcon>
#include <QPen>
#include <QCheckBox>

#include <iio.h>
#include <iioutil/connectionprovider.h>
#include <admtcontroller.h>
#include <admtplugin.h>
#include <toolbuttons.h>
#include <tooltemplate.h>
#include <menuheader.h>
#include <menusectionwidget.h>
#include <menucollapsesection.h>
#include <menucontrolbutton.h>
#include <menucombo.h>
#include <stylehelper.h>
#include <filemanager.h>
#include <customSwitch.h>
#include <widgets/horizontalspinbox.h>
#include <widgets/registerblockwidget.h>
#include <pluginbase/statusbarmanager.h>

namespace scopy::admt {

class SCOPY_ADMT_EXPORT HarmonicCalibration : public QWidget
{
	Q_OBJECT
public:
	HarmonicCalibration(ADMTController *m_admtController, QWidget *parent = nullptr);
	~HarmonicCalibration();
	bool running() const;
	void setRunning(bool newRunning);
public Q_SLOTS:
	void run(bool);
	void stop();
	void start();
	void restart();
	void timerTask();
	void canCalibrate(bool);
Q_SIGNALS:
	void runningChanged(bool);
	void canCalibrateChanged(bool);
private:
	ADMTController *m_admtController;
	iio_context *m_ctx;
	bool m_running;
	ToolTemplate *tool;
	GearBtn *settingsButton;
	InfoBtn *infoButton;
	RunBtn *runButton;

	double rotation, angle, count, temp, amax, rotate_vmax, dmax, disable, target_pos, current_pos, ramp_mode;

	QPushButton *openLastMenuButton, *calibrationStartMotorButton, *applyCalibrationDataButton, *calibrateDataButton, *extractDataButton;
	QButtonGroup *rightMenuButtonGroup;

	QLineEdit 	*graphUpdateIntervalLineEdit, *dataSampleSizeLineEdit,
				*dataGraphSamplesLineEdit, *tempGraphSamplesLineEdit, 
				*calibrationH1MagLineEdit, *calibrationH2MagLineEdit, 
				*calibrationH3MagLineEdit, *calibrationH8MagLineEdit,
				*calibrationH1PhaseLineEdit, *calibrationH2PhaseLineEdit, 
				*calibrationH3PhaseLineEdit, *calibrationH8PhaseLineEdit;
	QLabel 	*rotationValueLabel, *angleValueLabel, *countValueLabel, *tempValueLabel, 
			*calibrationMotorCurrentPositionLabel,
			*motorAmaxValueLabel, *motorRotateVmaxValueLabel, *motorDmaxValueLabel,
			*motorDisableValueLabel, *motorTargetPosValueLabel, *motorCurrentPosValueLabel,
			*motorRampModeValueLabel,
			*calibrationH1MagLabel,
			*calibrationH1PhaseLabel,
			*calibrationH2MagLabel,
			*calibrationH2PhaseLabel,
			*calibrationH3MagLabel,
			*calibrationH3PhaseLabel,
			*calibrationH8MagLabel,
			*calibrationH8PhaseLabel;

	Sismograph *dataGraph, *tempGraph, *calibrationRawDataSismograph;

	MenuHeaderWidget *header;

	MenuSectionWidget *rightMenuSectionWidget;
	MenuCollapseSection *rotationCollapse, *angleCollapse, *countCollapse, *tempCollapse;
	MenuCombo *m_dataGraphChannelMenuCombo, *m_dataGraphDirectionMenuCombo, *m_tempGraphDirectionMenuCombo;

	QTabWidget *tabWidget;

	QListWidget *rawDataListWidget;

	QPlainTextEdit *logsPlainTextEdit;

	QCheckBox *autoCalibrateCheckBox;

	PlotWidget *calibrationFFTDataPlotWidget, *calibrationRawDataPlotWidget;
	PlotAxis *calibrationFFTXPlotAxis, *calibrationFFTYPlotAxis, *calibrationRawDataXPlotAxis, *calibrationRawDataYPlotAxis;
	PlotChannel *calibrationFFTPlotChannel, *calibrationFFTPhasePlotChannel, *calibrationRawDataPlotChannel, *calibrationSineDataPlotChannel, *calibrationCosineDataPlotChannel;

	HorizontalSpinBox *motorMaxVelocitySpinBox, *motorAccelTimeSpinBox, *motorMaxDisplacementSpinBox, *motorTargetPositionSpinBox;

	void updateChannelValues();
	void updateLineEditValues();
	void updateGeneralSettingEnabled(bool value);
	void connectLineEditToNumber(QLineEdit* lineEdit, int& variable);
	void connectLineEditToNumber(QLineEdit* lineEdit, double& variable);
	void connectLineEditToGraphSamples(QLineEdit* lineEdit, int& variable, Sismograph* graph);
	void connectMenuComboToGraphDirection(MenuCombo* menuCombo, Sismograph* graph);
	void changeGraphColorByChannelName(Sismograph* graph, const char* channelName);
	void connectMenuComboToGraphChannel(MenuCombo* menuCombo, Sismograph* graph);
	ToolTemplate* createCalibrationWidget();
	ToolTemplate* createRegistersWidget();
	ToolTemplate* createUtilityWidget();
	void updateLabelValue(QLabel* label, int channelIndex);
	void updateLabelValue(QLabel *label, ADMTController::MotorAttribute attribute);
	void updateChannelValue(int channelIndex);
	void calibrationTask();
	void addAngleToRawDataList();
	void calibrateData();
	void registerCalibrationData();
	void extractCalibrationData();
	void importCalibrationData();
	void calibrationLogWrite(QString message);
	void calibrationLogWriteLn(QString message = "");
	void readMotorAttributeValue(ADMTController::MotorAttribute attribute, double& value);
	void writeMotorAttributeValue(ADMTController::MotorAttribute attribute, double value);
	void applyLineEditStyle(QLineEdit *widget);
	void applyComboBoxStyle(QComboBox *widget, const QString& styleHelperColor = "CH0");
	void applyTextStyle(QWidget *widget, const QString& styleHelperColor = "CH0", bool isBold = false);
	void applyLabelStyle(QLabel *widget);
	void initializeMotor();
	void stepMotorAcquisition(double step = -408);
	void clearRawDataList();
	void motorCalibrationAcquisitionTask();
	void connectLineEditToRPSConversion(QLineEdit* lineEdit, double& vmax);
	void connectLineEditToNumberWrite(QLineEdit* lineEdit, double& variable, ADMTController::MotorAttribute attribute);
	double convertRPStoVMAX(double rps);
	double convertVMAXtoRPS(double vmax);
	void connectLineEditToAMAXConversion(QLineEdit* lineEdit, double& amax);
	void connectRegisterBlockToRegistry(RegisterBlockWidget* widget);
	double convertAccelTimetoAMAX(double accelTime);
	double convertAMAXtoAccelTime(double amax);
	void updateCalculatedCoeff();
	void resetCalculatedCoeff();
	void connectMenuComboToNumber(MenuCombo* menuCombo, double& variable);
	void appendSamplesToPlotCurve(PlotWidget *plotWidget, QVector<double>& newYData);
	void applyTabWidgetStyle(QTabWidget *widget, const QString& styleHelperColor = "ScopyBlue");
	MenuControlButton *createStatusLEDWidget(const QString title, QColor color, QWidget *parent = nullptr);
	MenuControlButton *createChannelToggleWidget(const QString title, QColor color, QWidget *parent = nullptr);

	QTimer *timer, *calibrationTimer, *motorCalibrationAcquisitionTimer;

	int uuid = 0;
	const char *rotationChannelName, *angleChannelName, *countChannelName, *temperatureChannelName;
};

enum TABS
{
	ACQUISITION = 0,
	UTILITIES = 1,
	CALIBRATION = 2,
};



} // namespace scopy::admt
#endif // HARMONICCALIBRATION_H
