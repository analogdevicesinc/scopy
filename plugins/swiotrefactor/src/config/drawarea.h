/*
 * Copyright (c) 2023 Analog Devices Inc.
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
 */

#ifndef TOOL_LAUNCHER_DRAWAREA_H
#define TOOL_LAUNCHER_DRAWAREA_H

#include <QLabel>
#include <QMap>
#include <QWidget>

namespace scopy::swiotrefactor {
class DrawArea : public QWidget
{
	Q_OBJECT

public:
	explicit DrawArea(QWidget *parent = nullptr);

	~DrawArea() override;

	enum ChannelName : int
	{
		AD74413R = 0,
		MAX14906 = 1
	};

	void activateConnection(int channelIndex, ChannelName channelName);
	void deactivateConnections();

private:
	QString m_filePath;
	QImage *m_boardImage;
	const QImage *m_baseImage;

	QLabel *m_drawLabel;

	// channel-device pairs
	QMap<QPair<int, int>, QPixmap *> *m_connectionsMap;
};
} // namespace scopy::swiotrefactor

#endif // TOOL_LAUNCHER_DRAWAREA_H
