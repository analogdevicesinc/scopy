/*
 * Shamelessly stolen from the PulseView project.
 *
 * Copyright (C) 2013 Joel Holdsworth <joel@airwebreathe.org.uk>
 *
 * License: GPLv2+
 */

#ifndef PULSEVIEW_PV_WIDGETS_POPUP_H
#define PULSEVIEW_PV_WIDGETS_POPUP_H

#include <QWidget>

namespace adiscope {

class Popup : public QWidget
{
	Q_OBJECT

public:
	enum Position
	{
		Right,
		Top,
		Left,
		Bottom
	};

private:
	static const unsigned int ArrowLength;
	static const unsigned int ArrowOverlap;
	static const unsigned int MarginWidth;

public:
	Popup(QWidget *parent);

	const QPoint& point() const;
	Position position() const;

	void set_position(const QPoint point, Position pos);

private:
	bool space_for_arrow() const;

	QPolygon arrow_polygon() const;

	QRegion arrow_region() const;

	QRect bubble_rect() const;

	QRegion bubble_region() const;

	QRegion popup_region() const;

	void reposition_widget();

private:
	void closeEvent(QCloseEvent*);

	void paintEvent(QPaintEvent*);

	void resizeEvent(QResizeEvent*);

	void mouseReleaseEvent(QMouseEvent *e);

protected:
	void showEvent(QShowEvent *e);

Q_SIGNALS:
	void closed();

private:
	QPoint _point;
	Position _pos;
};

}

#endif // PULSEVIEW_PV_WIDGETS_POPUP_H
