/*
 * This file is part of the PulseView project.
 *
 * Copyright (C) 2012 Joel Holdsworth <joel@airwebreathe.org.uk>
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

#ifndef PULSEVIEW_PV_BINDING_BINDING_HPP
#define PULSEVIEW_PV_BINDING_BINDING_HPP

#include "scopy-sigrok-gui_export.h"

#include <QObject>
#include <QString>

#include <glib.h>
#include <map>
#include <memory>
#include <vector>

using std::map;
using std::shared_ptr;
using std::vector;

class QFormLayout;
class QLabel;
class QWidget;

namespace scopy {

namespace prop {
class Property;
}

namespace bind {

class SCOPY_SIGROK_GUI_EXPORT Binding : public QObject
{
	Q_OBJECT

public:
	const vector<shared_ptr<prop::Property>> &properties();

	void commit();

	void add_properties_to_form(QFormLayout *layout, bool auto_commit = false);

	QWidget *get_property_form(QWidget *parent, bool auto_commit = false);

	void update_property_widgets();

	static QString print_variant(QVariant gvar);

protected Q_SLOTS:
	void on_help_clicked();

protected:
	vector<shared_ptr<prop::Property>> properties_;
	map<QWidget *, QLabel *> help_labels_;
};

} // namespace bind
} // namespace scopy

#endif // PULSEVIEW_PV_BINDING_BINDING_HPP
