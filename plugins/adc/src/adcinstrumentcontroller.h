#ifndef ADCINSTRUMENTCONTROLLER_H
#define ADCINSTRUMENTCONTROLLER_H

#include "toolcomponent.h"
#include "scopy-adc_export.h"
#include "timeplotcomponent.h"
#include "timeplotmanagersettings.h"
#include "cursorcomponent.h"
#include "measurecomponent.h"
#include <QMap>
#include <pluginbase/resourcemanager.h>

namespace scopy {
namespace adc {
class ChannelIdProvider;


class SyncController;
class SCOPY_ADC_EXPORT SyncInstrument {
public:
	virtual void setSyncController(SyncController *s) = 0;
	virtual void setSync(bool) = 0;
	virtual void runSync() = 0;	
	virtual void setSyncSingleShot(bool) = 0;
	virtual void setSyncBufferSize(uint32_t) = 0;
};

class SCOPY_ADC_EXPORT SyncController : public QObject {
	Q_OBJECT
public:
	SyncController(QObject *parent = nullptr) {}
	~SyncController() {}

	void addInstrument(SyncInstrument *s) { m_syncInstruments.append(s);
		m_syncState.insert(s,false);}
	void removeInstrument(SyncInstrument *s) {m_syncInstruments.removeAll(s);
		m_syncState.remove(s);}
	void run(SyncInstrument *master) {
		for (SyncInstrument *s : m_syncInstruments) {
			if(s == master)
				continue;

			s->setSyncSingleShot(m_singleShot);
			s->setSyncBufferSize(m_bufferSize);
			s->runSync();
		}
	}

	void sync(SyncInstrument *s) {
		m_syncState[s] = true;
		bool reset = true;
		for(auto sync : m_syncState.keys()) {
			if(m_syncState[sync] == false) {
				reset = false;
			}
		}
		if(reset == true) {
			for(auto sync : m_syncState.keys()) {
				m_syncState[sync] = false;
			}
			Q_EMIT resetAll();
		}
	}

	uint32_t bufferSize() const;
	void setBufferSize(uint32_t newBufferSize);

	bool singleShot() const;
	void setSingleShot(bool newSingleShot);

Q_SIGNALS:
	void resetAll();
	void bufferSizeChanged();
	void singleShotChanged();

private:
	QMap<SyncInstrument*, bool> m_syncState;
	QList<SyncInstrument*> m_syncInstruments;
	bool m_singleShot;
	uint32_t m_bufferSize;

	Q_PROPERTY(bool singleShot READ singleShot WRITE setSingleShot NOTIFY singleShotChanged)
	Q_PROPERTY(uint32_t bufferSize READ bufferSize WRITE setBufferSize NOTIFY bufferSizeChanged)
	//	Q_PROPERTY(float sampleRate READ sampleRate WRITE sampleRate);


};

class SCOPY_ADC_EXPORT ADCInstrumentController :
	  public QObject
	, public PlotProxy
	, public AcqNodeChannelAware
	, public ResourceUser
{
	Q_OBJECT
public:
	ADCInstrumentController(QString name, AcqTreeNode *tree, QObject *parent = nullptr);
	~ADCInstrumentController();

	ChannelIdProvider *getChannelIdProvider();
public:
	ToolComponent *getPlotAddon();
	ToolComponent *getPlotSettings();

	QList<ToolComponent *> getChannelAddons();
	QList<ToolComponent *> getComponents();

	QWidget *getInstrument() override;
	void setInstrument(QWidget *) override;

	void stop() override;
public Q_SLOTS:
	void init() override;
	void deinit() override;
	void onStart() override;
	void onStop() override;

	void addChannel(AcqTreeNode *c) override;
	void removeChannel(AcqTreeNode *c) override;

private Q_SLOTS:
	void stopUpdates();
	void startUpdates();

	void setSingleShot(bool b);
	void setFrameRate(double val);
	void updateFrameRate();
	void handlePreferences(QString key, QVariant v);

	void updateData();
	void update();

Q_SIGNALS:
	void requestStart();
	void requestStop();
	void requestStartLater();
	void requestStopLater();

private:
	void setupChannelMeasurement(TimePlotManager *c, ChannelComponent *ch);

	ADCInstrument *m_tool;
	TimePlotManager *m_plotComponentManager;
	MapStackedWidget *plotStack;

	TimePlotManagerSettings *m_timePlotSettingsComponent;
	CursorComponent *m_cursorComponent;
	MeasureComponent *m_measureComponent;

	ChannelIdProvider *chIdP;
	CollapsableMenuControlButton *m_otherCMCB;

	QFuture<void> m_refillFuture;
	QFutureWatcher<void> *m_fw;
	QTimer *m_plotTimer;

	AcqTreeNode *m_tree;
	DataProvider *m_dataProvider;
	QMap<AcqTreeNode *, ToolComponent *> m_acqNodeComponentMap;

	bool m_refreshTimerRunning;
	QString currentCategory;
};
} // namespace adc
} // namespace scopy
#endif // ADCINSTRUMENTCONTROLLER_H
