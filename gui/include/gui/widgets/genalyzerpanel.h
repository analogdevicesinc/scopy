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

#ifndef GENALYZERPANEL_H
#define GENALYZERPANEL_H

#include "scopy-gui_export.h"
#include "utils.h"
#include "measurementpanel.h"

#include <QGridLayout>
#include <QScrollBar>
#include <QWidget>
#include <QLabel>
#include <QPen>
#include <QScrollArea>
#include <QHBoxLayout>
#include <QColor>
#include <QTextBrowser>

namespace scopy {

class SCOPY_GUI_EXPORT GenalyzerChannelDisplay : public QTextBrowser
{
	Q_OBJECT
public:
	GenalyzerChannelDisplay(const QString &channelName, QColor channelColor, QWidget *parent = nullptr);
	void updateResults(size_t results_size, char **rkeys, double *rvalues);
	void setChannelColor(QColor color);
	QString channelName() const;

private:
	QString formatResultsText(size_t results_size, char **rkeys, double *rvalues);

	QString m_channelName;
	QColor m_channelColor;
};

class SCOPY_GUI_EXPORT GenalyzerPanel : public QWidget
{
	Q_OBJECT
	QWIDGET_PAINT_EVENT_HELPER
public:
	GenalyzerPanel(QWidget *parent = nullptr);
	~GenalyzerPanel();

public Q_SLOTS:
	void updateResults(const QString &channelName, QColor channelColor, size_t results_size, char **rkeys,
			   double *rvalues);
	void clear();
	void clearChannel(const QString &channelName);

private:
	GenalyzerChannelDisplay *findOrCreateChannelDisplay(const QString &channelName, QColor channelColor);

	QVBoxLayout *panelLayout;
	QMap<QString, GenalyzerChannelDisplay *> m_channelDisplays;
	QScrollArea *scrollArea;
	QWidget *m_panel;
};

} // namespace scopy
#endif // GENALYZERPANEL_H
