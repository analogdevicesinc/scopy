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
#include "bubblelevelrenderer.hpp"
#include "scenerenderer.hpp"
#include "menuspinbox.h"

#include <QColorDialog>
#include <QPalette>

#include "scopy-imuanalyzer_export.h"
#include "iio.h"
#include "iio-widgets/iiowidget.h"
#include "iio-widgets/iiowidgetbuilder.h"
#include "iio-widgets/iiowidgetdata.h"

namespace scopy{

class SCOPY_IMUANALYZER_EXPORT ImuAnalyzerSettings : public QWidget
{
	Q_OBJECT
public:
	explicit ImuAnalyzerSettings(SceneRenderer *scRend, BubbleLevelRenderer *blRend, iio_device *device, QWidget *parent = nullptr);

signals:
	void updateCubeColor(QColor color);
	void updatePlaneColor(QColor color);
	void updateDisplayPoints(QString displayP);

private:
	iio_device *m_device;
};
}
#endif // IMUANALYZERSETTINGS_H
