#ifndef TESTTOOL_H
#define TESTTOOL_H

#include "gui/mapstackedwidget.h"
#include "gui/tooltemplate.h"

#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QWidget>

#include <scopy-testplugin_export.h>

using namespace scopy;

class SCOPY_TESTPLUGIN_EXPORT TestTool : public QWidget
{
public:
	TestTool(QWidget *parent = nullptr);
	void initData();
public Q_SLOTS:
	void acquireData();

private:
	int testDataSize = 128;
	double amplitude = 0.5;
	double phase = 0;
	std::vector<double> y2Volt;
	std::vector<double> y1Volt;
	std::vector<double> xTime;

	ToolTemplate *tool;
	QPushButton *btn1, *btn2, *btn3;
	QWidget *createMenu(QWidget *parent);
};
//};
#endif // TESTTOOL_H
