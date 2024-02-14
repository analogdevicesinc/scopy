#ifndef TIMECHANNELADDON_H
#define TIMECHANNELADDON_H

#include "measurementcontroller.h"
#include "scopy-gr-util_export.h"
#include "tooladdon.h"

#include <QObject>

#include <plotchannel.h>

namespace scopy {
namespace grutil {
class PlotAddon;

class SCOPY_GR_UTIL_EXPORT ChannelAddon : public QObject, public ToolAddon
{
	Q_OBJECT
public:
	ChannelAddon(QString ch, PlotAddon *plotAddon, QPen pen, QObject *parent = nullptr);
	virtual ~ChannelAddon();

	QString getName() override;
	QWidget *getWidget() override;
	QWidget *getMenuControlWidget();
	void setMenuControlWidget(QWidget *);

	QPen pen() const;
	bool enabled() const;
	PlotChannel *plotCh() const;

	virtual QWidget *createCurveMenu(QWidget *parent);

Q_SIGNALS:
	void requestDeleteChannel(ChannelAddon *);

protected:
	QString m_channelName;
	PlotChannel *m_plotCh;
	PlotAxis *m_plotAxis;
	PlotAxisHandle *m_plotAxisHandle;
	PlotAddon *m_plotAddon;
	QPen m_pen;
	QWidget *widget;
	QWidget *m_mcw;

	bool m_enabled;

public Q_SLOTS:
	void enable() override;
	void disable() override;
	void onStart() override;
	void onStop() override;
	void onInit() override;
	void onDeinit() override;

	void onNewData(const float *xData, const float *yData, int size);
};

class SCOPY_GR_UTIL_EXPORT MeasurementProvider
{
public:
	virtual MeasureManagerInterface *getMeasureManager() = 0;
};

class SCOPY_GR_UTIL_EXPORT SnapshotProvider
{
public:
	typedef struct
	{
		std::vector<float> x;
		std::vector<float> y;
		QString name;
	} SnapshotRecipe;

	virtual void addNewSnapshot(SnapshotRecipe) = 0;
};

class SCOPY_GR_UTIL_EXPORT SampleRateProvider
{
public:
	virtual bool sampleRateAvailable() = 0;
	virtual double sampleRate() = 0;
};

class SCOPY_GR_UTIL_EXPORT ChannelConfigAware
{
public:
	virtual void onChannelAdded(ChannelAddon *) = 0;
	virtual void onChannelRemoved(ChannelAddon *) = 0;
};

class SCOPY_GR_UTIL_EXPORT SingleYModeAware
{
public:
	virtual void setSingleYMode(bool) = 0;
};

class SCOPY_GR_UTIL_EXPORT TimeChannelAddon : public ChannelAddon,
					      public SampleRateProvider,
					      public MeasurementProvider,
					      public SnapshotProvider,
					      public SingleYModeAware
{
public:
	TimeChannelAddon(QString ch, PlotAddon *plotAddon, QPen pen, QObject *parent = nullptr);
	virtual ~TimeChannelAddon();
};

} // namespace grutil
} // namespace scopy
#endif // TIMECHANNELADDON_H
