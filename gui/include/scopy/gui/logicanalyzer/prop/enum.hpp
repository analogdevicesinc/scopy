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

#ifndef PULSEVIEW_PV_PROP_ENUM_HPP
#define PULSEVIEW_PV_PROP_ENUM_HPP

#include <QMetaType>

#include <scopy/gui/logicanalyzer/prop/property.hpp>
#include <utility>
#include <vector>

using std::pair;
using std::vector;

Q_DECLARE_METATYPE(Glib::VariantBase);

class QComboBox;
class QLabel;
class QSlider;

namespace scopy {
namespace prop {

class Enum : public Property
{
	Q_OBJECT;

public:
	Enum(QString name, QString desc, vector<pair<Glib::VariantBase, QString>> values, Getter getter, Setter setter);

	virtual ~Enum() = default;

	QWidget* getWidget(QWidget* parent, bool auto_commit);
	void updateWidget();

	void commit();

private Q_SLOTS:
	void onCurrentIndexChanged(int);
	void onValueChanged(int);

private:
	const vector<pair<Glib::VariantBase, QString>> m_values_;
	bool m_isRange_;

	QComboBox* m_selector_;

	QWidget* m_sliderLayoutWidget_;
	QSlider* m_slider_;
	QLabel* m_sliderLabel_;
};

} // namespace prop
} // namespace scopy

#endif // PULSEVIEW_PV_PROP_ENUM_HPP
