#ifndef ADCINSTRUMENTCONTROLLER_H
#define ADCINSTRUMENTCONTROLLER_H

#include "toolcomponent.h"
#include "scopy-adcplugin_export.h"
#include "plotcomponent.h"
#include "timeplotsettingscomponent.h"
#include "xyplotsettingscomponent.h"
#include "cursorcomponent.h"
#include "measurecomponent.h"

namespace scopy {
namespace adc {
class ChannelIdProvider;

class SCOPY_ADCPLUGIN_EXPORT ADCInstrumentController : public QObject, public PlotProxy, public AcqNodeChannelAware
{
	Q_OBJECT
public:
	ADCInstrumentController(QString name, AcqTreeNode *tree, QObject *parent = nullptr);
	~ADCInstrumentController();

	ChannelIdProvider *getChannelIdProvider();

	// PlotProxy interface
public:
	ToolComponent *getPlotAddon();
	ToolComponent *getPlotSettings();

	QList<ToolComponent *> getChannelAddons();
	QList<ToolComponent *> getComponents();

	QWidget *getInstrument() override;
	void setInstrument(QWidget *) override;

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
	void requestStop();
private:

	void setupChannelMeasurement(PlotComponentManager *c, ChannelComponent *ch);

	ADCInstrument *m_tool;
	PlotComponentManager *m_plotComponentManager;
	MapStackedWidget *plotStack;

	TimePlotSettingsComponent *m_timePlotSettingsComponent;
	CursorComponent *m_cursorComponent;
	MeasureComponent *m_measureComponent;

	ChannelIdProvider *chIdP;

	QFuture<void> m_refillFuture;
	QFutureWatcher<void> *m_fw;
	QTimer *m_plotTimer;

	AcqTreeNode *m_tree;
	QMap<AcqTreeNode*, ToolComponent*> m_acqNodeComponentMap;

	bool m_refreshTimerRunning;
	QString currentCategory;


};
} // namespace adc
} // namespace scopy
#endif // ADCINSTRUMENTCONTROLLER_H
