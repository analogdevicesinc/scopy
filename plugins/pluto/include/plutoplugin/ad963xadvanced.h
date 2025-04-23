/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#ifndef AD963XADVANCED_H
#define AD963XADVANCED_H

#include <QBoxLayout>
#include <QPushButton>
#include <QWidget>
#include <tooltemplate.h>

namespace scopy {
namespace pluto {
class AD936XAdvanced : public QWidget
{
	Q_OBJECT
public:
	explicit AD936XAdvanced(QString uri, QWidget *parent = nullptr);
	~AD936XAdvanced();

signals:

private:
	QString m_uri;
	ToolTemplate *m_tool;
	QVBoxLayout *m_mainLayout;
	QPushButton *m_refreshButton;
};
} // namespace pluto
} // namespace scopy
#endif // AD963XADVANCED_H
