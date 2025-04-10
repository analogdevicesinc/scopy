#ifndef IMUANALYZERINTERFACE_H
#define IMUANALYZERINTERFACE_H

#include "scopy-imuanalyzer_export.h"
#include "3Drenderer.hpp"

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
    //~IMUAnalyzerInterface();

private:
    ToolTemplate *m_tool;

    InfoBtn *m_infoBtn;
    RunBtn *m_runBtn;
    QPushButton *m_rstPos;
    QPushButton *m_rstView;

    SceneRenderer *m_sceneRender;
};
}

#endif //IMUANALYZERINTERFACE_H
