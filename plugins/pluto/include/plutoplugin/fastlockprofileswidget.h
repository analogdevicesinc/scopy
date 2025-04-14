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

#ifndef FASTLOCKPROFILESWIDGET_H
#define FASTLOCKPROFILESWIDGET_H

#include <QWidget>
#include "scopy-plutoplugin_export.h"

#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <iio.h>

namespace scopy {
namespace pluto {
class SCOPY_PLUTOPLUGIN_EXPORT FastlockProfilesWidget : public QWidget
{
	Q_OBJECT
public:
	explicit FastlockProfilesWidget(iio_channel *chn, QWidget *parent = nullptr);

	QComboBox *fastlockProfiles() const;

	QString title() const;
	void setTitle(QString *newTitle);

Q_SIGNALS:
	void recallCalled();

private:
	QLabel *m_title;
	QComboBox *m_fastlockProfiles;
	QPushButton *m_storeBtn;
	QPushButton *m_recallBtn;
};
} // namespace pluto
} // namespace scopy
#endif // FASTLOCKPROFILESWIDGET_H
