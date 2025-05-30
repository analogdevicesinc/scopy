#ifndef IMUANALYZERINTERFACE_H
#define IMUANALYZERINTERFACE_H

#include "scopy-imuanalyzer_export.h"
#include "scenerenderer.hpp"
#include "bubblelevelrenderer.hpp"
#include "imuanalyzersettings.hpp"
#include "datavisualizer.hpp"

#include <QLineEdit>
#include <QObject>

#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QWidget>
#include <toolbuttons.h>
#include <tooltemplate.h>

#include <menucontrolbutton.h>
#include <printplotmanager.h>
#include <toolbuttons.h>
#include <tooltemplate.h>

#include <iio.h>
#include <iioutil/connectionprovider.h>

#include <hoverwidget.h>
#include <QStackedLayout>
#include <measurementpanel.h>
#include <math.h>
#include <chrono>

namespace scopy {

class SCOPY_IMUANALYZER_EXPORT IMUAnalyzerInterface : public QWidget
{
	Q_OBJECT
public:
	IMUAnalyzerInterface(QString uri, QWidget *parent = nullptr);
	~IMUAnalyzerInterface();

Q_SIGNALS:
	void generateRot(data3P rot);
	void updateValues(data3P rot, data3P pos, float temp);

public:
	void generateRotation();
	void initIIODevice();

private:
	ToolTemplate *m_tool;

	InfoBtn *m_infoBtn;
	RunBtn *m_runBtn;
	MenuControlButton *m_rstView;
	MenuControlButton *m_measureBtn;
	GearBtn *m_gearBtn;

	SceneRenderer *m_sceneRender;
	BubbleLevelRenderer *m_bubbleLevelRenderer;

	ImuAnalyzerSettings *m_settingsPanel;
	DataVisualizer *m_dataV;
	data3P m_rot = {0.0f, 0.0f, 0.0f};
	data3P m_dist = {0.0f, 0.0f, 0.0f};

	std::thread t;
	QString m_uri;

	iio_device *m_device;
};
} // namespace scopy

#endif // IMUANALYZERINTERFACE_H
