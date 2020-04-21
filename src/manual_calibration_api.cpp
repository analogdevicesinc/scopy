/*
 * Copyright 2018 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "manual_calibration_api.hpp"

#include "ui_calibratetemplate.h"
#include "ui_manualcalibration.h"

#include <QApplication>
#include <QElapsedTimer>
#include <QJSEngine>
#include <QMetaProperty>
#include <QThread>

using namespace adiscope;

ManualCalibration_API::ManualCalibration_API(ManualCalibration *m_calib)
	: ApiObject(), calib(m_calib), step_in_progress(-1) {}

void ManualCalibration_API::autoCalibration() {
	calib->ui->autoButton->click();
}

int ManualCalibration_API::start(int story) {
	if (step_in_progress >= 0) {
		// Reset the current step if another story
		// was already started and did not end yet
		step_in_progress = -1;
	}
	if (story >= calib->ui->calibList->count()) {
		return -1;
	}
	calib->ui->calibList->setCurrentRow(story);
	calib->ui->calibList->itemClicked(calib->ui->calibList->item(story));
	int nb_steps = calib->stCalibrationStory.story.count();
	if (nb_steps > 0) {
		step_in_progress = 0;
	}
	return nb_steps;
}

int ManualCalibration_API::next() {
	auto currentStory = calib->stCalibrationStory;

	// If a story was not yet started, return -1
	if (step_in_progress < 0) {
		return -1;
	}

	calib->TempUi->nextButton->click();
	step_in_progress++;
	if ((step_in_progress + 1) > currentStory.story.count()) {
		return -1;
	}
	return step_in_progress + 1;
}

int ManualCalibration_API::finish() {
	auto currentStory = calib->stCalibrationStory;

	// If a story was not yet started, return -1
	if (step_in_progress < 0) {
		return -1;
	}

	// Finish the story, if not all the steps are done yet
	while (step_in_progress < currentStory.story.count()) {
		next();
	}

	calib->TempUi->finishButton->click();
	step_in_progress = -1;

	if ((currentStory.calibProcedure + 1) <
	    calib->calibListString.count()) {
		return currentStory.calibProcedure + 1;
	}
	return -1;
}

bool ManualCalibration_API::setParam(double value) {
	auto currentStory = calib->stCalibrationStory;
	if (currentStory.story.count() > 0) {
		calib->TempUi->lineEdit->setText(QString::number(value));
		return true;
	}
	return false;
}

void ManualCalibration_API::saveCalibration(QString path) {
	bool buttonEn = calib->ui->saveButton->isEnabled();
	if (!buttonEn) {
		calib->ui->saveButton->setEnabled(true);
	}
	calib->setCalibrationFilePath(path);
	calib->ui->saveButton->click();
	calib->setCalibrationFilePath("");
	if (!buttonEn) {
		calib->ui->saveButton->setEnabled(false);
	}
}

void ManualCalibration_API::loadCalibration() {
	bool buttonEn = calib->ui->loadButton->isEnabled();
	if (!buttonEn) {
		calib->ui->loadButton->setEnabled(true);
	}
	calib->ui->loadButton->click();
	if (!buttonEn) {
		calib->ui->loadButton->setEnabled(false);
	}
}

double ManualCalibration_API::getOffsetPosDac() const {
	return calib->paramTable->item(0, 1)->text().toDouble();
}

double ManualCalibration_API::getGainPosDac() const {
	return calib->paramTable->item(1, 1)->text().toDouble();
}

double ManualCalibration_API::getOffsetPosAdc() const {
	return calib->paramTable->item(2, 1)->text().toDouble();
}

double ManualCalibration_API::getGainPosAdc() const {
	return calib->paramTable->item(3, 1)->text().toDouble();
}

double ManualCalibration_API::getOffsetNegDac() const {
	return calib->paramTable->item(4, 1)->text().toDouble();
}

double ManualCalibration_API::getGainNegDac() const {
	return calib->paramTable->item(5, 1)->text().toDouble();
}

double ManualCalibration_API::getOffsetNegAdc() const {
	return calib->paramTable->item(6, 1)->text().toDouble();
}

double ManualCalibration_API::getGainNegAdc() const {
	return calib->paramTable->item(7, 1)->text().toDouble();
}
