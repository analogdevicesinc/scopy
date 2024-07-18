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

namespace scopy::admt {

class HarmonicCalibration : public QWidget
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
Q_SIGNALS:
	void runningChanged(bool);
private:
	ADMTController *m_admtController;
	iio_context *m_ctx;
	bool m_running;
	ToolTemplate *tool;
	GearBtn *settingsButton;
	InfoBtn *infoButton;
	RunBtn *runButton;

	double rotation, angle, count, temp;

	QPushButton *openLastMenuButton;
	QButtonGroup *rightMenuButtonGroup;

	QLineEdit *sampleRateLineEdit, *bufferSizeLineEdit, *dataGraphSamplesLineEdit, *tempGraphSamplesLineEdit, 
			*calibrationH1MagLineEdit, *calibrationH2MagLineEdit, *calibrationH3MagLineEdit, *calibrationH8MagLineEdit,
			*calibrationH1PhaseLineEdit, *calibrationH2PhaseLineEdit, *calibrationH3PhaseLineEdit, *calibrationH8PhaseLineEdit;
	QLabel *rotationValueLabel, *angleValueLabel, *countValueLabel, *tempValueLabel, *calibrationAngleLabel;

	Sismograph *dataGraph, *tempGraph;

	MenuHeaderWidget *header;

	MenuSectionWidget *rightMenuSectionWidget;
	MenuCollapseSection *rotationCollapse, *angleCollapse, *countCollapse, *tempCollapse;
	MenuCombo *m_dataGraphChannelMenuCombo, *m_dataGraphDirectionMenuCombo, *m_tempGraphDirectionMenuCombo;

	QTabWidget *tabWidget;

	QListWidget *rawDataListWidget;

	QPlainTextEdit *logsPlainTextEdit;

	void updateChannelValues();
	void updateLineEditValues();
	void updateGeneralSettingEnabled(bool value);
	void connectLineEditToNumber(QLineEdit* lineEdit, int& variable);
	void connectLineEditToGraphSamples(QLineEdit* lineEdit, int& variable, Sismograph* graph);
	void connectMenuComboToGraphDirection(MenuCombo* menuCombo, Sismograph* graph);
	void changeGraphColorByChannelName(Sismograph* graph, const char* channelName);
	void connectMenuComboToGraphChannel(MenuCombo* menuCombo, Sismograph* graph);
	ToolTemplate* createCalibrationWidget();
	void updateLabelValue(QLabel* label, int channelIndex);
	void updateChannelValue(int channelIndex);
	void calibrationTask();
	void addAngleToRawDataList();
	void removeLastItemFromRawDataList();
	void calibrateData();
	void registerCalibrationData();
	void extractCalibrationData();
	void importCalibrationData();
	void calibrationLogWrite(QString message);
	void calibrationLogWriteLn(QString message);

	QTimer *timer, *calibrationTimer;

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
