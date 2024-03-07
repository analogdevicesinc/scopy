#ifndef PLOTNAVIGATOR_H
#define PLOTNAVIGATOR_H

#include <QSet>
#include <QStack>
#include <QwtPlot>
#include "mouseplotmagnifier.hpp"
#include "scopy-gui_export.h"

namespace scopy {

class PlotChannel;
class PlotWidget;
class MousePlotMagnifier;
class BoundedPlotZoomer;

typedef enum
{
	None,
	Zoom,
	Magnify,
	Pan
} navigationType;

class Navigator
{
public:
	explicit Navigator();
	~Navigator();

	QStack<QRectF> *history;
	MousePlotMagnifier *magnifier;
	BoundedPlotZoomer *zoomer;
	navigationType lastOperation;
};

class SCOPY_GUI_EXPORT PlotNavigator : public QObject
{
	Q_OBJECT

public:
	explicit PlotNavigator(PlotWidget *plotWidget, QSet<PlotChannel *> *channels = new QSet<PlotChannel *>());
	~PlotNavigator();

	QSet<PlotChannel *> *channels();
	void addChannel(PlotChannel *channel);
	void removeChannel(PlotChannel *channel);

	void setXAxesEn(bool en);
	bool isXAxesEn();
	void setYAxesEn(bool en);
	bool isYAxesEn();

	bool isEnabled();
	void setEnabled(bool en);

	void setBaseRect(const QRectF &rect);
	void setBaseRect();

	void setBounded(bool bounded);
	bool isBounded();

	void setHistoryEn(bool en);
	bool isHistoryEn();

	void setMagnifierEn(bool en);
	bool isMagnifierEn();

	void setZoomerEn(bool en);
	bool isZoomerEn();

	void setAutoBaseEn(bool en);
	bool isAutoBaseEn();

	static void syncPlotNavigators(PlotNavigator *pNav1, PlotNavigator *pNav2, QSet<QwtAxisId> *axes);

Q_SIGNALS:
	void reset();
	void undo();

protected:
	virtual bool eventFilter(QObject *object, QEvent *event) QWT_OVERRIDE;
	QSet<Navigator *> *navigators();
	QSet<QwtAxisId> *axes();

private:
	void init();
	void initNavigators();
	QWidget *canvas();
	MousePlotMagnifier *createMagnifier(QwtAxisId axisId);
	BoundedPlotZoomer *createZoomer(QwtAxisId axisId);
	void addNavigators(QwtAxisId axisId);
	void removeNavigators(QwtAxisId axisId);
	void addRectToHistory(Navigator *nav, const QRectF &rect, navigationType type);
	void onUndo();
	void onReset();
	static void syncNavigators(PlotNavigator *pNav1, Navigator *nav1, PlotNavigator *pNav2, Navigator *nav2);

private:
	bool m_en;
	PlotWidget *m_plotWidget;
	QwtPlot *m_plot;
	QSet<Navigator *> *m_navigators;
	BoundedPlotZoomer *m_visibleZoomer;
	QSet<QwtAxisId> *m_axes;
	QSet<PlotChannel *> *m_channels;
	bool m_historyEn;
	bool m_autoBase;
	bool m_magnifierEn, m_zoomerEn;
};
} // namespace scopy

#endif // PLOTNAVIGATOR_H
