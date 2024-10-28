/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef APPLICATIONRESTARTER_H
#define APPLICATIONRESTARTER_H

#include "scopy-core_export.h"

#include <QString>
#include <QStringList>

namespace scopy {
class SCOPY_CORE_EXPORT ApplicationRestarter
{
public:
	ApplicationRestarter(const QString &executable);

	static ApplicationRestarter *GetInstance();
	void setArguments(const QStringList &arguments);
	QStringList getArguments() const;

	int restart(int exitCode);
	static void triggerRestart();

private:
	static ApplicationRestarter *pinstance_;
	QString m_executable;
	QStringList m_arguments;
	QString m_currentPath;
	bool m_restart;
};
} // namespace scopy
#endif // APPLICATIONRESTARTER_H
