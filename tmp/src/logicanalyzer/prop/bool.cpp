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


#include <cassert>

#include <QCheckBox>
#include <QDebug>

#include "bool.hpp"

namespace adiscope {
namespace prop {

Bool::Bool(QString name, QString desc, Getter getter, Setter setter) :
	Property(name, desc, getter, setter),
	check_box_(nullptr)
{
}

QWidget* Bool::get_widget(QWidget *parent, bool auto_commit)
{
	if (check_box_)
		return check_box_;

	if (!getter_)
		return nullptr;

	try {
		QVariant variant = getter_();
		if (!variant.isValid())
			return nullptr;
    } catch (const std::exception &e) {
		qWarning() << tr("Querying config key %1 resulted in %2").arg(name_, e.what());
		return nullptr;
	}

	check_box_ = new QCheckBox(name(), parent);
	check_box_->setToolTip(desc());

	update_widget();

	if (auto_commit)
		connect(check_box_, SIGNAL(stateChanged(int)),
			this, SLOT(on_state_changed(int)));

	return check_box_;
}

bool Bool::labeled_widget() const
{
	return true;
}

void Bool::update_widget()
{
	if (!check_box_)
		return;

	QVariant variant;

	try {
		variant = getter_();
    } catch (const std::exception &e) {
		qWarning() << tr("Querying config key %1 resulted in %2").arg(name_, e.what());
		return;
	}

	assert(variant.isValid());
	bool value = variant.toBool();
	check_box_->setCheckState(value ? Qt::Checked : Qt::Unchecked);
}

void Bool::commit()
{
	assert(setter_);

	if (!check_box_)
		return;

	setter_(QVariant(check_box_->checkState() == Qt::Checked));
}

void Bool::on_state_changed(int)
{
	commit();
}

}  // namespace prop
}  // namespace pv
