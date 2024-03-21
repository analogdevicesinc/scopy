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

#include "scopy-swiotrefactor_export.h"
#include "faultsdevice.h"

#include <iio.h>
#include <QPushButton>
#include <QTextEdit>
#include <QWidget>

namespace scopy::swiotrefactor {
class FaultsDevice;

class SCOPY_SWIOTREFACTOR_EXPORT FaultsPage : public QWidget
{
	Q_OBJECT
public:
	explicit FaultsPage(QString uri, QWidget *parent = nullptr);
	~FaultsPage();

	void update();

private:
	QString m_uri;
	struct iio_context *m_context;

	FaultsDevice *m_ad74413rFaultsDevice = nullptr;
	FaultsDevice *m_max14906FaultsDevice = nullptr;

	void setupDevices();
};
} // namespace scopy::swiotrefactor
#endif // FAULTSPAGE_H
