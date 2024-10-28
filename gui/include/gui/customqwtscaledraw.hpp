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

#ifndef CUSTOMQWTSCALEDRAW_HPP
#define CUSTOMQWTSCALEDRAW_HPP

#include "plot_utils.hpp"
#include "scopy-gui_export.h"

#include <qwt_interval.h>
#include <qwt_scale_draw.h>
#include <qwt_scale_engine.h>
#include <qwt_text.h>

namespace scopy {

class SCOPY_GUI_EXPORT CustomQwtScaleDraw : public QwtScaleDraw
{
public:
	CustomQwtScaleDraw();
	~CustomQwtScaleDraw();
	void setUnitOfMeasure(QString unitOfMeasure);

protected:
	virtual QwtText label(double value) const;

private:
	QString m_unitOfMeasure;
};
} // namespace scopy
#endif // CUSTOMQWTSCALEDRAW_HPP
