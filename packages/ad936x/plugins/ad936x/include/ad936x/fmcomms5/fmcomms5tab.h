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
 *
 */

#ifndef FMCOMMS5TAB_H
#define FMCOMMS5TAB_H

#include <QWidget>
#include <QBoxLayout>
#include <iio.h>
#include <QCheckBox>
#include <QProgressBar>
#include <QPushButton>

namespace scopy {
namespace ad936x {
class Fmcomms5Tab : public QWidget
{
	Q_OBJECT
public:
	explicit Fmcomms5Tab(iio_context *ctx, QWidget *parent = nullptr);
	~Fmcomms5Tab();

Q_SIGNALS:
	void readRequested();

private:
	iio_context *m_ctx;
	QVBoxLayout *m_layout;
	QProgressBar *m_calibProgressBar;
	QPushButton *m_calibrateBtn;
	QPushButton *m_resetCalibrationBtn;
};
} // namespace ad936x
} // namespace scopy
#endif // FMCOMMS5TAB_H
