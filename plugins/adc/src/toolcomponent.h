#ifndef TOOLCOMPONENT_H
#define TOOLCOMPONENT_H

#include "scopy-adcplugin_export.h"
#include <QPen>
#include <gui/tooltemplate.h>
#include <gui/stylehelper.h>
#include <QObject>
#include "adcacquisitionmanager.h"

namespace scopy {
namespace adc {

class PlotProxy;
class ADCInstrument;


class SCOPY_ADCPLUGIN_EXPORT DataProvider
{
public:
	virtual void setSingleShot(bool) = 0;
	virtual size_t updateData() = 0;
	virtual bool finished() = 0;
	virtual void setCurveData(bool raw = false) = 0;
};

class SCOPY_ADCPLUGIN_EXPORT ToolComponent
{
public:
	virtual QString name() const { return m_name; };
	virtual int priority() const { return m_priority; };
	virtual void onStart(){};
	virtual void onStop(){};
	virtual void onInit(){};
	virtual void onDeinit(){};

	virtual void enable() {m_enabled = true;}
	virtual void disable() {m_enabled = false;}
	QStringList *category() {
		return &m_category;
	}


	bool enabled() const {
		return m_enabled;
	}


protected:
	QString m_name;
	bool m_enabled;
	QStringList m_category;
	int m_priority = 0;
};

class SCOPY_ADCPLUGIN_EXPORT MetaComponent : public ToolComponent
{
public:
	virtual void addComponent(ToolComponent *c)
	{
		m_components.append(c);
		// std::sort(m_components.first(), m_components.last(), [](const ToolComponent &a, const ToolComponent
		// &b){ return a.priority() > b.priority(); });
		c->onInit();
	};

	virtual void removeComponent(ToolComponent *c)
	{
		c->onDeinit();
		m_components.removeAll(c);
	}

	virtual QList<ToolComponent *> components() const { return m_components; }
	virtual void onStart()
	{
		auto cm = components();
		for(auto c : cm) {
			if(c->enabled())
				c->onStart();
		}
	}

	virtual void onStop()
	{
		auto cm = components();
		for(auto c : cm) {
			c->onStop();
		}
	}
	virtual void onInit()
	{
		/*auto cm = components();
		for(auto c : cm) {
			c->onInit();
		}*/
	}
	virtual void onDeinit()
	{
		/*auto cm = components();
		for(auto c : cm) {
			c->onDeinit();
		}*/
	}

protected:
	QList<ToolComponent *> m_components;
};

class SCOPY_ADCPLUGIN_EXPORT AcqNodeChannelAware
{
public:
	virtual void addChannel(AcqTreeNode *c) = 0;
	virtual void removeChannel(AcqTreeNode *c) = 0;
};

class SCOPY_ADCPLUGIN_EXPORT PlotProxy : public MetaComponent
{
public:
	virtual void init() = 0;
	virtual void deinit() = 0;

	virtual QWidget *getInstrument() = 0;
	virtual void setInstrument(QWidget *) = 0;
};

class SCOPY_ADCPLUGIN_EXPORT ChannelIdProvider : public QObject
{
	Q_OBJECT
public:
	ChannelIdProvider(QObject *parent)
		: QObject(parent)
	{
		idx = 0;
	}
	virtual ~ChannelIdProvider() {}

	int next() { return idx++; }
	QPen pen(int idx) { return QPen(StyleHelper::getColor("CH" + QString::number(idx))); }

	int idx;
};

} // namespace adc
} // namespace scopy

#endif // TOOLCOMPONENT_H
