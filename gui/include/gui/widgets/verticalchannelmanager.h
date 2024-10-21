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

#ifndef VERTICALCHANNELMANAGER_H
#define VERTICALCHANNELMANAGER_H

#include <QVBoxLayout>
#include <QWidget>

#include <scopy-gui_export.h>
#include <compositewidget.h>

class QScrollArea;
namespace scopy {
class SCOPY_GUI_EXPORT VerticalChannelManager : public QWidget, public CompositeWidget
{
	Q_OBJECT
public:
	VerticalChannelManager(QWidget *parent = nullptr);
	~VerticalChannelManager();
	void add(QWidget *ch) override;
	void remove(QWidget *ch) override;
	void addEnd(QWidget *ch);

private:
	QSpacerItem *spacer;
	QVBoxLayout *lay;
	QVBoxLayout *m_contLayout;
	QScrollArea *m_scrollArea;
	QWidget *m_container;
};
} // namespace scopy

#endif // VERTICALCHANNELMANAGER_H
