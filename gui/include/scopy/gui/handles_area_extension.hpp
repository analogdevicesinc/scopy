#ifndef HANDLESAREAEXTENSION_H
#define HANDLESAREAEXTENSION_H

#include <QPainter>

class QwtPlot;

class HandlesAreaExtension
{
public:
	HandlesAreaExtension(QwtPlot* plot);
	virtual ~HandlesAreaExtension() = default;

	// Override to provide extensions drawing behaviour
	// return false if other extensions will be drawn afther this one
	// return true if this will be the last extension to be drawn no matter
	// the number of extensions installed
	virtual bool draw(QPainter* painter, QWidget* owner) = 0;

protected:
	QwtPlot* m_plot;
};

class XBottomRuller : public HandlesAreaExtension
{
public:
	XBottomRuller(QwtPlot* plot);
	virtual ~XBottomRuller() = default;

	virtual bool draw(QPainter* painter, QWidget* owner) Q_DECL_OVERRIDE;
};

class YLeftRuller : public HandlesAreaExtension
{
public:
	YLeftRuller(QwtPlot* plot);
	virtual ~YLeftRuller() = default;

	virtual bool draw(QPainter* painter, QWidget* owner) Q_DECL_OVERRIDE;
};

class XTopRuller : public HandlesAreaExtension
{
public:
	XTopRuller(QwtPlot* plot);
	virtual ~XTopRuller() = default;

	virtual bool draw(QPainter* painter, QWidget* owner) Q_DECL_OVERRIDE;
};

#endif // HANDLESAREAEXTENSION_H
