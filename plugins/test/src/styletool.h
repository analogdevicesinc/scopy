#ifndef STYLETOOL_H
#define STYLETOOL_H

#include "gui/tooltemplate.h"

#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QWidget>

#include <scopy-testplugin_export.h>

using namespace scopy;

class SCOPY_TESTPLUGIN_EXPORT StyleTool : public QWidget
{
public:
	StyleTool(QWidget *parent = nullptr);
private:
	QWidget *buildStylePage();
	QWidget *buildPageColors();
};
//};
#endif // STYLETOOL_H
