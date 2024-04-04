#ifndef GRDEVICECOMPONENT_H
#define GRDEVICECOMPONENT_H

#include "scopy-adcplugin_export.h"
#include "channelcomponent.h"
#include "toolcomponent.h"
#include "measurementcontroller.h"

#include <gr-util/griiodevicesource.h>
#include <gui/widgets/menucontrolbutton.h>

#include <QLabel>
#include <QPen>

namespace scopy::adc {
using namespace scopy;
using namespace scopy::grutil;
// class GRTimeChannelAddon;

class SCOPY_ADCPLUGIN_EXPORT SampleRateUser {
public:
	virtual void setSampleRate(double) = 0;
};

class SCOPY_ADCPLUGIN_EXPORT RollingModeUser {
public:
	virtual void setRollingMode(bool) = 0;
};

class SCOPY_ADCPLUGIN_EXPORT PlotSizeUser {
public:
	virtual void setPlotSize(uint32_t bufferSize) = 0;
};

class SCOPY_ADCPLUGIN_EXPORT BufferSizeUser {
public:
	virtual void setBufferSize(uint32_t bufferSize) = 0;
};
class SCOPY_ADCPLUGIN_EXPORT SingleYModeUser {
public:
		 virtual void setSingleYMode(bool) = 0;
};

class SCOPY_ADCPLUGIN_EXPORT XMinMaxUser {
public:
	virtual void setXMinMax(double, double) = 0;
};

class SCOPY_ADCPLUGIN_EXPORT MeasurementProvider {
public:
	virtual MeasureManagerInterface *getMeasureManager() = 0;
};

class SCOPY_ADCPLUGIN_EXPORT SnapshotProvider {
public:
	typedef struct
	{
		std::vector<float> x;
		std::vector<float> y;
		QString name;
	} SnapshotRecipe;

	virtual void addNewSnapshot(SnapshotRecipe) = 0;
};

class SCOPY_ADCPLUGIN_EXPORT SampleRateProvider {
public:
	virtual bool sampleRateAvailable() = 0;
	virtual double sampleRate() = 0;
};


class SCOPY_ADCPLUGIN_EXPORT GRDeviceComponent : public QWidget,
						 public ToolComponent,
						 public BufferSizeUser,
						 public SampleRateProvider
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
	void setBufferSize(uint32_t bufferSize) override;


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
	void createMenuControlButton(QWidget* = nullptr);

	void setupDeviceMenuControlButtonHelper(MenuControlButton *devBtn, QString name);

	QList<ChannelComponent*> m_channels;

	// SampleRateProvider interface

};
} // namespace scopy::adc

#endif // GRDEVICECOMPONENT_H
