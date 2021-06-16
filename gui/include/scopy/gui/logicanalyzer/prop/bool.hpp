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


#ifndef PULSEVIEW_PV_PROP_BOOL_HPP
#define PULSEVIEW_PV_PROP_BOOL_HPP

#include <scopy/gui/logicanalyzer/prop/property.hpp>

class QCheckBox;

namespace scopy {
namespace prop {

class Bool : public Property
{
	Q_OBJECT;

public:
	Bool(QString name, QString desc, Getter getter, Setter setter);

	virtual ~Bool() = default;

	QWidget* getWidget(QWidget *parent, bool auto_commit);
	bool labeledWidget() const;
	void updateWidget();

	void commit();

private Q_SLOTS:
	void on_state_changed(int);

private:
	QCheckBox *check_box_;
};

}  // namespace prop
}  // namespace pv

#endif // PULSEVIEW_PV_PROP_BOOL_HPP
