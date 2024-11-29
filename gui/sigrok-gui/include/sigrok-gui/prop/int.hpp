/*
 * This file is part of the PulseView project.
 *
 * Copyright (C) 2013 Joel Holdsworth <joel@airwebreathe.org.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Copyright (c) 2020 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef PULSEVIEW_PV_PROP_INT_HPP
#define PULSEVIEW_PV_PROP_INT_HPP

#include <utility>

#include <optional>

#include "property.hpp"
#include <glib.h>
#include "scopy-sigrok-gui_export.h"

using std::pair;

class QSpinBox;

namespace scopy {
namespace prop {

class SCOPY_SIGROK_GUI_EXPORT Int : public Property
{
	Q_OBJECT;

public:
	Int(QString name, QString desc, QString suffix,
		std::optional< pair<int64_t, int64_t> > range,
		Getter getter, Setter setter, GVariantClass gvarClass);

	virtual ~Int() = default;

	QWidget* get_widget(QWidget *parent, bool auto_commit);
	void update_widget();

	void commit();

private Q_SLOTS:
	void on_value_changed(int);

private:
	const QString suffix_;
	const std::optional< pair<int64_t, int64_t> > range_;

	QVariant value_;
	QSpinBox *spin_box_;
	GVariantClass gvar_class_type_;
};

}  // namespace prop
}  // namespace pv

#endif // PULSEVIEW_PV_PROP_INT_HPP
