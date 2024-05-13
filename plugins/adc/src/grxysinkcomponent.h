#ifndef GRXYSINKCOMPONENT_H
#define GRXYSINKCOMPONENT_H
#include <QObject>
#include "toolcomponent.h"
#include <gr-util/time_sink_f.h>
#include "grxychannelcomponent.h"

namespace scopy {
namespace adc {
#if 0
class GRXySinkComponent : public QObject,
			    public ToolComponent,
			    public DataProvider,
			    public BufferSizeUser,
			  public SinkComponent
{
	Q_OBJECT
public:
	GRXySinkComponent(QString name, GRTopBlockNode *t, QObject *parent = nullptr);
	~GRXySinkComponent();

	bool finished() override;
	bool enabled() const;

public Q_SLOTS:
	void connectSignalPaths();
	void tearDownSignalPaths();
	void setXChannelData(const float *data);
	void setXChannel(GRXyChannelComponent *c);

	virtual size_t updateData() override;
	virtual void setSingleShot(bool) override;
	virtual void setCurveData(bool raw = true) override;
	virtual void setBufferSize(uint32_t size) override;
	virtual void onStart() override;
	virtual void onStop() override;
	virtual void onInit() override;
	virtual void onDeinit() override;

	void addChannel(ChannelComponent *c) override;
	void removeChannel(ChannelComponent *c) override;
Q_SIGNALS:
	void requestRebuild();

private:
	std::mutex refillMutex;
	time_sink_f::sptr time_sink;
	QMap<QString, int> time_channel_map;
	PlotSamplingInfo m_currentSamplingInfo;

	GRTopBlockNode *m_node;
	GRTopBlock *m_top;

	const float *m_xData;

	bool m_singleShot;
	bool m_refreshTimerRunning;
	bool m_syncMode;
	bool m_enabled;

	GRXyChannelComponent* m_xChannel;
	QList<GRXyChannelComponent*> m_channels;

};
#endif
}
}


#endif // GRXYSINKCOMPONENT_H
