#ifndef IMUANALYZERSETTINGS_H
#define IMUANALYZERSETTINGS_H

#include <QWidget>
#include <QLineEdit>
#include <QObject>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <toolbuttons.h>
#include <tooltemplate.h>
#include <edittextmenuheader.h>
#include <menucombo.h>
#include <menuonoffswitch.h>
#include <menucontrolbutton.h>
#include <menuheader.h>
#include <menucollapsesection.h>
#include <menusectionwidget.h>
#include "2Drenderer.hpp"
#include "3Drenderer.hpp"
#include "menuspinbox.h"

#include <QColorDialog>
#include <QPalette>

#include "scopy-imuanalyzer_export.h"

namespace scopy{

class SCOPY_IMUANALYZER_EXPORT ImuAnalyzerSettings : public QWidget
{
	Q_OBJECT
public:
	explicit ImuAnalyzerSettings(SceneRenderer *scRend, BubbleLevelRenderer *blRend, QWidget *parent);

signals:
	void updateCubeColor(QColor color);
	void updatePlaneColor(QColor color);
	void updateDisplayPoints(QString displayP);
};
}
#endif // IMUANALYZERSETTINGS_H
