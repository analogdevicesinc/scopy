/*
 * Copyright (c) 2026 Analog Devices Inc.
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

#ifndef ACQUISITIONINFOPAGE_H
#define ACQUISITIONINFOPAGE_H

#include "scopy-acquisition_export.h"

#include <QLabel>
#include <QWidget>

#include <gui/infopage.h>

namespace scopy::component {
class Context;
}

namespace scopy::acquisition {

// The device-controller equivalent of gui's DeviceInfoPage. That one takes a
// libiio Connection; this one takes a component::Context and reads the same
// information through the component tree - identity off the Context itself,
// attribute values through each Attribute's read capability.
class SCOPY_ACQUISITION_EXPORT AcquisitionInfoPage : public QWidget
{
	Q_OBJECT
public:
	explicit AcquisitionInfoPage(component::Context *ctx, QWidget *parent = nullptr);
	~AcquisitionInfoPage() = default;

private:
	void setupUi();
	void setupInfoPage();

	component::Context *m_ctx;
	InfoPage *m_infoPage;
	QLabel *m_title;
	QLabel *m_contextInfo;
};

} // namespace scopy::acquisition
#endif // ACQUISITIONINFOPAGE_H
