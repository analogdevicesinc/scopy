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

#include "scopy-sigrok-gui_export.h"

#include <QString>
#include <QVariant>
#include <QWidget>

#include <functional>

using std::function;

class QWidget;

namespace scopy {
namespace prop {

class SCOPY_SIGROK_GUI_EXPORT Property : public QObject
{
	Q_OBJECT;

public:
	typedef function<QVariant()> Getter;
	typedef function<void(QVariant)> Setter;

protected:
	Property(QString name, QString desc, Getter getter, Setter setter);

public:
	const QString &name() const;
	const QString &desc() const;

	QVariant get() const;
	void set(QVariant val);

	virtual QWidget *get_widget(QWidget *parent, bool auto_commit = false) = 0;
	virtual bool labeled_widget() const;
	virtual void update_widget() = 0;

	virtual void commit() = 0;

protected:
	const Getter getter_;
	const Setter setter_;

protected:
	QString name_;
	QString desc_;
};

} // namespace prop
} // namespace scopy

#endif // PULSEVIEW_PV_PROP_PROPERTY_HPP
