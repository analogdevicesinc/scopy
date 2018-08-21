/*
 * Copyright 2017 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef APIOBJECT_HPP
#define APIOBJECT_HPP

#include <QObject>

class QJSEngine;
class QSettings;
template <typename T> class QList;

namespace adiscope {
	class ApiObject : public QObject
	{
		Q_OBJECT

	public:
		ApiObject();
		~ApiObject();

		void load();
		void save();

		virtual void load(QSettings& settings);
		virtual void save(QSettings& settings);

		void js_register(QJSEngine *engine);

	private:
		template <typename T> void save(QSettings& settings,
				const QString& prop, const QList<T>& list);
		template <typename T> QList<T> load(QSettings& settings,
				const QString& prop);

		void save(QSettings& settings, const QString& prop,
				const QVariantList& list);
		void load(QSettings& settings, const QString& prop,
				const QVariantList& list);

		void save_nogroup(ApiObject *, QSettings&);
		void load_nogroup(ApiObject *, QSettings&);
	};
}

Q_DECLARE_METATYPE(adiscope::ApiObject *)

#endif /* APIOBJECT_HPP */
