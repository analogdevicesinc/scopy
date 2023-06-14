#ifndef ADCINSTRUMENT_H
#define ADCINSTRUMENT_H

#include <QWidget>
#include <adcplugin.h>
#include "gui/tooltemplate.h"
#include <QPushButton>

namespace scopy {
class AdcInstrument : public QWidget
{
	Q_OBJECT
public:
	AdcInstrument(PlotProxy *proxy, QWidget *parent = nullptr);
public Q_SLOTS:
	void run(bool);
private:
	ToolTemplate *tool;
	PlotProxy* proxy;
	QPushButton *openLastMenuBtn;
};
}
#endif // ADCINSTRUMENT_H
