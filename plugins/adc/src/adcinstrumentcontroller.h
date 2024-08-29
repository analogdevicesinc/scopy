#ifndef ADCINSTRUMENTCONTROLLER_H
#define ADCINSTRUMENTCONTROLLER_H

#include "toolcomponent.h"
#include "scopy-adc_export.h"
#include "timeplotcomponent.h"
#include "timeplotmanagersettings.h"
#include "measurecomponent.h"
#include <QMap>
#include <pluginbase/resourcemanager.h>
#include <pluginbase/toolmenuentry.h>

namespace scopy {
namespace adc {
class ChannelIdProvider;

class SCOPY_ADC_EXPORT ADCInstrumentController : public QObject,
						 public AcqNodeChannelAware,
						 public MetaComponent,
						 public ResourceUser
{
	Q_OBJECT
public:
	ADCInstrumentController(ToolMenuEntry *tme, QString name, AcqTreeNode *tree, QObject *parent = nullptr);
	virtual ~ADCInstrumentController();

	ChannelIdProvider *getChannelIdProvider();

	virtual void setEnableAddRemovePlot(bool);
	virtual void setEnableAddRemoveInstrument(bool);

public:
	ADCInstrument *ui() const;

public Q_SLOTS:
	virtual void init();
	virtual void deinit();
	virtual void onStart() override;
	virtual void onStop() override;

	virtual void start();
	virtual void stop() override;

protected Q_SLOTS:
	virtual void stopUpdates();
	virtual void startUpdates();

	virtual void setSingleShot(bool b);
	virtual void setFrameRate(double val);
	virtual void updateFrameRate();
	virtual void handlePreferences(QString key, QVariant v);

	virtual void updateData();
	virtual void update();

Q_SIGNALS:
	void requestStart();
	void requestStop();
	void requestStartLater();
	void requestStopLater();

protected:
	void setupChannelMeasurement(PlotManager *c, ChannelComponent *ch);

	ADCInstrument *m_ui;
	PlotManager *m_plotComponentManager;
	MapStackedWidget *plotStack;

	MeasureComponent *m_measureComponent;

	ChannelIdProvider *chIdP;
	CollapsableMenuControlButton *m_otherCMCB;

	QFuture<void> m_refillFuture;
	QFutureWatcher<void> *m_fw;
	QTimer *m_plotTimer;

	bool m_started;

	AcqTreeNode *m_tree;
	DataProvider *m_dataProvider;
	QMap<AcqTreeNode *, ToolComponent *> m_acqNodeComponentMap;

	bool m_refreshTimerRunning;
};

} // namespace adc
} // namespace scopy
#endif // ADCINSTRUMENTCONTROLLER_H
