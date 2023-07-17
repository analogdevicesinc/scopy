#ifndef GRTIMEPLOTADDON_H
#define GRTIMEPLOTADDON_H

#include "scopy-gr-util_export.h"
#include "tooladdon.h"
#include <gui/oscilloscope_plot.hpp>
#include <QGridLayout>
#include "time_sink_f.h"
#include <QTimer>

namespace scopy::grutil {
using namespace scopy;
class GRTopBlock;
class GRTimeChannelAddon;

class SCOPY_GR_UTIL_EXPORT GRTimePlotAddon : public QObject, public ToolAddon {
	Q_OBJECT
public:
	GRTimePlotAddon(QString name, GRTopBlock *top, QObject *parent = nullptr);
	~GRTimePlotAddon();

	QString getName() override;
	QWidget *getWidget() override;
	PlotWidget *plot();

public Q_SLOTS:
	void enable() override;
	void disable() override;
	void onStart() override;
	void onStop() override;
	void onAdd() override;
	void onRemove() override;
	void onChannelAdded(ToolAddon* t) override;
	void onChannelRemoved(ToolAddon*) override;

	void replot();
	void connectSignalPaths();
	void tearDownSignalPaths();
	void onNewData();
private:
	QString name;
	QWidget *widget;
	QTimer *m_plotTimer;
	GRTopBlock *m_top;
	PlotWidget *m_plotWidget;
	time_sink_f::sptr time_sink;
	QList<GRTimeChannelAddon*> grChannels;
};
}

#endif // GRTIMEPLOTADDON_H
