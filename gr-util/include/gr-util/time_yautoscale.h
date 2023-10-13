#ifndef TIMEYAUTOSCALE_H
#define TIMEYAUTOSCALE_H

#include "scopy-gr-util_export.h"

#include <QObject>
#include <QWidget>

#include <plotchannel.h>
#include <spinbox_a.hpp>

namespace scopy::grutil {

class SCOPY_GR_UTIL_EXPORT TimeYAutoscale : public QObject
{
	Q_OBJECT
public:
	TimeYAutoscale(QObject *parent);
	~TimeYAutoscale();
Q_SIGNALS:
	void newMin(double);
	void newMax(double);

public Q_SLOTS:
	void start();
	void stop();
	void autoscale();
	void addChannels(PlotChannel *);
	void removeChannels(PlotChannel *);

private:
	QTimer *m_autoScaleTimer;
	QList<PlotChannel *> m_channels;
};
} // namespace scopy::grutil
#endif // TIMEYAUTOSCALE_H
