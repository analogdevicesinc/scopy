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
#include <QSpinBox>

#include <cassert>
#include <cstdint>
#include <scopy/gui/logicanalyzer/prop/int.hpp>

using boost::optional;
using std::max;
using std::min;
using std::pair;

namespace scopy {
namespace prop {

Int::Int(QString name, QString desc, QString suffix, optional<pair<int64_t, int64_t>> range, Getter getter,
	 Setter setter)
	: Property(name, desc, getter, setter)
	, m_suffix_(suffix)
	, m_range_(range)
	, m_spinBox_(nullptr)
{}

QWidget* Int::getWidget(QWidget* parent, bool auto_commit)
{
	int64_t range_min = 0;
	uint64_t range_max = 0;

	if (m_spinBox_)
		return m_spinBox_;

	if (!m_getter_)
		return nullptr;

	try {
		m_value_ = m_getter_();
	} catch (const std::exception& e) {
		qWarning() << tr("Querying config key %1 resulted in %2").arg(m_name_, e.what());
		return nullptr;
	}

	GVariant* value = m_value_.gobj();
	if (!value)
		return nullptr;

	m_spinBox_ = new QSpinBox(parent);
	m_spinBox_->setSuffix(m_suffix_);

	const GVariantType* const type = g_variant_get_type(value);
	assert(type);

	if (g_variant_type_equal(type, G_VARIANT_TYPE_BYTE)) {
		range_min = 0, range_max = UINT8_MAX;
	} else if (g_variant_type_equal(type, G_VARIANT_TYPE_INT16)) {
		range_min = INT16_MIN, range_max = INT16_MAX;
	} else if (g_variant_type_equal(type, G_VARIANT_TYPE_UINT16)) {
		range_min = 0, range_max = UINT16_MAX;
	} else if (g_variant_type_equal(type, G_VARIANT_TYPE_INT32)) {
		range_min = INT32_MIN, range_max = INT32_MAX;
	} else if (g_variant_type_equal(type, G_VARIANT_TYPE_UINT32)) {
		range_min = 0, range_max = UINT32_MAX;
	} else if (g_variant_type_equal(type, G_VARIANT_TYPE_INT64)) {
		range_min = INT64_MIN, range_max = INT64_MAX;
	} else if (g_variant_type_equal(type, G_VARIANT_TYPE_UINT64)) {
		range_min = 0, range_max = UINT64_MAX;
	} else {
		// Unexpected value type.
		assert(false);
	}

	// @todo sigrok supports 64-bit quantities, but Qt does not have a
	// standard widget to allow the values to be modified over the full
	// 64-bit range on 32-bit machines. To solve the issue we need a
	// custom widget.

	range_min = max(range_min, (int64_t)INT_MIN);
	range_max = min(range_max, (uint64_t)INT_MAX);

	if (m_range_)
		m_spinBox_->setRange((int)m_range_->first, (int)m_range_->second);
	else
		m_spinBox_->setRange((int)range_min, (int)range_max);

	updateWidget();

	if (auto_commit)
		connect(m_spinBox_, SIGNAL(valueChanged(int)), this, SLOT(onValueChanged(int)));

	return m_spinBox_;
}

void Int::updateWidget()
{
	if (!m_spinBox_)
		return;

	try {
		m_value_ = m_getter_();
	} catch (const std::exception& e) {
		qWarning() << tr("Querying config key %1 resulted in %2").arg(m_name_, e.what());
		return;
	}

	GVariant* value = m_value_.gobj();
	assert(value);

	const GVariantType* const type = g_variant_get_type(value);
	assert(type);

	int64_t int_val = 0;

	if (g_variant_type_equal(type, G_VARIANT_TYPE_BYTE)) {
		int_val = g_variant_get_byte(value);
	} else if (g_variant_type_equal(type, G_VARIANT_TYPE_INT16)) {
		int_val = g_variant_get_int16(value);
	} else if (g_variant_type_equal(type, G_VARIANT_TYPE_UINT16)) {
		int_val = g_variant_get_uint16(value);
	} else if (g_variant_type_equal(type, G_VARIANT_TYPE_INT32)) {
		int_val = g_variant_get_int32(value);
	} else if (g_variant_type_equal(type, G_VARIANT_TYPE_UINT32)) {
		int_val = g_variant_get_uint32(value);
	} else if (g_variant_type_equal(type, G_VARIANT_TYPE_INT64)) {
		int_val = g_variant_get_int64(value);
	} else if (g_variant_type_equal(type, G_VARIANT_TYPE_UINT64)) {
		int_val = g_variant_get_uint64(value);
	} else {
		// Unexpected value type.
		assert(false);
	}

	m_spinBox_->setValue((int)int_val);
}

void Int::commit()
{
	assert(m_setter_);

	if (!m_spinBox_)
		return;

	GVariant* new_value = nullptr;
	const GVariantType* const type = g_variant_get_type(m_value_.gobj());
	assert(type);

	if (g_variant_type_equal(type, G_VARIANT_TYPE_BYTE))
		new_value = g_variant_new_byte(m_spinBox_->value());
	else if (g_variant_type_equal(type, G_VARIANT_TYPE_INT16))
		new_value = g_variant_new_int16(m_spinBox_->value());
	else if (g_variant_type_equal(type, G_VARIANT_TYPE_UINT16))
		new_value = g_variant_new_uint16(m_spinBox_->value());
	else if (g_variant_type_equal(type, G_VARIANT_TYPE_INT32))
		new_value = g_variant_new_int32(m_spinBox_->value());
	else if (g_variant_type_equal(type, G_VARIANT_TYPE_UINT32))
		new_value = g_variant_new_uint32(m_spinBox_->value());
	else if (g_variant_type_equal(type, G_VARIANT_TYPE_INT64))
		new_value = g_variant_new_int64(m_spinBox_->value());
	else if (g_variant_type_equal(type, G_VARIANT_TYPE_UINT64))
		new_value = g_variant_new_uint64(m_spinBox_->value());
	else {
		// Unexpected value type.
		assert(false);
	}

	assert(new_value);

	m_value_ = Glib::VariantBase(new_value);

	m_setter_(m_value_);
}

void Int::onValueChanged(int) { commit(); }

} // namespace prop
} // namespace scopy
