#ifndef PLOTCOMPONENT_H
#define PLOTCOMPONENT_H

#include "scopy-adcplugin_export.h"
#include "toolcomponent.h"

#include <QFuture>
#include <QTimer>
#include <QVBoxLayout>
#include <QtConcurrent>

#include <plotwidget.h>

namespace scopy {
namespace adc {

class SCOPY_ADCPLUGIN_EXPORT PlotComponent : public QWidget, public ToolComponent
{
	Q_OBJECT
public:
	PlotComponent(QWidget *parent = nullptr);
	~PlotComponent();

	virtual PlotWidget *plot();
public Q_SLOTS:
	virtual void replot();
	//virtual double sampleRate()


public:
	QString getName();
	void onStart();
	void onStop();
	void onInit();
	void onDeinit();
private:

	QVBoxLayout *m_lay;
	PlotWidget* m_plot;
};

} // namespace grutil
} // namespace scopy

#endif // PLOTCOMPONENT_H
