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
#include <QLineEdit>
#include <QSpinBox>

#include <cassert>
#include <scopy/gui/logicanalyzer/prop/string.hpp>

using std::string;

using Glib::ustring;

namespace scopy {
namespace prop {

String::String(QString name, QString desc, Getter getter, Setter setter)
	: Property(name, desc, getter, setter)
	, m_lineEdit_(nullptr)
{}

QWidget* String::getWidget(QWidget* parent, bool auto_commit)
{
	if (m_lineEdit_)
		return m_lineEdit_;

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

	m_lineEdit_ = new QLineEdit(parent);

	updateWidget();

	if (auto_commit)
		connect(m_lineEdit_, SIGNAL(textEdited(const QString&)), this, SLOT(onTextEdited(const QString&)));

	return m_lineEdit_;
}

void String::updateWidget()
{
	if (!m_lineEdit_)
		return;

	Glib::VariantBase variant;

	try {
		variant = m_getter_();
	} catch (const std::exception& e) {
		qWarning() << tr("Querying config key %1 resulted in %2").arg(m_name_, e.what());
		return;
	}

	assert(variant.gobj());

	string value = Glib::VariantBase::cast_dynamic<Glib::Variant<ustring>>(variant).get();

	m_lineEdit_->setText(QString::fromStdString(value));
}

void String::commit()
{
	assert(m_setter_);

	if (!m_lineEdit_)
		return;

	QByteArray ba = m_lineEdit_->text().toLocal8Bit();
	m_setter_(Glib::Variant<ustring>::create(ba.data()));
}

void String::onTextEdited(const QString&) { commit(); }

} // namespace prop
} // namespace scopy
