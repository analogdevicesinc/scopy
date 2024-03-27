#ifndef GRDEVICECOMPONENT_H
#define GRDEVICECOMPONENT_H

#include <gr-util/griiodevicesource.h>
#include "scopy-adcplugin_export.h"
#include "toolcomponent.h"
#include <gui/widgets/menucontrolbutton.h>

#include <QLabel>
#include <QPen>

namespace scopy::adc {
using namespace scopy;
using namespace scopy::grutil;
// class GRTimeChannelAddon;
class SCOPY_ADCPLUGIN_EXPORT GRDeviceComponent : public QWidget, public ToolComponent
{
	Q_OBJECT
public:
	GRDeviceComponent(GRIIODeviceSourceNode *node, QWidget *parent = nullptr);
	~GRDeviceComponent();

	// QString getName() override;
	// QWidget *getWidget() override;


	GRIIODeviceSource *src() const;
	CollapsableMenuControlButton *ctrl();

	// void registerChannel(GRTimeChannelAddon *ch);
	// QList<GRTimeChannelAddon *> getRegisteredChannels();

/*Q_SIGNALS:
	void updateBufferSize(uint32_t);*/

public Q_SLOTS:
	void onStart() override;
	void onStop() override;
	void onInit() override;
	void onDeinit() override;

	/*void preFlowStart() override;
	void postFlowStart() override;
	void preFlowStop() override;
	void postFlowStop() override;
	void preFlowBuild() override;
	void postFlowBuild() override;
	void preFlowTeardown() override;
	void postFlowTeardown() override;*/

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
	void createMenuControlButton(QWidget* = nullptr);

	void setupDeviceMenuControlButtonHelper(MenuControlButton *devBtn, QString name);

/*private Q_SLOTS:
	void setBufferSize(uint32_t bufferSize);*/
};
} // namespace scopy::adc

#endif // GRDEVICECOMPONENT_H
