#ifndef TOOLCOMPONENT_H
#define TOOLCOMPONENT_H

#include "gr-util/grtimeplotaddon.h"
#include "scopy-adcplugin_export.h"
#include <QPen>
#include <gui/tooltemplate.h>
#include <gui/stylehelper.h>
#include <QObject>
#include "adcacquisitionmanager.h"


namespace scopy {
namespace adc {

class PlotProxy;
class ADCTimeInstrument;

class SCOPY_ADCPLUGIN_EXPORT ToolComponent
{
public:
	virtual QString getName() = 0;

	virtual void onStart() {}
	virtual void onStop() {}
	virtual void onInit() {}
	virtual void onDeinit() {}
private:
	PlotProxy* m_proxy;
};

class SCOPY_ADCPLUGIN_EXPORT AcqNodeChannelAware {
public:
	virtual void addChannel(AcqTreeNode *c) = 0;
	virtual void removeChannel(AcqTreeNode *c) = 0;
};

class SCOPY_ADCPLUGIN_EXPORT PlotProxy
{
public:
	virtual void init() = 0;
	virtual void deinit() = 0;
	virtual void onStart() = 0;
	virtual void onStop() = 0;

	virtual QWidget* getInstrument() = 0;
	virtual void setInstrument(QWidget*) = 0;

};





} // namespace adc
} // namespace scopy

#endif // TOOLCOMPONENT_H
