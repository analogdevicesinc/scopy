#ifndef SCENERENDERER_H
#define SCENERENDERER_H

#include "scopy-imuanalyzer_export.h"
#include "imuanalyzerutils.hpp"

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QVBoxLayout>
#include <QPushButton>
#include <QWidget>
#include <QOpenGLShaderProgram>
#include <QPainter>

#include <QWidget>
#include <Qt3DCore/QEntity>
#include <Qt3DExtras/Qt3DWindow>
#include <QTimer>
#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QOrbitCameraController>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QCuboidMesh>
#include <Qt3DExtras>
#include <Qt3DRender/QCamera>

#include <Qt3DCore/QEntity>
#include <Qt3DRender/QGeometry>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DRender/QAttribute>
#include <QColor>
#include <QBuffer>

#include <Qt3DRender/QMaterial>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QRenderPass>
#include <Qt3DRender/QShaderProgram>
#include <QPolygon>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QRenderStateSet>
#include <Qt3DExtras/QText2DEntity>
#include <Qt3DCore/QEntity>

namespace scopy{
class SCOPY_IMUANALYZER_EXPORT SceneRenderer : public QWidget{
	Q_OBJECT
public:
	SceneRenderer(QWidget *parent = nullptr);
	// ~SceneRenderer();
private:
	Qt3DExtras::Qt3DWindow *view;
	QWidget *container;
	Qt3DCore::QEntity *rootEntity;

	Qt3DCore::QTransform *cubeTransform;
	Qt3DCore::QTransform *planeTransform;
	Qt3DExtras::QPhongMaterial *m_cubeMaterial;
	Qt3DExtras::QPhongMaterial *m_planeMaterial;

	Qt3DCore::QEntity *cubeEntity;

	QColor m_cubeColor = QRgb(0x008ECC);
	QColor m_planeColor = QRgb(0xa0a0a0);

public Q_SLOTS:
	void resetView();
	void resetPos();
	void setRot(data3P rot);
	void updateCubeColor(QColor color);
	void updatePlaneColor(QColor color);
};
}

#endif //SCENERENDERER_H
