#ifndef GRDEVICECOMPONENT_H
#define GRDEVICECOMPONENT_H

#include "scopy-adc_export.h"
#include "channelcomponent.h"
#include "toolcomponent.h"
#include "interfaces.h"

#include <gr-util/griiodevicesource.h>
#include <gui/widgets/menucontrolbutton.h>

#include <QLabel>
#include <QPen>

namespace scopy::adc {
using namespace scopy;
using namespace scopy::grutil;
// class GRTimeChannelAddon;

class SCOPY_ADC_EXPORT GRDeviceComponent : public QWidget, public ToolComponent, public SampleRateProvider
{
	Q_OBJECT
public:
	GRDeviceComponent(GRIIODeviceSourceNode *node, QWidget *parent = nullptr);
	~GRDeviceComponent();

	GRIIODeviceSource *src() const;
	CollapsableMenuControlButton *ctrl();

	virtual bool sampleRateAvailable() override;
	virtual double sampleRate() override;

public Q_SLOTS:
	void onStart() override;
	void onStop() override;
	void onInit() override;
	void onDeinit() override;
	void setBufferSize(uint32_t bufferSize);

	void removeChannel(ChannelComponent *c);
	void addChannel(ChannelComponent *c);

private:
	QString name;
	QWidget *widget;
	GRIIODeviceSourceNode *m_node;
	GRIIODeviceSource *m_src;
	CollapsableMenuControlButton *m_ctrl;
	QPen m_pen;
	// QList<GRTimeChannelAddon *> m_channels;
	QWidget *createAttrMenu(QWidget *parent);
	QWidget *createMenu(QWidget *parent = nullptr);
	void createMenuControlButton(QWidget * = nullptr);

	void setupDeviceMenuControlButtonHelper(MenuControlButton *devBtn, QString name);

	QList<ChannelComponent *> m_channels;

	// SampleRateProvider interface
};
} // namespace scopy::adc

#endif // GRDEVICECOMPONENT_H
