#ifndef HARMONICCALIBRATION_H
#define HARMONICCALIBRATION_H

#include "scopy-admt_export.h"

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
#include <QFuture>
#include <QFutureWatcher>
#include <QtMath>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QThread>
#include <QMetaObject>
#include <QMap>

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
#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>
#include <pluginbase/statusbarmanager.h>
#include <plotwidget.h>
#include <plotaxis.h>

enum AcquisitionDataKey
{
	RADIUS,
	ANGLE,
	TURNCOUNT,
	ABSANGLE,
	SINE,
	COSINE,
	SECANGLI,
	SECANGLQ,
	ANGLESEC,
	DIAG1,
	DIAG2,
	TMP0,
	TMP1,
	CNVCNT,
	SCRADIUS,
	SPIFAULT
};

namespace scopy{
namespace admt {

class SCOPY_ADMT_EXPORT HarmonicCalibration : public QWidget
{
	Q_OBJECT
public:
	HarmonicCalibration(ADMTController *m_admtController, bool isDebug = false, QWidget *parent = nullptr);
	~HarmonicCalibration();
	bool running() const;
	void setRunning(bool newRunning);
	void requestDisconnect();
public Q_SLOTS:
	void run(bool);
	void stop();
	void start();
	void restart();
	void commandLogWrite(QString message);
	void updateFaultStatus(bool value);
	void updateMotorPosition(double position);
	void updateDIGIOUI(uint32_t *registerValue);
	void updateFaultRegisterUI(uint32_t *registerValue);
	void updateMTDiagnosticRegisterUI(uint32_t *registerValue);
	void updateMTDiagnosticsUI(uint32_t *registerValue);
Q_SIGNALS:
	void runningChanged(bool);
	void canCalibrateChanged(bool);
	void updateUtilityUI();
	void commandLogWriteSignal(QString message);
	void updateFaultStatusSignal(bool value);
	void motorPositionChanged(double position);
	void DIGIORegisterChanged(uint32_t *registerValue);
	void FaultRegisterChanged(uint32_t *registerValue);
	void DIAG1RegisterChanged(uint32_t *registerValue);
	void DIAG2RegisterChanged(uint32_t *registerValue);
private:
	ADMTController *m_admtController;
	iio_context *m_ctx;
	bool m_running, isDebug;
	ToolTemplate *tool;
	GearBtn *settingsButton;
	InfoBtn *infoButton;
	RunBtn *runButton;

	const char *rotationChannelName, *angleChannelName, *countChannelName, *temperatureChannelName;

	double rotation, angle, count, temp = 0.0, amax, rotate_vmax, dmax, disable, target_pos, current_pos, ramp_mode,
		afeDiag0, afeDiag1, afeDiag2;

	QPushButton *openLastMenuButton, *calibrationStartMotorButton,
				*calibrateDataButton, *extractDataButton, *clearCalibrateDataButton,
				*clearCommandLogButton, *applySequenceButton, *readAllRegistersButton;
	QButtonGroup *rightMenuButtonGroup;

	QLineEdit 	*motorTargetPositionLineEdit, *graphUpdateIntervalLineEdit, *displayLengthLineEdit,
				*dataGraphSamplesLineEdit, *tempGraphSamplesLineEdit, 
				*acquisitionMotorCurrentPositionLineEdit,
				*calibrationH1MagLineEdit, *calibrationH2MagLineEdit, 
				*calibrationH3MagLineEdit, *calibrationH8MagLineEdit,
				*calibrationH1PhaseLineEdit, *calibrationH2PhaseLineEdit, 
				*calibrationH3PhaseLineEdit, *calibrationH8PhaseLineEdit,
				*calibrationMotorCurrentPositionLineEdit,
				*AFEDIAG0LineEdit, *AFEDIAG1LineEdit, *AFEDIAG2LineEdit;

	QLabel 	*rotationValueLabel, *angleValueLabel, *countValueLabel, *tempValueLabel,
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

	MenuHeaderWidget *header;

	MenuSectionWidget *rightMenuSectionWidget;
	MenuCollapseSection *rotationCollapse, *angleCollapse, *countCollapse, *tempCollapse;
	MenuCombo *m_dataGraphChannelMenuCombo, *m_dataGraphDirectionMenuCombo, *m_tempGraphDirectionMenuCombo, *m_calibrationMotorRampModeMenuCombo,
			  *sequenceTypeMenuCombo, *conversionTypeMenuCombo, *cnvSourceMenuCombo, *convertSynchronizationMenuCombo, *angleFilterMenuCombo, *eighthHarmonicMenuCombo;

	QTabWidget *tabWidget, *calibrationDataGraphTabWidget, *resultDataTabWidget;

	QListWidget *rawDataListWidget;

	QPlainTextEdit *logsPlainTextEdit, *commandLogPlainTextEdit;

	QCheckBox *acquisitionFaultRegisterLEDWidget, *calibrationFaultRegisterLEDWidget, 
			  *DIGIOBusyStatusLED ,*DIGIOCNVStatusLED ,*DIGIOSENTStatusLED ,*DIGIOACALCStatusLED ,*DIGIOFaultStatusLED ,*DIGIOBootloaderStatusLED,
			  *R0StatusLED, *R1StatusLED, *R2StatusLED, *R3StatusLED, *R4StatusLED, *R5StatusLED, *R6StatusLED, *R7StatusLED,
			  *VDDUnderVoltageStatusLED, *VDDOverVoltageStatusLED, *VDRIVEUnderVoltageStatusLED, *VDRIVEOverVoltageStatusLED, *AFEDIAGStatusLED, 
			  *NVMCRCFaultStatusLED, *ECCDoubleBitErrorStatusLED, *OscillatorDriftStatusLED, *CountSensorFalseStateStatusLED, *AngleCrossCheckStatusLED,
			  *TurnCountSensorLevelsStatusLED, *MTDIAGStatusLED, *TurnCounterCrossCheckStatusLED, *RadiusCheckStatusLED, *SequencerWatchdogStatusLED;

	QScrollArea *MTDiagnosticsScrollArea;

	PlotWidget *acquisitionGraphPlotWidget, *angleErrorPlotWidget, *calibrationRawDataPlotWidget, *FFTAngleErrorPlotWidget,
			   *correctedErrorPlotWidget, *postCalibrationRawDataPlotWidget, *FFTCorrectedErrorPlotWidget;
	PlotAxis *acquisitionXPlotAxis, *acquisitionYPlotAxis, *calibrationRawDataXPlotAxis, *calibrationRawDataYPlotAxis, 
			 *angleErrorXPlotAxis, *angleErrorYPlotAxis, *FFTAngleErrorXPlotAxis, *FFTAngleErrorYPlotAxis,
			 *correctedErrorXPlotAxis, *correctedErrorYPlotAxis, *FFTCorrectedErrorXPlotAxis, *FFTCorrectedErrorYPlotAxis,
			 *postCalibrationRawDataXPlotAxis, *postCalibrationRawDataYPlotAxis;
	PlotChannel *acquisitionAnglePlotChannel, *acquisitionABSAnglePlotChannel, *acquisitionTurnCountPlotChannel, *acquisitionTmp0PlotChannel,
				*acquisitionTmp1PlotChannel, *acquisitionSinePlotChannel, *acquisitionCosinePlotChannel, *acquisitionRadiusPlotChannel, 
				*acquisitionSecAnglQPlotChannel, *acquisitionSecAnglIPlotChannel,
	 			*angleErrorPlotChannel, *preCalibrationFFTPhasePlotChannel, *calibrationRawDataPlotChannel, *calibrationSineDataPlotChannel, *calibrationCosineDataPlotChannel,
				*FFTAngleErrorMagnitudeChannel, *FFTAngleErrorPhaseChannel,
				*correctedErrorPlotChannel,
				*postCalibrationRawDataPlotChannel, *postCalibrationSineDataPlotChannel, *postCalibrationCosineDataPlotChannel,
				*FFTCorrectedErrorMagnitudeChannel, *FFTCorrectedErrorPhaseChannel;

	HorizontalSpinBox *motorMaxVelocitySpinBox, *motorAccelTimeSpinBox, *motorMaxDisplacementSpinBox, *motorTargetPositionSpinBox;

	CustomSwitch *calibrationDisplayFormatSwitch, 
		*DIGIO0ENToggleSwitch, *DIGIO0FNCToggleSwitch, 
		*DIGIO1ENToggleSwitch, *DIGIO1FNCToggleSwitch,
		*DIGIO2ENToggleSwitch, *DIGIO2FNCToggleSwitch,
		*DIGIO3ENToggleSwitch, *DIGIO3FNCToggleSwitch,
		*DIGIO4ENToggleSwitch, *DIGIO4FNCToggleSwitch,
		*DIGIO5ENToggleSwitch, *DIGIO5FNCToggleSwitch, 
		*DIGIOALLToggleSwitch;

	RegisterBlockWidget *cnvPageRegisterBlock, *digIORegisterBlock, *faultRegisterBlock, *generalRegisterBlock, *digIOEnRegisterBlock, *angleCkRegisterBlock, *eccDcdeRegisterBlock, *eccDisRegisterBlock, *absAngleRegisterBlock, *angleRegisterBlock, *angleSecRegisterBlock, *sineRegisterBlock, *cosineRegisterBlock, *secAnglIRegisterBlock, *secAnglQRegisterBlock, *radiusRegisterBlock, *diag1RegisterBlock, *diag2RegisterBlock, *tmp0RegisterBlock, *tmp1RegisterBlock, *cnvCntRegisterBlock, *uniqID0RegisterBlock, *uniqID1RegisterBlock, *uniqID2RegisterBlock, *uniqID3RegisterBlock, *h1MagRegisterBlock, *h1PhRegisterBlock, *h2MagRegisterBlock, *h2PhRegisterBlock, *h3MagRegisterBlock, *h3PhRegisterBlock, *h8MagRegisterBlock, *h8PhRegisterBlock;

	QFuture<void> m_deviceStatusThread, m_currentMotorPositionThread,
				  m_acquisitionUIThread, m_acquisitionDataThread, m_acquisitionGraphThread,
				  m_calibrationUIThread, m_utilityUIThread, m_utilityThread;
	QFutureWatcher<void> m_deviceStatusWatcher, m_currentMotorPositionWatcher,
						 m_acquisitionUIWatcher, m_acquisitionDataWatcher, m_acquisitionGraphWatcher,
						 m_calibrationUIWatcher, m_utilityUIWatcher, m_utilityWatcher;

	ToolTemplate* createAcquisitionWidget();
	ToolTemplate* createCalibrationWidget();
	ToolTemplate* createRegistersWidget();
	ToolTemplate* createUtilityWidget();

	void readDeviceProperties();
	void initializeADMT();
	bool readSequence();
	void applySequence();
	bool changeCNVPage(uint32_t page);
	void initializeMotor();
	void startDeviceStatusMonitor();
	void stopDeviceStatusMonitor();
	void getDeviceFaultStatus(int sampleRate);
	void startCurrentMotorPositionMonitor();
	void stopCurrentMotorPositionMonitor();
	void currentMotorPositionTask(int sampleRate);
	bool resetGENERAL();

	#pragma region Acquisition Methods
	bool updateChannelValues();
	void updateCountValue();
	void updateLineEditValues();
	void startAcquisition();
	void stopAcquisition();
	void getAcquisitionSamples(int sampleRate);
	double getAcquisitionParameterValue(const AcquisitionDataKey &key);
	void plotAcquisition(QVector<double>& list, PlotChannel* channel);
	void prependAcquisitionData(const double& data, QVector<double>& list);
	void resetAcquisitionYAxisScale();
	void acquisitionPlotTask(int sampleRate);
	void acquisitionUITask(int sampleRate);
	void startAcquisitionUITask();
	void stopAcquisitionUITask();
	void updateSequenceWidget();
	void applySequenceAndUpdate();
	void updateGeneralSettingEnabled(bool value);
	void connectCheckBoxToAcquisitionGraph(QCheckBox* widget, PlotChannel* channel, AcquisitionDataKey key);
	void GMRReset();
	#pragma endregion

	#pragma region Calibration Methods
	void startCalibrationUITask();
	void stopCalibrationUITask();
	void calibrationUITask(int sampleRate);
	void getCalibrationSamples();
	void startCalibration();
	void stopCalibration();
	void startMotor();
	void startMotorContinuous();
	void postCalibrateData();
	void resetAllCalibrationState();
	void computeSineCosineOfAngles(QVector<double> graphDataList);
	void populateAngleErrorGraphs();
	void populateCorrectedAngleErrorGraphs();
	void flashHarmonicValues();
	void calculateHarmonicValues();
	void updateCalculatedCoeffAngle();
	void updateCalculatedCoeffHex();
	void resetCalculatedCoeffAngle();
	void resetCalculatedCoeffHex();
	void displayCalculatedCoeff();
	void calibrationLogWrite(QString message = "");
	void importCalibrationData();
	void extractCalibrationData();
	void toggleTabSwitching(bool value);
	void canStartMotor(bool value);
	void canCalibrate(bool);
	void toggleMotorControls(bool value);
	void clearCalibrationSamples();
	void clearCalibrationSineCosine();
	void clearPostCalibrationSamples();
	void clearAngleErrorGraphs();
	void clearCorrectedAngleErrorGraphs();
	#pragma endregion

	#pragma region Motor Methods
	bool moveMotorToPosition(double& position, bool validate = true);
	bool resetCurrentPositionToZero();
	void stopMotor();
	int readMotorAttributeValue(ADMTController::MotorAttribute attribute, double& value);
	int writeMotorAttributeValue(ADMTController::MotorAttribute attribute, double value);
	#pragma endregion

	#pragma region Utility Methods
	void startUtilityTask();
	void stopUtilityTask();
	void utilityTask(int sampleRate);
	void toggleUtilityTask(bool run);
	void getDIGIOENRegister();
	void updateDIGIOMonitorUI();
	void updateDIGIOControlUI();
	void getDIAG2Register();
	void getDIAG1Register();
	void getFAULTRegister();
	void toggleDIGIOEN(string DIGIOENName, bool value);
	void toggleMTDiagnostics(int mode);
	void toggleFaultRegisterMode(int mode);
	bool resetDIGIO();
	void clearCommandLog();
	#pragma endregion

	#pragma region Register Methods
	void readAllRegisters();
	void toggleRegisters(int mode);
	#pragma endregion

	#pragma region UI Helper Methods
	void updateLabelValue(QLabel* label, int channelIndex);
	void updateLabelValue(QLabel *label, ADMTController::MotorAttribute attribute);
	bool updateChannelValue(int channelIndex);
	void updateLineEditValue(QLineEdit* lineEdit, double value);
	void toggleWidget(QPushButton *widget, bool value);
	void changeCustomSwitchLabel(CustomSwitch *customSwitch, QString onLabel, QString offLabel);
	void changeStatusLEDColor(MenuControlButton *menuControlButton, QColor color, bool checked = true);
	void changeStatusLEDColor(QCheckBox *widget, const char *colorAttribute);
	void updateFaultStatusLEDColor(MenuControlButton *widget, bool value);
	void toggleStatusLEDColor(QCheckBox *widget, const char *trueAttribute, const char* falseAttribute, bool value);
	MenuControlButton *createStatusLEDWidget(const QString title, QColor color, QWidget *parent = nullptr);
	QCheckBox *createStatusLEDWidget(const QString &text, const char *colorAttribute, bool checked = false, QWidget *parent = nullptr);
	MenuControlButton *createChannelToggleWidget(const QString title, QColor color, QWidget *parent = nullptr);
	#pragma endregion

	#pragma region Connect Methods
	void connectLineEditToNumber(QLineEdit* lineEdit, int& variable, int min, int max);
	void connectLineEditToNumber(QLineEdit* lineEdit, double& variable, QString unit = "");
	void connectLineEditToDouble(QLineEdit* lineEdit, double& variable);
	void connectLineEditToNumberWrite(QLineEdit* lineEdit, double& variable, ADMTController::MotorAttribute attribute);
	void connectMenuComboToNumber(MenuCombo* menuCombo, double& variable);
	void connectMenuComboToNumber(MenuCombo* menuCombo, int& variable);
	void connectLineEditToRPSConversion(QLineEdit* lineEdit, double& vmax);
	void connectLineEditToAMAXConversion(QLineEdit* lineEdit, double& amax);
	void connectRegisterBlockToRegistry(RegisterBlockWidget* widget);
	#pragma endregion

	#pragma region Convert Methods
	double convertRPStoVMAX(double rps);
	double convertVMAXtoRPS(double vmax);
	double convertAccelTimetoAMAX(double accelTime);
	double convertAMAXtoAccelTime(double amax);
	#pragma endregion
};
} // namespace admt
} // namespace scopy
#endif // HARMONICCALIBRATION_H
