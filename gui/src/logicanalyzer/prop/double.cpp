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

#include <QDebug>
#include <QDoubleSpinBox>

#include <cassert>
#include <scopy/gui/logicanalyzer/prop/double.hpp>

using boost::optional;
using std::pair;

namespace scopy {
namespace prop {

Double::Double(QString name, QString desc, int decimals, QString suffix, optional<pair<double, double>> range,
	       optional<double> step, Getter getter, Setter setter)
	: Property(name, desc, getter, setter)
	, m_decimals_(decimals)
	, m_suffix_(suffix)
	, m_range_(range)
	, m_step_(step)
	, m_spinBox_(nullptr)
{}

QWidget* Double::getWidget(QWidget* parent, bool auto_commit)
{
	if (m_spinBox_)
		return m_spinBox_;

	if (!m_getter_)
		return nullptr;

	try {
		Glib::VariantBase variant = m_getter_();
		if (!variant.gobj())
			return nullptr;
	} catch (const std::exception& e) {
		qWarning() << tr("Querying config key %1 resulted in %2").arg(m_name_, e.what());
		return nullptr;
	}

	m_spinBox_ = new QDoubleSpinBox(parent);
	m_spinBox_->setDecimals(m_decimals_);
	m_spinBox_->setSuffix(m_suffix_);
	if (m_range_)
		m_spinBox_->setRange(m_range_->first, m_range_->second);
	if (m_step_)
		m_spinBox_->setSingleStep(*m_step_);

	updateWidget();

	if (auto_commit)
		connect(m_spinBox_, SIGNAL(valueChanged(double)), this, SLOT(onValueChanged(double)));

	return m_spinBox_;
}

void Double::updateWidget()
{
	if (!m_spinBox_)
		return;

	Glib::VariantBase variant;

	try {
		variant = m_getter_();
	} catch (const std::exception& e) {
		qWarning() << tr("Querying config key %1 resulted in %2").arg(m_name_, e.what());
		return;
	}

	assert(variant.gobj());

	double value = Glib::VariantBase::cast_dynamic<Glib::Variant<double>>(variant).get();
	m_spinBox_->setValue(value);
}

void Double::commit()
{
	assert(m_setter_);

	if (!m_spinBox_)
		return;

	m_setter_(Glib::Variant<double>::create(m_spinBox_->value()));
}

void Double::onValueChanged(double) { commit(); }

} // namespace prop
} // namespace scopy
