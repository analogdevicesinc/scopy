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

#include <QDebug>
#include <QLineEdit>
#include <QSpinBox>

#include "string.hpp"

using std::string;

namespace adiscope {
namespace prop {

String::String(QString name,
	QString desc,
	Getter getter,
	Setter setter) :
	Property(name, desc, getter, setter),
	line_edit_(nullptr)
{
}

QWidget* String::get_widget(QWidget *parent, bool auto_commit)
{
	if (line_edit_)
		return line_edit_;

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

	line_edit_ = new QLineEdit(parent);

	update_widget();

	if (auto_commit)
		connect(line_edit_, SIGNAL(textEdited(const QString&)),
			this, SLOT(on_text_edited(const QString&)));

	return line_edit_;
}

void String::update_widget()
{
	if (!line_edit_)
		return;

	QVariant variant;

	try {
		variant = getter_();
    } catch (const std::exception &e) {
		qWarning() << tr("Querying config key %1 resulted in %2").arg(name_, e.what());
		return;
	}

	assert(variant.isValid());

	QString value = variant.toString();
	line_edit_->setText(value);
}

void String::commit()
{
	assert(setter_);

	if (!line_edit_)
		return;

	QString ba = line_edit_->text();
	setter_(QVariant(ba));
}

void String::on_text_edited(const QString&)
{
	commit();
}

}  // namespace prop
}  // namespace pv
