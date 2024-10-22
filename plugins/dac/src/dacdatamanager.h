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

#ifndef DACDATAMANAGER_H
#define DACDATAMANAGER_H

#include <QWidget>
#include <QList>
#include <QPushButton>
#include <QBoxLayout>
#include <QString>
#include <QColor>

#include <gui/widgets/menucombo.h>
#include <gui/mapstackedwidget.h>
#include <gui/widgets/menucontrolbutton.h>

#include <iio.h>

namespace scopy {
namespace dac {
class DacDataModel;
class DacAddon;
class DacDataManager : public QWidget
{
	Q_OBJECT
public:
	typedef enum
	{
		DAC_DISABLED,
		DAC_BUFFER,
		DAC_DDS
	} DacMode;

	DacDataManager(struct iio_device *dev, QWidget *parent = nullptr);
	virtual ~DacDataManager();

	QString getName() const;
	QWidget *getWidget();

	QColor getColor() const;
	void setColor(QColor newColor);

	MapStackedWidget *getRightMenuStack() const;
	QList<MenuControlButton *> getMenuControlBtns() const;

	bool isBufferCapable() const;
	bool isDds() const;

	void toggleCyclicBuffer(bool toggled);
	void toggleBufferMode();
	void toggleDdsMode();
	void runToggled(bool toggled);
	bool isRunning();
private Q_SLOTS:
	void handleChannelMenuRequest(QString uuid);
Q_SIGNALS:
	void running(bool toggled);
	void requestMenu();

private:
	QVBoxLayout *m_layout;
	DacDataModel *m_model;
	QWidget *m_widget;
	QColor m_color;
	MapStackedWidget *dacAddonStack;
	MapStackedWidget *rightMenuStack;
	MenuCombo *m_mode;
	QList<MenuControlButton *> m_menuControlBtns;

	void setupDdsDac();
	void setupDacMode(QString mode_name, unsigned int mode);
	QWidget *createMenu();
	QWidget *createAttrMenu(QWidget *parent);
};
} // namespace dac
} // namespace scopy

#endif // DACDATAMANAGER_H
