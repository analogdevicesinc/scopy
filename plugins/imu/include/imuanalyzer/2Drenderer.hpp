#ifndef BUBBLELEVELRENDERER_H
#define BUBBLELEVELRENDERER_H

#include "imuanalyzerutils.hpp"
#include "scopy-imuanalyzer_export.h"
#include "plotwidget.h"
#include <plotaxis.h>

#include <QWidget>
#include <QPainter>
#include <QVBoxLayout>

namespace scopy {

class SCOPY_IMUANALYZER_EXPORT BubbleLevelRenderer : public QWidget {
	Q_OBJECT
public:
	BubbleLevelRenderer(QWidget *parent = nullptr);
	//~BubbleLevelRenderer();

public Q_SLOTS:
	void resetView();
	void resetPos();
	void setRot(rotation rot);

private:
	rotation m_rot;
	QVector<double> xLinePoint{90,90};
	QVector<double> yLinePoint{3,3};
	PlotWidget *plotWidget;
	QwtPlotCurve *point;

public:
	friend class ImuAnalyzerSettings;


};
}
#endif //BUBBLELEVELRENDERER_H
