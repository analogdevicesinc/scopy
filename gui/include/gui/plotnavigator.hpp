#ifndef PLOTNAVIGATOR_H
#define PLOTNAVIGATOR_H

#include <QSet>
#include <QStack>
#include <QwtPlot>
#include "scopy-gui_export.h"

class QPushButton;

namespace scopy {
class HoverWidget;
class PlotChannel;
class PlotWidget;
class PlotMagnifier;
class PlotZoomer;

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
	PlotMagnifier *magnifier;
	PlotZoomer *zoomer;
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

	bool isZoomed();

	void setZoomerXAxesEn(bool en);
	void setZoomerYAxesEn(bool en);

	void setXAxesEn(bool en);
	bool isXAxesEn();
	void setYAxesEn(bool en);
	bool isYAxesEn();

	bool isEnabled();
	void setEnabled(bool en);

	void setBaseRect(const QRectF &rect);
	void setBaseRect(QwtAxisId axisId);
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

	void setResetOnNewBase(bool en);
	bool getResetOnNewBase();

	void setMagnifierPanModifier(Qt::KeyboardModifier modifier);
	void setMagnifierZoomModifier(Qt::KeyboardModifier modifier);
	Qt::KeyboardModifier getMagnifierPanModifier();
	Qt::KeyboardModifier getMagnifierZoomModifier();

	void setZoomerXModifier(Qt::KeyboardModifier modifier);
	void setZoomerYModifier(Qt::KeyboardModifier modifier);
	void setZoomerXYModifier(Qt::KeyboardModifier modifier);
	Qt::KeyboardModifier getZoomerXModifier();
	Qt::KeyboardModifier getZoomerYModifier();
	Qt::KeyboardModifier getZoomerXYModifier();

	static void syncPlotNavigators(PlotNavigator *pNav1, PlotNavigator *pNav2, QSet<QwtAxisId> *axes);
	void setResetButtonEn(bool en);

Q_SIGNALS:
	void reset();
	void undo();
	void rectChanged(const QRectF &rect, navigationType type);

protected:
	virtual bool eventFilter(QObject *object, QEvent *event) QWT_OVERRIDE;
	QSet<Navigator *> *navigators();
	QSet<QwtAxisId> *axes();

private:
	void init();
	void initNavigators();
	void initResetButton();
	QWidget *canvas();
	PlotMagnifier *createMagnifier(QwtAxisId axisId);
	PlotZoomer *createZoomer(QwtAxisId axisId);
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
	PlotZoomer *m_visibleZoomer;
	QSet<QwtAxisId> *m_axes;
	QSet<PlotChannel *> *m_channels;
	bool m_historyEn;
	bool m_autoBase;
	bool m_resetOnNewBase;
	bool m_magnifierEn, m_zoomerEn;
	Qt::KeyboardModifier m_zoomerXModifier, m_zoomerYModifier, m_zoomerXYModifier;
	Qt::KeyboardModifier m_magnifierPanModifier, m_magnifierZoomModifier;
	QPushButton *m_resetButton;
	HoverWidget *m_resetHover;
};
} // namespace scopy

#endif // PLOTNAVIGATOR_H
