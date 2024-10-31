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

#include <QWidget>
#include <QPushButton>
#include <QList>

namespace scopy {
class PathTitle : public QWidget
{
	Q_OBJECT
public:
	PathTitle(QWidget *parent = nullptr);
	PathTitle(QString title, QWidget *parent = nullptr);

	void setTitle(QString title);
	QString title() const;

Q_SIGNALS:
	void pathSelected(QString path);

private:
	void setupUi();
	QString m_titlePath;
};
} // namespace scopy
