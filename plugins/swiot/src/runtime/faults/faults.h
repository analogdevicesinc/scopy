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

#ifndef FAULTS_H
#define FAULTS_H

#include "faultspage.h"
#include "pluginbase/toolmenuentry.h"

#include "ui_swiotfaults.h"

#include <iio.h>

#include <gui/channel_manager.hpp>
#include <gui/generic_menu.hpp>
#include <gui/tool_view.hpp>

namespace scopy::swiot {
class Faults : public QWidget
{
	Q_OBJECT
public:
	explicit Faults(struct iio_context *ctx, ToolMenuEntry *tme, QWidget *parent = nullptr);
	~Faults();

	void pollFaults();

Q_SIGNALS:
	void backBtnPressed();

public Q_SLOTS:
	void runButtonClicked(bool toggled);
	void singleButtonClicked();

	void externalPowerSupply(bool ps);

private Q_SLOTS:
	void onBackBtnPressed();

private:
	void connectSignalsAndSlots();
	void setupDynamicUi(QWidget *parent);
	void initTutorialProperties();
	static QPushButton *createBackButton();

	struct iio_context *ctx;

	Ui::Faults *ui;
	QPushButton *m_backButton;
	QLabel *m_statusLabel;
	QWidget *m_statusContainer;

	QTimer *timer;
	QThread *thread;

	uint32_t ad74413r_numeric;
	uint32_t max14906_numeric;

	scopy::gui::ToolView *m_toolView{};
	FaultsPage *m_faultsPage;

	ToolMenuEntry *m_tme;
};
} // namespace scopy::swiot
#endif // FAULTS_H
