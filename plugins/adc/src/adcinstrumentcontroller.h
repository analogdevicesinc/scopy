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
#include <pluginbase/toolmenuentry.h>

namespace scopy {
namespace adc {
class ChannelIdProvider;

class SCOPY_ADC_EXPORT ADCInstrumentController :
	  public QObject
	, public AcqNodeChannelAware
	, public MetaComponent
	, public ResourceUser
{
	Q_OBJECT
public:
	ADCInstrumentController(ToolMenuEntry *tme, QString name, AcqTreeNode *tree, QObject *parent = nullptr);
	~ADCInstrumentController();

	ChannelIdProvider *getChannelIdProvider();
public:
	ToolComponent *getPlotAddon();
	ToolComponent *getPlotSettings();

	QList<ToolComponent *> getChannelAddons();
	QList<ToolComponent *> getComponents();

	ADCInstrument *ui() const;

public Q_SLOTS:
	void init();
	void deinit();
	void onStart();
	void onStop();

	void start();
	void stop() override;

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
	void setupChannelMeasurement(PlotManager *c, ChannelComponent *ch);

	ADCInstrument *m_ui;
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

	bool m_started;

	AcqTreeNode *m_tree;
	DataProvider *m_dataProvider;
	QMap<AcqTreeNode *, ToolComponent *> m_acqNodeComponentMap;

	bool m_refreshTimerRunning;
};
} // namespace adc
} // namespace scopy
#endif // ADCINSTRUMENTCONTROLLER_H
