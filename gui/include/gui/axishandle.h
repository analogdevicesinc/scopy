#ifndef AXISHANDLE_H
#define AXISHANDLE_H

#include "scopy-gui_export.h"
#include <qwt_painter.h>
#include <qwt_plot.h>

namespace scopy {

class PlotAxis;
class PlotWidget;

enum HandlePos : int
{
	NORTH_OR_WEST = 0, // west for y axis, north for x axis
	SOUTH_OR_EAST = 1  // east for y axis, south for x axis
};

enum HandleOrientation : int
{
	TO_CENTER = 0,
	TO_MIN = 1,
	TO_MAX = 2

};

enum BarVisibility : int
{
	ALWAYS = 0,
	ON_HOVER = 1,
	NEVER = 2

};

class SCOPY_GUI_EXPORT AxisHandle : public QWidget
{
	Q_OBJECT

public:
	AxisHandle(QwtAxisId axisId, HandlePos handlePos, QwtPlot *plot);
	~AxisHandle() override;

	void setColor(const QColor &color);
	QColor getColor();

	void setAxis(QwtAxisId axis);
	QwtAxisId getAxisId();

	int getPos();
	void setPos(int pos);
	void setPosSilent(int pos);

	void setBarVisibility(BarVisibility bar);
	BarVisibility getBarVisibility();

	void setHandlePos(HandlePos pos);
	HandlePos getHandlePos();

	void setBounded(bool bounded);
	bool isBounded() const;

	uint getLineWidth();
	void setLineWidth(int width);

	void syncWithPlotAxis(bool sync = true);

Q_SIGNALS:
	void pixelPosChanged(int pos);

protected:
	void init();
	void paintEvent(QPaintEvent *event) override;
	bool eventFilter(QObject *object, QEvent *event) override;

	QRect getBigRect();
	QRect getSmallRect();
	QRect getRect();
	QLine getLine();
	QRect getRectFromLine(QLine line);
	void setHandle(HandleOrientation orientation = HandleOrientation::TO_CENTER);
	void updateHandleOrientation();

	void onMouseMove(QPointF pos);
	bool onMouseButtonPress(QPointF pos);
	void onMouseButtonRelease();
	bool onEnter(QPointF pos);
	void onLeave();
	void onResize();
	void onDoubleClick(QPointF pos);

private:
	QwtAxisId m_axisId;
	QPixmap m_handle;
	QwtPlot *m_plot;
	QColor m_color;
	QPen m_pen;
	QPoint m_pos;
	HandlePos m_handlePos;
	int m_handleMargins;
	int m_handlePadding;
	int m_handleSize;
	bool m_isHovering;
	bool m_pressed;
	bool m_bounded;
	BarVisibility m_barVisibility;
};
} // namespace scopy

#endif /* AXISHANDLE_H */
