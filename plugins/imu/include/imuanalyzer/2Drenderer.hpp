#ifndef BUBBLELEVELRENDERER_H
#define BUBBLELEVELRENDERER_H

#include "imuanalyzerutils.hpp"
#include "scopy-imuanalyzer_export.h"

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

protected:
	void paintEvent(QPaintEvent *event) override;

private:
	rotation m_rot;

};
}
#endif //BUBBLELEVELRENDERER_H
