#ifndef GRTIMESINKCOMPONENT_H
#define GRTIMESINKCOMPONENT_H

#include <QObject>
#include "toolcomponent.h"
#include <gr-util/time_sink_f.h>
#include "channelcomponent.h"

namespace scopy {
namespace adc {

class GRTimeSinkComponent : public QObject, public ToolComponent, public DataProvider
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
	virtual void setData(bool copy = false) override;
	virtual void setRollingMode(bool b);
	virtual void setSampleRate(double);
	virtual void setBufferSize(uint32_t size);
	virtual void setPlotSize(uint32_t size);
	virtual void onStart() override;
	virtual void onStop() override;
	virtual void onInit() override;
	virtual void onDeinit() override;

	void addChannel(GRChannel *ch);
	void removeChannel(GRChannel *c);
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

	QList<GRChannel *> m_channels;
};
} // namespace adc
} // namespace scopy

#endif // GRTIMESINKCOMPONENT_H
