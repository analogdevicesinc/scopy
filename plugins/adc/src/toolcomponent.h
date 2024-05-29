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

class SCOPY_ADCPLUGIN_EXPORT ChannelData : public QObject{
	Q_OBJECT
public:
	ChannelData(QObject *parent) : QObject(parent) {

	}
	~ChannelData() {
		freeData();

	}

	void freeData() {
		if(m_ownsData) {
			free((void*)m_xData);
			free((void*)m_yData);
		}
	}

	const float *xData() { return m_xData;}
	const float *yData() { return m_yData;}
	const size_t size() { return m_size;}

public Q_SLOTS:
	virtual void onNewData(const float *xData_, const float *yData_, size_t size, bool copy) {
		// this could be optimized not to reallocate the data
		freeData();
		if(copy) {
			m_xData = (float*)malloc(sizeof(float) * size);
			m_yData = (float*)malloc(sizeof(float) * size);
			memcpy(m_xData, xData_, sizeof(float) * size);
			memcpy(m_yData, yData_, sizeof(float) * size);
		} else {
			m_xData = (float*)xData_;
			m_yData = (float*)yData_;
		}
		m_size = size;
		m_ownsData = copy;

		Q_EMIT newData(xData_, yData_, size, copy);

	}
Q_SIGNALS:
	void newData(const float *xData_, const float *yData_, size_t size, bool copy);

private:
	size_t m_size = 0;
	float *m_xData = 0;
	float *m_yData = 0;
	bool m_ownsData	= false;
};

class SCOPY_ADCPLUGIN_EXPORT DataProvider
{
public:
	virtual void setSingleShot(bool) = 0;
	virtual size_t updateData() = 0;
	virtual bool finished() = 0;
	virtual void setData(bool copy = false) = 0;
};

class SCOPY_ADCPLUGIN_EXPORT ToolComponent
{
public:
	ToolComponent() : m_enabled(true), m_priority(0) {}
	virtual ~ToolComponent() {};
	virtual QString name() const { return m_name; };
	virtual int priority() const { return m_priority; };
	virtual void onStart(){};
	virtual void onStop(){};
	virtual void onInit(){};
	virtual void onDeinit(){};

	virtual void enable() {m_enabled = true;}
	virtual void disable() {m_enabled = false;}

	bool enabled() const {
		return m_enabled;
	}


protected:
	QString m_name;
	bool m_enabled = true;
	int m_priority = 0;
};

class SCOPY_ADCPLUGIN_EXPORT MetaComponent : public ToolComponent
{
public:
	MetaComponent() : ToolComponent() {}
	virtual ~MetaComponent() {}
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
		auto cm = components();
		for(auto c : cm) {
			c->onInit();
		}
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
