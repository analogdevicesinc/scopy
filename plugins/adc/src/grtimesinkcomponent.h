#ifndef GRTIMESINKCOMPONENT_H
#define GRTIMESINKCOMPONENT_H

#include <QObject>
#include "src/grdevicecomponent.h"
#include "toolcomponent.h"
#include <gr-util/time_sink_f.h>
#include "grtimechannelcomponent.h"

namespace scopy {
namespace adc {

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
public Q_SLOTS:
	void connectSignalPaths();
	void tearDownSignalPaths();

	virtual size_t updateData() override;
	virtual void setSingleShot(bool) override;
	virtual void setCurveData(bool raw = true) override;
	virtual void setRollingMode(bool b) override;
	virtual void setSampleRate(double) override;
	virtual void setBufferSize(uint32_t size) override;
	virtual void setPlotSize(uint32_t size) override;
	virtual void onStart() override;
	virtual void onStop() override;
	virtual void onInit() override;
	virtual void onDeinit() override;

	void addChannel(GRTimeChannelComponent *c);
	void removeChannel(GRTimeChannelComponent *c);
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
	bool m_showPlotTags;
	bool m_refreshTimerRunning;
	bool m_syncMode;
	bool fftComplexMode;

	QList<GRTimeChannelComponent*> m_channels;

};
}
}

#endif // GRTIMESINKCOMPONENT_H
