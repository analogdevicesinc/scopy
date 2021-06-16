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

#include <QComboBox>
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>

#include <cassert>
#include <cfloat>
#include <cmath>
#include <limits>
#include <scopy/gui/logicanalyzer/prop/enum.hpp>
#include <vector>

using std::abs;
// Note that "using std::isnan;" is _not_ put here since that would break
// compilation on some platforms. Use "std::isnan()" instead in checks below.
using std::numeric_limits;
using std::pair;
using std::vector;

namespace scopy {
namespace prop {

Enum::Enum(QString name, QString desc, vector<pair<Glib::VariantBase, QString>> values, Getter getter, Setter setter)
	: Property(name, desc, getter, setter)
	, m_values_(values)
	, m_isRange_(false)
	, m_selector_(nullptr)
	, m_sliderLayoutWidget_(nullptr)
	, m_slider_(nullptr)
	, m_sliderLabel_(nullptr)
{
	// Try to determine whether the values make up a range, created by e.g.
	// std_gvar_min_max_step_thresholds()

	vector<double> deltas;
	double prev_value = 0;

	for (const pair<Glib::VariantBase, QString>& v : m_values_) {
		gdouble value;
		if (v.first.is_of_type(Glib::VariantType("d"))) {
			g_variant_get((GVariant*)(v.first.gobj()), "d", &value);
		} else if (v.first.is_of_type(Glib::VariantType("(dd)"))) {
			gdouble dummy;
			g_variant_get((GVariant*)(v.first.gobj()), "(dd)", &value, &dummy);
		} else
			break; // Type not d or (dd), so not a range that we can handle
		deltas.push_back(value - prev_value);
		prev_value = value;
	}

	if (deltas.size() > 0) {
		bool constant_delta = true;
		double prev_delta = numeric_limits<double>::quiet_NaN();

		bool skip_first = true;
		for (double delta : deltas) {
			// First value is incorrect, it's the delta to 0 since no
			// previous value existed yet
			if (skip_first) {
				skip_first = false;
				continue;
			}
			if (std::isnan(prev_delta))
				prev_delta = delta;

			// 2*DBL_EPSILON doesn't work here, so use a workaround
			if (abs(delta - prev_delta) > (delta / 10))
				constant_delta = false;

			prev_delta = delta;
		}

		if (constant_delta)
			m_isRange_ = true;
	}
}

QWidget* Enum::getWidget(QWidget* parent, bool auto_commit)
{
	if (!m_getter_)
		return nullptr;

	Glib::VariantBase variant;

	try {
		variant = m_getter_();
	} catch (const std::exception& e) {
		qWarning() << tr("Querying config key %1 resulted in %2").arg(m_name_, e.what());
		return nullptr;
	}

	if (!variant.gobj())
		return nullptr;

	if (m_isRange_) {
		// Use slider
		if (m_sliderLayoutWidget_)
			return m_sliderLayoutWidget_;

		m_slider_ = new QSlider();
		// Sliders can't handle float values, so we just use it to specify
		// the number of steps that we're away from the range's beginning
		m_slider_->setOrientation(Qt::Horizontal);
		m_slider_->setMinimum(0);
		m_slider_->setMaximum(m_values_.size() - 1);
		m_slider_->setSingleStep(1);

		m_sliderLabel_ = new QLabel();

		m_sliderLayoutWidget_ = new QWidget(parent);
		QHBoxLayout* layout = new QHBoxLayout(m_sliderLayoutWidget_);
		layout->addWidget(m_slider_);
		layout->addWidget(m_sliderLabel_);

		updateWidget();

		if (auto_commit)
			connect(m_slider_, SIGNAL(valueChanged(int)), this, SLOT(onValueChanged(int)));

		return m_sliderLayoutWidget_;

	} else {
		// Use combo box
		if (m_selector_)
			return m_selector_;

		m_selector_ = new QComboBox(parent);
		for (unsigned int i = 0; i < m_values_.size(); i++) {
			const pair<Glib::VariantBase, QString>& v = m_values_[i];
			m_selector_->addItem(v.second, qVariantFromValue(v.first));
		}

		updateWidget();

		if (auto_commit)
			connect(m_selector_, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentIndexChanged(int)));

		return m_selector_;
	}
}

void Enum::updateWidget()
{
	Glib::VariantBase variant;

	try {
		variant = m_getter_();
	} catch (const std::exception& e) {
		qWarning() << tr("Querying config key %1 resulted in %2").arg(m_name_, e.what());
		return;
	}

	assert(variant.gobj());

	if (m_isRange_) {

		// Use slider
		if (!m_sliderLayoutWidget_)
			return;

		for (unsigned int i = 0; i < m_values_.size(); i++) {
			const pair<Glib::VariantBase, QString>& v = m_values_[i];

			// g_variant_equal() doesn't handle floating point properly
			if (v.first.is_of_type(Glib::VariantType("d"))) {
				gdouble a, b;
				g_variant_get(variant.gobj(), "d", &a);
				g_variant_get((GVariant*)(v.first.gobj()), "d", &b);

				if (abs(a - b) <= 2 * DBL_EPSILON) {
					m_slider_->setValue(i);
					m_sliderLabel_->setText(v.second);
				}
			} else {
				// Check for "(dd)" type and handle it if it's found
				if (v.first.is_of_type(Glib::VariantType("(dd)"))) {
					gdouble a1, a2, b1, b2;
					g_variant_get(variant.gobj(), "(dd)", &a1, &a2);
					g_variant_get((GVariant*)(v.first.gobj()), "(dd)", &b1, &b2);

					if ((abs(a1 - b1) <= 2 * DBL_EPSILON) && (abs(a2 - b2) <= 2 * DBL_EPSILON)) {
						m_slider_->setValue(i);
						m_sliderLabel_->setText(v.second);
					}

				} else {
					qWarning() << "Enum property" << name() << "encountered unsupported type";
					return;
				}
			}
		}

	} else {
		// Use combo box
		if (!m_selector_)
			return;

		for (unsigned int i = 0; i < m_values_.size(); i++) {
			const pair<Glib::VariantBase, QString>& v = m_values_[i];

			// g_variant_equal() doesn't handle floating point properly
			if (v.first.is_of_type(Glib::VariantType("d"))) {
				gdouble a, b;
				g_variant_get(variant.gobj(), "d", &a);
				g_variant_get((GVariant*)(v.first.gobj()), "d", &b);
				if (abs(a - b) <= 2 * DBL_EPSILON)
					m_selector_->setCurrentIndex(i);
			} else {
				// Check for "(dd)" type and handle it if it's found
				if (v.first.is_of_type(Glib::VariantType("(dd)"))) {
					gdouble a1, a2, b1, b2;
					g_variant_get(variant.gobj(), "(dd)", &a1, &a2);
					g_variant_get((GVariant*)(v.first.gobj()), "(dd)", &b1, &b2);
					if ((abs(a1 - b1) <= 2 * DBL_EPSILON) && (abs(a2 - b2) <= 2 * DBL_EPSILON))
						m_selector_->setCurrentIndex(i);

				} else
					// Handle all other types
					if (v.first.equal(variant))
					m_selector_->setCurrentIndex(i);
			}
		}
	}
}

void Enum::commit()
{
	assert(m_setter_);

	if (m_isRange_) {
		// Use slider
		if (!m_sliderLayoutWidget_)
			return;

		m_setter_(m_values_.at(m_slider_->value()).first);

		updateWidget();
	} else {
		// Use combo box
		if (!m_selector_)
			return;

		const int index = m_selector_->currentIndex();
		if (index < 0)
			return;

		m_setter_(m_selector_->itemData(index).value<Glib::VariantBase>());

		// The combo box needs no update, it already shows the current value
		// by definition: the user picked it
	}
}

void Enum::onCurrentIndexChanged(int) { commit(); }

void Enum::onValueChanged(int) { commit(); }

} // namespace prop
} // namespace scopy
