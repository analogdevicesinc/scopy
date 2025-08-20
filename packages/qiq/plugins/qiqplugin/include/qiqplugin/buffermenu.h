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

#ifndef BUFFERMENU_H
#define BUFFERMENU_H

#include "qiqutils.h"

#include <QWidget>
#include <QMap>
#include <QStringList>
#include <QSpinBox>
#include <menucombo.h>
#include <menuspinbox.h>

using namespace scopy::gui;

namespace scopy::qiqplugin {
class BufferMenu : public QWidget
{
	Q_OBJECT

public:
	BufferMenu(QWidget *parent = nullptr);
	~BufferMenu();

	void setAvailableChannels(const QMap<QString, QStringList> &channels);
	void updateChnList();

Q_SIGNALS:
	void bufferParamsChanged(BufferParams &params);

private Q_SLOTS:
	void onParamsChanged();

private:
	QStringList m_enChannels;
	QMap<QString, QStringList> m_availableChannels;
	MenuCombo *m_deviceCombo;
	MenuSpinbox *m_bufferSizeSpin;
	QWidget *m_chnList;

	void setupUI();
};

} // namespace scopy::qiqplugin

#endif // BUFFERMENU_H
