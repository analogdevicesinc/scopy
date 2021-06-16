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

#ifndef PULSEVIEW_PV_PROP_PROPERTY_HPP
#define PULSEVIEW_PV_PROP_PROPERTY_HPP

#include <glib.h>
// Suppress warnings due to use of deprecated std::auto_ptr<> by glibmm.
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
#include <glibmm.h>
G_GNUC_END_IGNORE_DEPRECATIONS

#include <QString>
#include <QWidget>

#include <functional>

using std::function;

class QWidget;

namespace scopy {
namespace prop {

class Property : public QObject
{
	Q_OBJECT;

public:
	typedef function<Glib::VariantBase()> Getter;
	typedef function<void(Glib::VariantBase)> Setter;

protected:
	Property(QString name, QString desc, Getter getter, Setter setter);

public:
	const QString& name() const;
	const QString& desc() const;

	Glib::VariantBase get() const;
	void set(Glib::VariantBase val);

	virtual QWidget* getWidget(QWidget* parent, bool auto_commit = false) = 0;
	virtual bool labeledWidget() const;
	virtual void updateWidget() = 0;

	virtual void commit() = 0;

protected:
	const Getter m_getter_;
	const Setter m_setter_;

protected:
	QString m_name_;
	QString m_desc_;
};

} // namespace prop
} // namespace scopy

#endif // PULSEVIEW_PV_PROP_PROPERTY_HPP
