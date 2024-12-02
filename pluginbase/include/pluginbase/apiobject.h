/*
 * Copyright (c) 2019 Analog Devices Inc.
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

#ifndef APIOBJECT_HPP
#define APIOBJECT_HPP

#include "scopy-pluginbase_export.h"

#include <QObject>

class QJSEngine;
class QSettings;
template <typename T>
class QList;

namespace scopy {
class SCOPY_PLUGINBASE_EXPORT ApiObject : public QObject
{
	Q_OBJECT

Q_SIGNALS:
	void loadingFinished();

public:
	ApiObject(QObject *parent = nullptr);
	~ApiObject();

	virtual void load(QSettings &settings);
	virtual void save(QSettings &settings);

private:
	template <typename T>
	void save(QSettings &settings, const QString &prop, const QList<T> &list);
	template <typename T>
	QList<T> load(QSettings &settings, const QString &prop);

	void save(QSettings &settings, const QString &prop, const QVariantList &list);
	void load(QSettings &settings, const QString &prop, const QVariantList &list);

	void save_nogroup(ApiObject *, QSettings &);
	void load_nogroup(ApiObject *, QSettings &);
};
} // namespace scopy

Q_DECLARE_METATYPE(scopy::ApiObject *)

#endif /* APIOBJECT_HPP */
