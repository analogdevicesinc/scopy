#ifndef HARMONICCALIBRATION_H
#define HARMONICCALIBRATION_H

#include "scopy-admt_export.h"

#include <QBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QString>
#include <QVBoxLayout>
#include <QScrollArea>

#include <admtplugin.h>
#include <iio.h>
#include <toolbuttons.h>
#include <tooltemplate.h>
#include <menuheader.h>
#include <menusectionwidget.h>
#include <menucollapsesection.h>

namespace scopy {
class SCOPY_ADMT_EXPORT HarmonicCalibration : public QWidget
{
	Q_OBJECT
public:
	HarmonicCalibration(PlotProxy *proxy, QWidget *parent = nullptr);
	~HarmonicCalibration();
private:
	void getRotationData();
	void getCountData();
	void getAngleData();
	QStringList getDeviceList(iio_context *context);

	ToolTemplate *tool;
	GearBtn *settingsButton;
	InfoBtn *infoButton;
	RunBtn *runButton;
	QPushButton *getRotationButton, *getCountButton, *getAngleButton;
	QLineEdit *rotationLineEdit, *countLineEdit, *angleLineEdit;
	MenuHeaderWidget *header;
	QWidget *leftWidget, *leftBody;
	QVBoxLayout *leftLayout, *leftBodyLayout;
	MenuSectionWidget *rotationSection, *countSection, *angleSection;
	MenuCollapseSection *rotationCollapse, *countCollapse, *angleCollapse;
	QScrollArea *scrollArea;

	QPushButton *openLastMenuBtn;
	PlotProxy *proxy;
	GRTimePlotAddon *plotAddon;
	GRTimePlotAddonSettings *plotAddonSettings;
	QButtonGroup *rightMenuBtnGrp;
	int uuid = 0;
};
} // namespace scopy::admt
#endif // HARMONICCALIBRATION_H
