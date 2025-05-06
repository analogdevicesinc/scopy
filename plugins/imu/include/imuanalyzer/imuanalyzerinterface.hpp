#ifndef IMUANALYZERINTERFACE_H
#define IMUANALYZERINTERFACE_H

#include "scopy-imuanalyzer_export.h"
#include "3Drenderer.hpp"
#include "2Drenderer.hpp"
#include "imuanalyzersettings.hpp"

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

namespace scopy {

class SCOPY_IMUANALYZER_EXPORT IMUAnalyzerInterface : public QWidget {
	Q_OBJECT
public:
    IMUAnalyzerInterface(QWidget *parent = nullptr);
    ~IMUAnalyzerInterface();

Q_SIGNALS:
	void generateRot(rotation rot);

public:
	void generateRotation();

private:
    ToolTemplate *m_tool;

    InfoBtn *m_infoBtn;
    RunBtn *m_runBtn;
    QPushButton *m_rstPos;
    QPushButton *m_rstView;
    GearBtn *m_gearBtn;

    SceneRenderer *m_sceneRender;

    ImuAnalyzerSettings *m_settingsPanel;
    rotation m_rot;

    std::thread t;
};
}

#endif //IMUANALYZERINTERFACE_H
