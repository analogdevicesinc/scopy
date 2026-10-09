/*
 * Copyright (c) 2024 Analog Devices Inc.
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

#ifndef DEVICEINFOPAGE_H
#define DEVICEINFOPAGE_H

#include "scopy-core_export.h"
#include <QWidget>
#include <QLabel>
#include <gui/infopage.h>

namespace scopy {
namespace component {
class Context;
}

// Shows a device-controller context's identity (backend name/version/description) and its
// context attributes. Lives in core (not gui) so it can read component::Context directly
// without coupling gui to the device-controller.
class SCOPY_CORE_EXPORT DeviceInfoPage : public QWidget
{
	Q_OBJECT
public:
	explicit DeviceInfoPage(scopy::component::Context *ctx, QWidget *parent = nullptr);

private:
	void setupUi();
	void setupInfoPage(scopy::component::Context *ctx);

	InfoPage *m_infoPage;
	QLabel *m_title;
	QLabel *m_backendInfo;
};
} // namespace scopy

#endif // DEVICEINFOPAGE_H
