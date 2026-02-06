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

#ifndef PLOTLEGEND_H
#define PLOTLEGEND_H

#include <QLabel>
#include <QMap>
#include <QVBoxLayout>
#include <QWidget>
#include <plotwidget.h>
#include <scopy-gui_export.h>

namespace scopy {

class PlotChannel;
class HoverWidget;

class SCOPY_GUI_EXPORT PlotLegend : public QWidget
{
	Q_OBJECT
public:
	explicit PlotLegend(PlotWidget *plot, QWidget *parent = nullptr);
	virtual ~PlotLegend();

public Q_SLOTS:
	void setVisible(bool visible) override;

private Q_SLOTS:
	void addChannel(PlotChannel *ch);
	void removeChannel(PlotChannel *ch);
	void updateLineStyle(PlotChannel *ch);

private:
	void updateStyles();

	PlotWidget *m_plot;
	QWidget *m_legendPanel;
	QVBoxLayout *m_legendLayout;
	HoverWidget *m_hoverWidget;
	QMap<PlotChannel *, QWidget *> m_entries;
	QMap<PlotChannel *, QLabel *> m_labels;
	QMap<PlotChannel *, QWidget *> m_colorLines;
	bool m_visible;
};

} // namespace scopy

#endif // PLOTLEGEND_H
