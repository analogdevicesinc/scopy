/*
 * Copyright (c) 2025 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef HARMONICCALIBRATION_H
#define HARMONICCALIBRATION_H

#include "scopy-admt_export.h"

#include <cstdint>

#include <QButtonGroup>
#include <QCheckBox>
#include <QDoubleValidator>
#include <QFileDialog>
#include <QFuture>
#include <QFutureWatcher>
#include <QGridLayout>
#include <QIcon>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMap>
#include <QMetaObject>
#include <QPen>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSpacerItem>
#include <QString>
#include <QTabWidget>
#include <QThread>
#include <QTimer>
#include <QVBoxLayout>
#include <QVariant>
#include <QWidget>
#include <QtMath>

#include <admtcontroller.h>
#include <admtplugin.h>
#include <customSwitch.h>
#include <filemanager.h>
#include <iio.h>
#include <iioutil/connectionprovider.h>
#include <menucollapsesection.h>
#include <menucombo.h>
#include <menucontrolbutton.h>
#include <menuheader.h>
#include <menusectionwidget.h>
#include <plotaxis.h>
#include <plotwidget.h>
#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>
#include <pluginbase/statusbarmanager.h>
#include <stylehelper.h>
#include <toolbuttons.h>
#include <tooltemplate.h>
#include <widgets/registerblockwidget.h>

enum SensorData
{
	ABSANGLE,
	ANGLE,
	ANGLESEC,
	SINE,
	COSINE,
	SECANGLI,
	SECANGLQ,
	RADIUS,
	DIAG1,
	DIAG2,
	TMP0,
	TMP1,
	CNVCNT
};

namespace scopy {
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
	void updateAcquisitionData(QMap<SensorData, double> sensorDataMap);
	void updateAcquisitionGraph();
	void updateCalibrationGraph();
	void calibrationLogWrite(QString message = "");
	void commandLogWrite(QString message = "");
	void updateFaultStatus(bool value);
	void updateMotorPosition(double position);
	void updateDIGIOUI(quint16 registerValue);
	void updateFaultRegisterUI(quint16 registerValue);
	void updateMTDiagnosticRegisterUI(quint16 registerValue);
	void updateMTDiagnosticsUI(quint16 registerValue);
Q_SIGNALS:
	void runningChanged(bool);
	void acquisitionDataChanged(QMap<SensorData, double> sensorDataMap);
	void acquisitionGraphChanged();
	void calibrationGraphChanged();
	void canCalibrateChanged(bool);
	void updateUtilityUI();
	void calibrationLogWriteSignal(QString message);
	void commandLogWriteSignal(QString message);
	void updateFaultStatusSignal(bool value);
	void motorPositionChanged(double position);
	void DIGIORegisterChanged(quint16 registerValue);
	void FaultRegisterChanged(quint16 registerValue);
	void DIAG1RegisterChanged(quint16 registerValue);
	void DIAG2RegisterChanged(quint16 registerValue);

private:
	ADMTController *m_admtController;
	iio_context *m_ctx;
	bool m_running, isDebug;
	ToolTemplate *tool;
	GearBtn *settingsButton;
	InfoBtn *infoButton;
	RunBtn *runButton;

	const char *rotationChannelName, *angleChannelName, *countChannelName, *temperatureChannelName;

	double rotation, angle, count, temp = 0.0, motor_rpm, amax, rotate_vmax, dmax, disable, target_pos, current_pos,
				       ramp_mode, afeDiag0, afeDiag1, afeDiag2;

	QPen channel0Pen, channel1Pen, channel2Pen, channel3Pen, channel4Pen, channel5Pen, channel6Pen, channel7Pen;

	QMap<string, string> deviceRegisterMap;
	QMap<string, int> GENERALRegisterMap;
	QMap<string, bool> DIGIOENRegisterMap, FAULTRegisterMap, DIAG1RegisterMap;
	QMap<string, double> DIAG2RegisterMap, DIAG1AFERegisterMap;

	QVector<double> acquisitionAngleList, acquisitionABSAngleList, acquisitionTmp0List, acquisitionTmp1List,
		acquisitionSineList, acquisitionCosineList, acquisitionRadiusList, acquisitionAngleSecList,
		acquisitionSecAnglIList, acquisitionSecAnglQList, graphDataList, graphPostDataList;

	QPushButton *openLastMenuButton, *calibrationStartMotorButton, *calibrateDataButton, *importSamplesButton,
		*extractDataButton, *clearCalibrateDataButton, *clearCommandLogButton, *applySequenceButton,
		*readAllRegistersButton;

	QButtonGroup *rightMenuButtonGroup;

	QLineEdit *acquisitionMotorRPMLineEdit, *calibrationCycleCountLineEdit, *calibrationTotalSamplesLineEdit,
		*calibrationMotorRPMLineEdit, *motorTargetPositionLineEdit, *displayLengthLineEdit,
		*dataGraphSamplesLineEdit, *tempGraphSamplesLineEdit, *acquisitionMotorCurrentPositionLineEdit,
		*calibrationH1MagLineEdit, *calibrationH2MagLineEdit, *calibrationH3MagLineEdit,
		*calibrationH8MagLineEdit, *calibrationH1PhaseLineEdit, *calibrationH2PhaseLineEdit,
		*calibrationH3PhaseLineEdit, *calibrationH8PhaseLineEdit, *calibrationMotorCurrentPositionLineEdit,
		*AFEDIAG0LineEdit, *AFEDIAG1LineEdit, *AFEDIAG2LineEdit;

	QLabel *rawAngleValueLabel, *rotationValueLabel, *angleValueLabel, *countValueLabel, *tempValueLabel,
		*motorAmaxValueLabel, *motorRotateVmaxValueLabel, *motorDmaxValueLabel, *motorDisableValueLabel,
		*motorTargetPosValueLabel, *motorCurrentPosValueLabel, *motorRampModeValueLabel, *calibrationH1MagLabel,
		*calibrationH1PhaseLabel, *calibrationH2MagLabel, *calibrationH2PhaseLabel, *calibrationH3MagLabel,
		*calibrationH3PhaseLabel, *calibrationH8MagLabel, *calibrationH8PhaseLabel;

	MenuHeaderWidget *header;

	MenuSectionWidget *rightMenuSectionWidget;
	MenuCollapseSection *rotationCollapse, *angleCollapse, *countCollapse, *tempCollapse;
	MenuCombo *m_dataGraphChannelMenuCombo, *m_dataGraphDirectionMenuCombo, *m_tempGraphDirectionMenuCombo,
		*m_calibrationMotorRampModeMenuCombo, *sequenceModeMenuCombo, *conversionModeMenuCombo,
		*cnvSourceMenuCombo, *convertSynchronizationMenuCombo, *angleFilterMenuCombo, *eighthHarmonicMenuCombo,
		*calibrationModeMenuCombo;

	QTabWidget *tabWidget, *calibrationDataGraphTabWidget, *resultDataTabWidget;

	QListWidget *rawDataListWidget;

	QPlainTextEdit *logsPlainTextEdit, *commandLogPlainTextEdit;

	QCheckBox *angleCheckBox, *absAngleCheckBox, *temp0CheckBox, *sineCheckBox, *cosineCheckBox, *radiusCheckBox,
		*angleSecCheckBox, *secAnglQCheckBox, *secAnglICheckBox, *temp1CheckBox,
		*acquisitionFaultRegisterLEDWidget, *calibrationFaultRegisterLEDWidget, *DIGIOBusyStatusLED,
		*DIGIOCNVStatusLED, *DIGIOSENTStatusLED, *DIGIOACALCStatusLED, *DIGIOFaultStatusLED,
		*DIGIOBootloaderStatusLED, *R0StatusLED, *R1StatusLED, *R2StatusLED, *R3StatusLED, *R4StatusLED,
		*R5StatusLED, *R6StatusLED, *R7StatusLED, *VDDUnderVoltageStatusLED, *VDDOverVoltageStatusLED,
		*VDRIVEUnderVoltageStatusLED, *VDRIVEOverVoltageStatusLED, *AFEDIAGStatusLED, *NVMCRCFaultStatusLED,
		*ECCDoubleBitErrorStatusLED, *OscillatorDriftStatusLED, *CountSensorFalseStateStatusLED,
		*AngleCrossCheckStatusLED, *TurnCountSensorLevelsStatusLED, *MTDIAGStatusLED,
		*TurnCounterCrossCheckStatusLED, *RadiusCheckStatusLED, *SequencerWatchdogStatusLED;

	QScrollArea *MTDiagnosticsScrollArea;

	QWidget *acquisitionGraphChannelWidget;

	QGridLayout *acquisitionGraphChannelGridLayout;

	PlotWidget *acquisitionGraphPlotWidget, *angleErrorPlotWidget, *calibrationRawDataPlotWidget,
		*FFTAngleErrorMagnitudePlotWidget, *FFTAngleErrorPhasePlotWidget, *correctedErrorPlotWidget,
		*postCalibrationRawDataPlotWidget, *FFTCorrectedErrorMagnitudePlotWidget,
		*FFTCorrectedErrorPhasePlotWidget;
	PlotAxis *acquisitionXPlotAxis, *acquisitionYPlotAxis, *calibrationRawDataXPlotAxis,
		*calibrationRawDataYPlotAxis, *angleErrorXPlotAxis, *angleErrorYPlotAxis,
		*FFTAngleErrorMagnitudeXPlotAxis, *FFTAngleErrorMagnitudeYPlotAxis, *FFTAngleErrorPhaseXPlotAxis,
		*FFTAngleErrorPhaseYPlotAxis, *correctedErrorXPlotAxis, *correctedErrorYPlotAxis,
		*FFTCorrectedErrorMagnitudeXPlotAxis, *FFTCorrectedErrorMagnitudeYPlotAxis,
		*FFTCorrectedErrorPhaseXPlotAxis, *FFTCorrectedErrorPhaseYPlotAxis, *postCalibrationRawDataXPlotAxis,
		*postCalibrationRawDataYPlotAxis;
	PlotChannel *acquisitionAnglePlotChannel, *acquisitionABSAnglePlotChannel, *acquisitionTmp0PlotChannel,
		*acquisitionTmp1PlotChannel, *acquisitionSinePlotChannel, *acquisitionCosinePlotChannel,
		*acquisitionRadiusPlotChannel, *acquisitionSecAnglQPlotChannel, *acquisitionSecAnglIPlotChannel,
		*acquisitionAngleSecPlotChannel, *angleErrorPlotChannel, *preCalibrationFFTPhasePlotChannel,
		*calibrationRawDataPlotChannel, *calibrationSineDataPlotChannel, *calibrationCosineDataPlotChannel,
		*FFTAngleErrorMagnitudeChannel, *FFTAngleErrorPhaseChannel, *correctedErrorPlotChannel,
		*postCalibrationRawDataPlotChannel, *postCalibrationSineDataPlotChannel,
		*postCalibrationCosineDataPlotChannel, *FFTCorrectedErrorMagnitudeChannel,
		*FFTCorrectedErrorPhaseChannel;

	CustomSwitch *acquisitionMotorDirectionSwitch, *calibrationMotorDirectionSwitch,
		*calibrationDisplayFormatSwitch, *DIGIO0ENToggleSwitch, *DIGIO0FNCToggleSwitch, *DIGIO1ENToggleSwitch,
		*DIGIO1FNCToggleSwitch, *DIGIO2ENToggleSwitch, *DIGIO2FNCToggleSwitch, *DIGIO3ENToggleSwitch,
		*DIGIO3FNCToggleSwitch, *DIGIO4ENToggleSwitch, *DIGIO4FNCToggleSwitch, *DIGIO5ENToggleSwitch,
		*DIGIO5FNCToggleSwitch, *DIGIOALLToggleSwitch;

	RegisterBlockWidget *cnvPageRegisterBlock, *digIORegisterBlock, *faultRegisterBlock, *generalRegisterBlock,
		*digIOEnRegisterBlock, *angleCkRegisterBlock, *eccCdeRegisterBlock, *eccDisRegisterBlock,
		*absAngleRegisterBlock, *angleRegisterBlock, *angleSecRegisterBlock, *sineRegisterBlock,
		*cosineRegisterBlock, *secAnglIRegisterBlock, *secAnglQRegisterBlock, *radiusRegisterBlock,
		*diag1RegisterBlock, *diag2RegisterBlock, *tmp0RegisterBlock, *tmp1RegisterBlock, *cnvCntRegisterBlock,
		*uniqID0RegisterBlock, *uniqID1RegisterBlock, *uniqID2RegisterBlock, *uniqID3RegisterBlock,
		*h1MagRegisterBlock, *h1PhRegisterBlock, *h2MagRegisterBlock, *h2PhRegisterBlock, *h3MagRegisterBlock,
		*h3PhRegisterBlock, *h8MagRegisterBlock, *h8PhRegisterBlock;

	QFuture<void> m_deviceStatusThread, m_currentMotorPositionThread, m_acquisitionUIThread,
		m_acquisitionDataThread, m_acquisitionGraphThread, m_calibrationUIThread, m_calibrationStreamThread,
		m_calibrationWaitVelocityThread, m_calibrationContinuousThread, m_resetMotorToZeroThread,
		m_utilityUIThread, m_utilityThread;
	QFutureWatcher<void> m_deviceStatusWatcher, m_currentMotorPositionWatcher, m_acquisitionUIWatcher,
		m_acquisitionDataWatcher, m_acquisitionGraphWatcher, m_calibrationUIWatcher, m_calibrationStreamWatcher,
		m_calibrationWaitVelocityWatcher, m_calibrationContinuousWatcher, m_resetMotorToZeroWatcher,
		m_utilityUIWatcher, m_utilityWatcher;

	ToolTemplate *createAcquisitionWidget();
	ToolTemplate *createCalibrationWidget();
	ToolTemplate *createRegistersWidget();
	ToolTemplate *createUtilityWidget();

	bool readDeviceProperties();
	void initializeADMT();
	bool readSequence();
	bool writeSequence(QMap<string, int> settings);
	void applySequence();
	bool changeCNVPage(uint8_t page);
	bool convertStart(bool start);
	bool convertRestart();
	void initializeMotor();
	void startDeviceStatusMonitor();
	void stopDeviceStatusMonitor();
	void getDeviceFaultStatus(int sampleRate);
	void currentMotorPositionTask(int sampleRate);
	bool resetGENERAL();
	void stopTasks();

#pragma region Acquisition Methods
	bool updateChannelValues();
	bool updateSensorValue(ADMTController::SensorRegister sensor);
	void updateLineEditValues();
	void startAcquisition();
	void stopAcquisition();
	void restartAcquisition();
	void updateAcquisitionMotorRPM();
	void updateAcquisitionMotorRotationDirection();
	void getAcquisitionSamples(QMap<SensorData, bool> dataMap);
	double getSensorDataAcquisitionValue(const ADMTController::SensorRegister &key);
	void plotAcquisition(QVector<double> list, PlotChannel *channel);
	void appendAcquisitionData(const double &data, QVector<double> &list);
	void resetAcquisitionYAxisScale();
	void updateSequenceWidget();
	void updateCapturedDataCheckBoxes();
	void applySequenceAndUpdate();
	void toggleAcquisitionControls(bool value);
	void connectCheckBoxToAcquisitionGraph(QCheckBox *widget, PlotChannel *channel, SensorData key);
	void GMRReset();
#pragma endregion

#pragma region Calibration Methods
	void startCalibrationUITask();
	void stopCalibrationUITask();
	void calibrationUITask(int sampleRate);
	void updateCalibrationMotorRPM();
	void updateCalibrationMotorRotationDirection();
	void getCalibrationSamples();
	void startCalibration();
	void stopCalibration();
	void startContinuousCalibration();
	void stopContinuousCalibration();
	void startCalibrationStreamThread();
	void stopCalibrationStreamThread();
	void startWaitForVelocityReachedThread(int mode);
	void stopWaitForVelocityReachedThread();
	void waitForVelocityReached(int mode, int sampleRate);
	int calculateContinuousCalibrationSampleRate(double motorRPS, int samplesPerCycle);
	void configureCalibrationSequenceSettings(int conversionMode);
	void startOneShotCalibration();
	void postCalibrateData();
	void resetAllCalibrationState();
	void computeSineCosineOfAngles(QVector<double> graphDataList);
	void populateAngleErrorGraphs();
	void populateCorrectedAngleErrorGraphs();
	bool clearHarmonicRegisters();
	bool flashHarmonicValues();
	void calculateHarmonicValues();
	void updateCalculatedCoeffAngle();
	void updateCalculatedCoeffHex();
	void resetCalculatedCoeffAngle();
	void resetCalculatedCoeffHex();
	void displayCalculatedCoeff();
	void importCalibrationData();
	void extractCalibrationData();
	void toggleTabSwitching(bool value);
	void toggleCalibrationButtonState(int state);
	void canStartMotor(bool value);
	void canCalibrate(bool);
	void toggleCalibrationControls(bool value);
	void clearCalibrationSamples();
	void clearCalibrationSineCosine();
	void clearPostCalibrationSamples();
	void clearAngleErrorGraphs();
	void clearFFTAngleErrorGraphs();
#pragma endregion

#pragma region Motor Methods
	bool moveMotorToPosition(double &position, bool validate = true);
	void moveMotorContinuous();
	bool resetCurrentPositionToZero();
	void stopMotor();
	int readMotorAttributeValue(ADMTController::MotorAttribute attribute, double &value);
	int writeMotorAttributeValue(ADMTController::MotorAttribute attribute, double value);
	int readMotorRegisterValue(uint32_t address, uint32_t *value);
	void setRampMode(bool motorRotationClockwise);
	void getRampMode();
	void startResetMotorToZero(bool enableWatcher = false);
	void stopResetMotorToZero();
	void resetMotorToZero();
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
	bool toggleDIGIOEN(string DIGIOENName, bool value);
	void toggleMTDiagnostics(int mode);
	void toggleFaultRegisterMode(int mode);
	bool resetDIGIO();
	bool disableECC(bool disable);
	void clearCommandLog();
#pragma endregion

#pragma region Register Methods
	void readAllRegisters();
	void toggleRegisters(int mode);
#pragma endregion

#pragma region UI Helper Methods
	bool updateChannelValue(int channelIndex);
	void updateLineEditValue(QLineEdit *lineEdit, double value);
	void toggleWidget(QPushButton *widget, bool value);
	void changeCustomSwitchLabel(CustomSwitch *customSwitch, QString onLabel, QString offLabel);
	QCheckBox *createStatusLEDWidget(const QString &text, QVariant variant = true, bool checked = false,
					 QWidget *parent = nullptr);
	void configureCoeffRow(QWidget *container, QHBoxLayout *layout, QLabel *hLabel, QLabel *hMagLabel,
			       QLabel *hPhaseLabel);
	void initializeChannelColors();
#pragma endregion

#pragma region Connect Methods
	void connectLineEditToNumber(QLineEdit *lineEdit, int &variable, int min, int max);
	void connectLineEditToNumber(QLineEdit *lineEdit, double &variable, QString unit = "");
	void connectLineEditToDouble(QLineEdit *lineEdit, double &variable);
	void connectLineEditToNumberWrite(QLineEdit *lineEdit, double &variable,
					  ADMTController::MotorAttribute attribute);
	void connectLineEditToMotorTurnCount(QLineEdit *lineEdit, int &variable, int min, int max);
	void connectMenuComboToNumber(MenuCombo *menuCombo, double &variable);
	void connectMenuComboToNumber(MenuCombo *menuCombo, int &variable);
	void connectLineEditToRPSConversion(QLineEdit *lineEdit, double &vmax);
	void connectLineEditToAMAXConversion(QLineEdit *lineEdit, double &amax);
	void connectRegisterBlockToRegistry(RegisterBlockWidget *widget);
	void connectLineEditToRPM(QLineEdit *lineEdit, double &variable, double min, double max);
#pragma endregion

#pragma region Convert Methods
	double convertRPStoVMAX(double rps);
	double convertVMAXtoRPS(double vmax);
	double convertAccelTimetoAMAX(double accelTime);
	double convertAMAXtoAccelTime(double amax);
	double convertRPMtoRPS(double rpm);
	double convertRPStoRPM(double rps);
#pragma endregion

#pragma region Debug Methods
	QString readRegmapDumpAttributeValue();
#pragma endregion
};
} // namespace admt
} // namespace scopy
#endif // HARMONICCALIBRATION_H
