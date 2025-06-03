/*
 * Copyright (c) 2025 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "scenerenderer.hpp"

using namespace scopy;

SceneRenderer::SceneRenderer(QWidget *parent)
	: QWidget{parent}
{
	view = new Qt3DExtras::Qt3DWindow();
	container = QWidget::createWindowContainer(view, this);
	container->setMinimumSize(400, 300);

	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(container);
	layout->setMargin(0);
	this->setLayout(layout);

	// Root entity
	rootEntity = new Qt3DCore::QEntity();

	// Camera
	Qt3DRender::QCamera *camera = view->camera();
	camera->lens()->setPerspectiveProjection(45.0f, 16.f / 9.f, 0.1f, 1000.0f);
	camera->setPosition(QVector3D(0, 2.5f, 5.0f));
	camera->setViewCenter(QVector3D(0, 0, 0));

	// Camera controls
	auto *camController = new Qt3DExtras::QOrbitCameraController(rootEntity);
	camController->setCamera(camera);

	// Initialize cube

	// Cube mesh
	auto *cubeMesh = new Qt3DExtras::QCuboidMesh();

	// Transform
	cubeTransform = new Qt3DCore::QTransform();

	// Material
	m_cubeMaterial = new Qt3DExtras::QPhongMaterial();
	m_cubeMaterial->setDiffuse(m_cubeColor);

	// Entity
	cubeEntity = new Qt3DCore::QEntity(rootEntity);
	cubeEntity->addComponent(cubeMesh);
	cubeEntity->addComponent(cubeTransform);
	cubeEntity->addComponent(m_cubeMaterial);

	// Add line axes
	auto *planeMesh = new Qt3DExtras::QPlaneMesh(rootEntity);
	planeMesh->setWidth(400);
	planeMesh->setHeight(300);

	m_planeMaterial = new Qt3DExtras::QPhongMaterial();
	m_planeMaterial->setDiffuse(m_planeColor);

	// Transform
	planeTransform = new Qt3DCore::QTransform();
	planeTransform->setTranslation((QVector3D(0.0f, -1.0f, 0.0f)));

	auto meshEntity = new Qt3DCore::QEntity(rootEntity);
	meshEntity->addComponent(planeMesh);
	meshEntity->addComponent(planeTransform);
	meshEntity->addComponent(m_planeMaterial);

	// Set root entity
	view->setRootEntity(rootEntity);
}

void SceneRenderer::resetView()
{
	view->camera()->lens()->setPerspectiveProjection(45.0f, 16.f / 9.f, 0.1f, 1000.0f);
	view->camera()->setPosition(QVector3D(0, 2.5f, 5.0f));
	view->camera()->setViewCenter(QVector3D(0, 0, 0));
	view->camera()->setUpVector(QVector3D(0.0f, 1.0f, 0.0f));
}

void SceneRenderer::setRot(data3P rot)
{
	// To match screen rotation with real time rotation
	cubeTransform->setRotationX(rot.dataX);
	cubeTransform->setRotationY(rot.dataZ);
	cubeTransform->setRotationZ(rot.dataY);
}

void SceneRenderer::updateCubeColor(QColor color) { m_cubeMaterial->setDiffuse(color); }

void SceneRenderer::updatePlaneColor(QColor color) { m_planeMaterial->setDiffuse(color); }

#include "moc_scenerenderer.cpp"
