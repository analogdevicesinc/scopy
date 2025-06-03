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

#ifndef DATAVISUALIZER_HPP
#define DATAVISUALIZER_HPP

#include "scopy-imuanalyzer_export.h"
#include <QWidget>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "imuanalyzerutils.hpp"

namespace scopy {

class DataVisualizer : public QWidget
{
	Q_OBJECT
public:
	DataVisualizer(QWidget *parent = nullptr);

public Q_SLOTS:
	void updateValues(data3P rot, data3P pos, float temp);

private:
	data3P m_rot, m_deltaRot;
	float temp = 0.0f;
	QPlainTextEdit *m_accelTextBox, *m_linearATextBox, *m_magnTextBox, *m_tempTextBox;
};
} // namespace scopy

#endif // DATAVISUALIZER_HPP
