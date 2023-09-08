#ifndef ADCINSTRUMENT_H
#define ADCINSTRUMENT_H

#include <QWidget>
#include <adcplugin.h>

namespace scopy {
class AdcInstrument : public QWidget
{
	Q_OBJECT
public:
	AdcInstrument(PlotProxy *proxy, QWidget *parent = nullptr);
};
}
#endif // ADCINSTRUMENT_H
