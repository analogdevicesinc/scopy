#ifndef BUBBLELEVELRENDERER_H
#define BUBBLELEVELRENDERER_H

#include "imuanalyzerutils.hpp"
#include "scopy-imuanalyzer_export.h"

#include <QWidget>
#include <QPainter>

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

};
}
#endif //BUBBLELEVELRENDERER_H
