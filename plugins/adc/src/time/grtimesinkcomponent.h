#ifndef GRTIMESINKCOMPONENT_H
#define GRTIMESINKCOMPONENT_H

#include <QObject>
#include "adcinterfaces.h"
#include <gui/toolcomponent.h>
#include <gui/channelcomponent.h>
#include <gr-util/time_sink_f.h>
#include <synccontroller.h>
#include "adcacquisitionmanager.h"

namespace scopy {
namespace adc {

class GRTimeSinkComponent : public QObject, public DataProvider, public SyncInstrument
{
	Q_OBJECT
public:
	GRTimeSinkComponent(QString name, GRTopBlockNode *t, QObject *parent = nullptr);
	~GRTimeSinkComponent();

	bool finished() override;
	const QString &name() const;

public Q_SLOTS:
	void connectSignalPaths();
	void tearDownSignalPaths();

	virtual void onArm() override;
	virtual void onDisarm() override;
	virtual void setSyncMode(bool b) override;
	virtual void setSyncController(SyncController *s) override;
	virtual bool syncMode() override;

	void init();
	void deinit();

	virtual bool start() override;
	virtual void stop() override;

	virtual size_t updateData() override;
	virtual void setSingleShot(bool) override;
	virtual void setData(bool copy = false) override;

	SamplingInfo samplingInfo();
	void setSamplingInfo(SamplingInfo p);

	void addChannel(GRChannel *ch);
	void removeChannel(GRChannel *c);

	void setSyncSingleShot(bool) override;
	void setSyncBufferSize(uint32_t) override;

Q_SIGNALS:
	void arm();
	void disarm();

	void ready();
	void finish();

	void requestRebuild();
	void requestSingleShot(bool);
	void requestBufferSize(uint32_t);

private:
	std::mutex refillMutex;
	time_sink_f::sptr time_sink;
	QMap<QString, int> time_channel_map;
	SamplingInfo m_samplingInfo;

	GRTopBlockNode *m_node;
	GRTopBlock *m_top;

	bool m_singleShot;
	bool m_syncMode;
	bool m_armed;

	SyncController *m_sync;

	QList<GRChannel *> m_channels;
	QString m_name;

	// SampleRateProvider interface
};
} // namespace adc
} // namespace scopy

#endif // GRTIMESINKCOMPONENT_H
