#ifndef HARMONICCALIBRATION_H
#define HARMONICCALIBRATION_H

#include "scopy-admt_export.h"
#include "sismograph.hpp"

#include <cstdint>

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
	HarmonicCalibration(ADMTController *m_admtController, bool isDebug = false, QWidget *parent = nullptr);
	~HarmonicCalibration();
	bool running() const;
	void setRunning(bool newRunning);
public Q_SLOTS:
	void run(bool);
	void stop();
	void start();
	void restart();
	void timerTask();
	void calibrationTask();
	void motorCalibrationAcquisitionTask();
	void utilityTask();
	void clearCommandLog();
	void canCalibrate(bool);
	void applySequence();
	void readSequence();
Q_SIGNALS:
	void runningChanged(bool);
	void canCalibrateChanged(bool);
private:
	ADMTController *m_admtController;
	iio_context *m_ctx;
	bool m_running, isDebug;
	ToolTemplate *tool;
	GearBtn *settingsButton;
	InfoBtn *infoButton;
	RunBtn *runButton;

	double rotation, angle, count, temp, amax, rotate_vmax, dmax, disable, target_pos, current_pos, ramp_mode,
		afeDiag0, afeDiag1, afeDiag2;

	QPushButton *openLastMenuButton, *calibrationStartMotorButton, *applyCalibrationDataButton, *calibrateDataButton, *extractDataButton,
				*clearCommandLogButton, *applySequenceButton;
	QButtonGroup *rightMenuButtonGroup;

	QLineEdit 	*graphUpdateIntervalLineEdit, *dataSampleSizeLineEdit,
				*dataGraphSamplesLineEdit, *tempGraphSamplesLineEdit, 
				*calibrationH1MagLineEdit, *calibrationH2MagLineEdit, 
				*calibrationH3MagLineEdit, *calibrationH8MagLineEdit,
				*calibrationH1PhaseLineEdit, *calibrationH2PhaseLineEdit, 
				*calibrationH3PhaseLineEdit, *calibrationH8PhaseLineEdit,
				*AFEDIAG0LineEdit, *AFEDIAG1LineEdit, *AFEDIAG2LineEdit;

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
	MenuCombo *m_dataGraphChannelMenuCombo, *m_dataGraphDirectionMenuCombo, *m_tempGraphDirectionMenuCombo,
			  *sequenceTypeMenuCombo, *conversionTypeMenuCombo, *cnvSourceMenuCombo, *convertSynchronizationMenuCombo, *angleFilterMenuCombo, *eighthHarmonicMenuCombo;

	QTabWidget *tabWidget;

	QListWidget *rawDataListWidget;

	QPlainTextEdit *logsPlainTextEdit, *commandLogPlainTextEdit;

	QCheckBox *autoCalibrateCheckBox;

	PlotWidget *preCalibrationFFTPlotWidget, *calibrationRawDataPlotWidget;
	PlotAxis *preCalibrationFFTXPlotAxis, *preCalibrationFFTYPlotAxis, *calibrationRawDataXPlotAxis, *calibrationRawDataYPlotAxis;
	PlotChannel *preCalibrationFFTMagnitudePlotChannel, *preCalibrationFFTPhasePlotChannel, *calibrationRawDataPlotChannel, *calibrationSineDataPlotChannel, *calibrationCosineDataPlotChannel;

	HorizontalSpinBox *motorMaxVelocitySpinBox, *motorAccelTimeSpinBox, *motorMaxDisplacementSpinBox, *motorTargetPositionSpinBox;

	MenuControlButton *DIGIOBusyStatusLED ,*DIGIOCNVStatusLED ,*DIGIOSENTStatusLED ,*DIGIOACALCStatusLED ,*DIGIOFaultStatusLED ,*DIGIOBootloaderStatusLED,
		*R0StatusLED, *R1StatusLED, *R2StatusLED, *R3StatusLED, *R4StatusLED, *R5StatusLED, *R6StatusLED, *R7StatusLED,
		*VDDUnderVoltageStatusLED, *VDDOverVoltageStatusLED, *VDRIVEUnderVoltageStatusLED, *VDRIVEOverVoltageStatusLED, 
		*AFEDIAGStatusLED, *NVMCRCFaultStatusLED, *ECCDoubleBitErrorStatusLED, *OscillatorDriftStatusLED, *CountSensorFalseStateStatusLED, 
		*AngleCrossCheckStatusLED, *TurnCountSensorLevelsStatusLED, *MTDIAGStatusLED, *TurnCounterCrossCheckStatusLED, *RadiusCheckStatusLED, *SequencerWatchdogStatusLED;

	void updateChannelValues();
	void updateLineEditValues();
	void updateGeneralSettingEnabled(bool value);
	void connectLineEditToNumber(QLineEdit* lineEdit, int& variable, int min, int max);
	void connectLineEditToNumber(QLineEdit* lineEdit, double& variable, QString unit = "");
	void connectLineEditToGraphSamples(QLineEdit* lineEdit, int& variable, Sismograph* graph, int min, int max);
	void connectMenuComboToGraphDirection(MenuCombo* menuCombo, Sismograph* graph);
	void connectMenuComboToGraphChannel(MenuCombo* menuCombo, Sismograph* graph);
	void connectMenuComboToNumber(MenuCombo* menuCombo, double& variable);
	void connectMenuComboToNumber(MenuCombo* menuCombo, int& variable);
	void changeGraphColorByChannelName(Sismograph* graph, const char* channelName);
	ToolTemplate* createCalibrationWidget();
	ToolTemplate* createRegistersWidget();
	ToolTemplate* createUtilityWidget();
	void updateLabelValue(QLabel* label, int channelIndex);
	void updateLabelValue(QLabel *label, ADMTController::MotorAttribute attribute);
	void updateChannelValue(int channelIndex);
	void addAngleToRawDataList();
	void calibrateData();
	void registerCalibrationData();
	void extractCalibrationData();
	void importCalibrationData();
	void calibrationLogWrite(QString message);
	void calibrationLogWriteLn(QString message = "");
	void commandLogWrite(QString message);
	void readMotorAttributeValue(ADMTController::MotorAttribute attribute, double& value);
	void writeMotorAttributeValue(ADMTController::MotorAttribute attribute, double value);
	void applyLineEditStyle(QLineEdit *widget);
	void applyComboBoxStyle(QComboBox *widget, const QString& styleHelperColor = "CH0");
	void applyTextStyle(QWidget *widget, const QString& styleHelperColor = "CH0", bool isBold = false);
	void applyLabelStyle(QLabel *widget);
	void initializeMotor();
	void stepMotorAcquisition(double step = -408);
	void clearRawDataList();
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
	void appendSamplesToPlotCurve(PlotWidget *plotWidget, QVector<double>& newYData);
	void applyTabWidgetStyle(QTabWidget *widget, const QString& styleHelperColor = "ScopyBlue");
	MenuControlButton *createStatusLEDWidget(const QString title, QColor color, QWidget *parent = nullptr);
	MenuControlButton *createChannelToggleWidget(const QString title, QColor color, QWidget *parent = nullptr);
	void updateDigioMonitor();
	void updateMTDiagRegister();
	void updateFaultRegister();
	void updateMTDiagnostics();
	void changeStatusLEDColor(MenuControlButton *menuControlButton, QColor color, bool checked = true);
	bool changeCNVPage(uint32_t page, QString registerName);
	void toggleWidget(QPushButton *widget, bool value);
	void GMRReset();
	void updateCountValue();

	QTimer *timer, *calibrationTimer, *motorCalibrationAcquisitionTimer, *utilityTimer;

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
