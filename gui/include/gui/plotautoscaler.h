#ifndef PLOTAUTOSCALER_H
#define PLOTAUTOSCALER_H

#include "scopy-gui_export.h"

#include <QObject>
#include <QWidget>

#include <plotchannel.h>
#include <spinbox_a.hpp>

namespace scopy::gui {

class SCOPY_GUI_EXPORT PlotAutoscaler : public QObject
{
	Q_OBJECT
public:
	PlotAutoscaler(QObject *parent = nullptr);
	~PlotAutoscaler();

	bool xAxisMode() const;
	double tolerance() const;
	int timeout() const;

Q_SIGNALS:
	void newMin(double);
	void newMax(double);

public Q_SLOTS:
	void start();
	void stop();
	void autoscale();
	void setXAxisMode(bool newXAxis);
	void setTolerance(double newTolerance);
	void addChannels(PlotChannel *);
	void removeChannels(PlotChannel *);
	void onNewData(const float *xData, const float *yData, size_t size, bool copy);
	void setTimeout(int);

private:
	QTimer *m_autoScaleTimer;
	QList<PlotChannel *> m_channels;
	bool m_xAxisMode;

	double m_tolerance;
	double m_max;
	double m_min;
	int m_timeout;
};
} // namespace scopy::gui
#endif // PLOTAUTOSCALER_H
