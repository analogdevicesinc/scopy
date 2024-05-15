#ifndef GRTIMESINKCOMPONENT_H
#define GRTIMESINKCOMPONENT_H

#include <QObject>
#include "toolcomponent.h"
#include <gr-util/time_sink_f.h>
#include "grtimechannelcomponent.h"

namespace scopy {
namespace adc {

class GRTimeSinkAcquisitionSignalPath : QObject {
public:
	GRTimeSinkAcquisitionSignalPath(QString m_name, ChannelComponent *ch, GRIIOFloatChannelNode *node, QObject *parent)  : QObject(parent) {
		m_ch = ch;
		m_node = node;
		m_grch = node->src();
		m_signalPath = new GRSignalPath(m_name	+ m_grch->getDeviceSrc()->deviceName()
							+ m_grch->getChannelName(), this);
		m_signalPath->append(m_grch);
		m_scOff = new GRScaleOffsetProc(m_signalPath);
		m_signalPath->append(m_scOff);
		m_scOff->setOffset(0);
		m_scOff->setScale(1);
		m_signalPath->setEnabled(true); // or false
		m_node->top()->src()->registerSignalPath(m_signalPath);
	}
	~GRTimeSinkAcquisitionSignalPath() {

	}

	void onNewData(const float *xData, const float *yData, size_t size, bool copy) {
		m_ch->chData()->onNewData(xData, yData, size, copy);
	}

	ChannelComponent *m_ch;
	GRIIOFloatChannelNode *m_node;
	GRSignalPath *m_signalPath;
	GRScaleOffsetProc *m_scOff;
	GRIIOFloatChannelSrc *m_grch;

};

class GRTimeSinkComponent : public QObject,
			    public ToolComponent,
			    public DataProvider,
			    public BufferSizeUser,
			    public PlotSizeUser,
			    public RollingModeUser,
			    public SampleRateUser
{
	Q_OBJECT
public:
	GRTimeSinkComponent(QString name, GRTopBlockNode *t, QObject *parent = nullptr);
	~GRTimeSinkComponent();

	bool finished() override;
	bool enabled() const;

public Q_SLOTS:
	void connectSignalPaths();
	void tearDownSignalPaths();

	virtual size_t updateData() override;
	virtual void setSingleShot(bool) override;
	virtual void setData(bool raw = false) override;
	virtual void setRollingMode(bool b) override;
	virtual void setSampleRate(double) override;
	virtual void setBufferSize(uint32_t size) override;
	virtual void setPlotSize(uint32_t size) override;
	virtual void onStart() override;
	virtual void onStop() override;
	virtual void onInit() override;
	virtual void onDeinit() override;

	virtual void enable() override;
	virtual void disable() override;

	void addChannel(ChannelComponent* ch, GRIIOFloatChannelNode *node);
	void removeChannel(GRIIOFloatChannelNode *c);
Q_SIGNALS:
	void requestRebuild();

private:
	std::mutex refillMutex;
	time_sink_f::sptr time_sink;
	QMap<QString, int> time_channel_map;
	PlotSamplingInfo m_currentSamplingInfo;

	GRTopBlockNode *m_node;
	GRTopBlock *m_top;

	bool m_rollingMode;
	bool m_singleShot;
	bool m_syncMode;

	QList<GRTimeSinkAcquisitionSignalPath*> m_channels;

};
}
}

#endif // GRTIMESINKCOMPONENT_H
