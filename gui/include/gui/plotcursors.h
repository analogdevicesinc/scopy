#ifndef PLOTCURSORS_H
#define PLOTCURSORS_H
#include <QObject>
#include "plotwidget.h"
#include "plotaxis.h"
#include <QPair>
#include "symbol_controller.h"
#include "scopy-gui_export.h"
#include <QDebug>
#include <QLabel>

namespace scopy {

class SCOPY_GUI_EXPORT Cursor : public QObject {
	Q_OBJECT
public:
	Cursor(PlotWidget* p, PlotAxis *ax, QPen pen);
	~Cursor();
	PlotLineHandle *cursorHandle() const;

public Q_SLOTS:
	void setAxis(PlotAxis *ax);
	void setVisible(bool b);
Q_SIGNALS:
	void positionChanged(double);

protected:
	PlotWidget *m_plotWidget;
	PlotAxis *m_axis;
	QwtPlot *m_plot;
	QPen m_pen;
	SymbolController* m_symbolCtrl;
	PlotLineHandle* m_cursorHandle;
	Symbol* m_cursorBar;

};

class SCOPY_GUI_EXPORT VCursor : public Cursor {
public:
	explicit VCursor(PlotWidget* p, PlotAxis *ax, bool right = true, QPen pen = QPen(QColor(155, 155, 155), 1, Qt::DashLine));
	~VCursor();;

	double computePosition(int pos);
};

class SCOPY_GUI_EXPORT HCursor : public Cursor {
public:
	explicit HCursor(PlotWidget* p, PlotAxis *ax, bool bottom = true, QPen pen = QPen(QColor(155, 155, 155), 1, Qt::DashLine));
	~HCursor();

	double computePosition(int pos);


};

class SCOPY_GUI_EXPORT PlotCursors : public QObject
{
	Q_OBJECT
public:
	PlotCursors(PlotWidget* plot) {
		m_vCursors.first = new VCursor(plot, plot->plotAxis(QwtAxis::YLeft)[0]); // default y-axis
		m_vCursors.second = new VCursor(plot, plot->plotAxis(QwtAxis::YLeft)[0]);
		m_hCursors.first = new HCursor(plot,plot->xAxis());
		m_hCursors.second = new HCursor(plot,plot->xAxis());

		connect(m_vCursors.first, &Cursor::positionChanged, this, &PlotCursors::v1PositionChanged);
		connect(m_vCursors.second, &Cursor::positionChanged, this,  &PlotCursors::v2PositionChanged);
		connect(m_hCursors.first, &Cursor::positionChanged, this, &PlotCursors::h1PositionChanged);
		connect(m_hCursors.second, &Cursor::positionChanged, this, &PlotCursors::h2PositionChanged);
	}
	~PlotCursors() {};
public Q_SLOTS:
	void setVisible(bool b) {
		m_vCursors.first->setVisible(b);
		m_vCursors.second->setVisible(b);
		m_hCursors.first->setVisible(b);
		m_hCursors.second->setVisible(b);
	}
Q_SIGNALS:
	void v1PositionChanged(double);
	void v2PositionChanged(double);
	void h1PositionChanged(double);
	void h2PositionChanged(double);

private:
	QPair<Cursor*, Cursor*> m_vCursors;
	QPair<Cursor*, Cursor*> m_hCursors;
};

class SCOPY_GUI_EXPORT PlotCursorReadouts : public QWidget
{
	Q_OBJECT
public:
	PlotCursorReadouts(QWidget *parent = nullptr) : QWidget(parent) {
		QGridLayout *grid = new QGridLayout(this);
		setLayout(grid);
		V1 = new QLabel("V1:",this);
		V1_val = new QLabel("",this);
		V2 = new QLabel("V2:",this);
		V2_val = new QLabel("",this);
		H1 = new QLabel("H1:",this);
		H1_val = new QLabel("",this);
		H2 = new QLabel("H2:",this);
		H2_val = new QLabel("123",this);
		deltaV = new QLabel("DeltaV",this);
		deltaV_val = new QLabel("",this);
		deltaH = new QLabel("DeltaH",this);
		deltaH_val = new QLabel("",this);

		grid->addWidget(V1, 0,0);
		grid->addWidget(V1_val, 0,1);
		grid->addWidget(V2, 1,0);
		grid->addWidget(V2_val, 1,1);
		grid->addWidget(deltaV, 2,0);
		grid->addWidget(deltaV_val, 2,1);

		grid->addWidget(H1, 0,2);
		grid->addWidget(H1_val, 0,3);
		grid->addWidget(H2, 1,2);
		grid->addWidget(H2_val, 1,3);
		grid->addWidget(deltaH, 2,2);
		grid->addWidget(deltaH_val, 2,3);
	}
	~PlotCursorReadouts() {};

public Q_SLOTS:
	void setV1(double val) { v1 = val; update();}
	void setV2(double val) { v2 = val; update();}
	void setH1(double val) { h1 = val; update();}
	void setH2(double val) { h2 = val; update();}
	void update() {
		V1_val->setText(QString::number(v1));
		V2_val->setText(QString::number(v2));
		H1_val->setText(QString::number(h1));
		H2_val->setText(QString::number(h2));
		deltaV_val->setText(QString::number(v2-v1));
		deltaH_val->setText(QString::number(h2-h1));
	}


private:
	double v1,v2,h1,h2;
	QLabel *V1,*V2,*H1,*H2;
	QLabel *V1_val,*V2_val,*H1_val,*H2_val;
	QLabel *deltaV, *deltaH;
	QLabel *deltaV_val, *deltaH_val;
};



}

#endif // PLOTCURSORS_H
