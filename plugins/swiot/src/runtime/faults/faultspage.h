/*
 * Copyright (c) 2023 Analog Devices Inc.
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

#ifndef FAULTSPAGE_H
#define FAULTSPAGE_H

#include "faultsdevice.h"
#include "faultsgroup.h"

#include <iio.h>

#include <QPushButton>
#include <QTextEdit>
#include <QWidget>

namespace Ui {
class FaultsPage;
}

namespace scopy::swiot {
class FaultsDevice;

class FaultsPage : public QWidget
{
	Q_OBJECT
public:
	explicit FaultsPage(struct iio_context *context, QWidget *parent = nullptr);
	~FaultsPage();

	void update();

private:
	struct iio_context *m_context;

	Ui::FaultsPage *ui;

	FaultsDevice *m_ad74413rFaultsDevice;
	FaultsDevice *m_max14906FaultsDevice;

	void setupDevices();
};
} // namespace scopy::swiot
#endif // FAULTSPAGE_H
