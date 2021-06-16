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

#ifndef PULSEVIEW_PV_PROP_DOUBLE_HPP
#define PULSEVIEW_PV_PROP_DOUBLE_HPP

#include <boost/optional.hpp>
#include <scopy/gui/logicanalyzer/prop/property.hpp>
#include <utility>

using std::pair;

class QDoubleSpinBox;

namespace scopy {
namespace prop {

class Double : public Property
{
	Q_OBJECT

public:
	Double(QString name, QString desc, int decimals, QString suffix, boost::optional<pair<double, double>> range,
	       boost::optional<double> step, Getter getter, Setter setter);

	virtual ~Double() = default;

	QWidget* getWidget(QWidget* parent, bool auto_commit);
	void updateWidget();

	void commit();

private Q_SLOTS:
	void onValueChanged(double);

private:
	const int m_decimals_;
	const QString m_suffix_;
	const boost::optional<pair<double, double>> m_range_;
	const boost::optional<double> m_step_;

	QDoubleSpinBox* m_spinBox_;
};

} // namespace prop
} // namespace scopy

#endif // PULSEVIEW_PV_PROP_DOUBLE_HPP
