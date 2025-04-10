#ifndef SCENERENDERER_H
#define SCENERENDERER_H

#include "scopy-imuanalyzer_export.h"

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
#include <Qt3DRender/QCamera>

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

	Qt3DCore::QTransform *transform;
	QTimer timer;
	float angle = 0.0f;

public Q_SLOTS:
	// void updateRotation();
	void resetView();
};
}

#endif //SCENERENDERER_H
