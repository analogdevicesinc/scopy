#ifndef PLOTCHANNELTRACKER_H
#define PLOTCHANNELTRACKER_H

#include "scopy-gui_export.h"
#include <basictracker.hpp>

namespace scopy {

class PlotChannel;
class PlotWidget;
class ChannelTracker
{
public:
	explicit ChannelTracker();
	~ChannelTracker();

	PlotChannel *channel;
	BasicTracker *tracker;
};

class SCOPY_GUI_EXPORT PlotTracker : public QObject
{
	Q_OBJECT

public:
	explicit PlotTracker(PlotWidget *plot, QList<PlotChannel *> *channels = new QList<PlotChannel *>());
	~PlotTracker();

	void setEnabled(bool en);
	bool isEnabled();

	void addChannel(PlotChannel *ch);
	void removeChannel(PlotChannel *ch);

	void setYAxisUnit(QString unit);

protected:
	void init(QList<PlotChannel *> *channels);
	ChannelTracker *createTracker(PlotChannel *ch);

protected Q_SLOTS:
	void onChannelSelected(PlotChannel *ch);

private:
	bool m_en;
	PlotWidget *m_plot;
	QSet<ChannelTracker *> *m_trackers;
	QString m_yAxisUnit;
};
} // namespace scopy

#endif // PLOTCHANNELTRACKER_H
