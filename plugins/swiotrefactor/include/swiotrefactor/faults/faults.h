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

#include "scopy-swiotrefactor_export.h"
#include "faultspage.h"
#include <pluginbase/toolmenuentry.h>

#include <iio.h>
#include <gui/widgets/toolbuttons.h>

#include <gui/channel_manager.hpp>
#include <gui/generic_menu.hpp>
#include <gui/tool_view.hpp>
#include <gui/tooltemplate.h>

namespace scopy::swiotrefactor {
class SCOPY_SWIOTREFACTOR_EXPORT Faults : public QWidget
{
	Q_OBJECT
public:
	explicit Faults(QString uri, ToolMenuEntry *tme, QWidget *parent = nullptr);
	~Faults();

	void pollFaults();

Q_SIGNALS:
	void backBtnPressed();

public Q_SLOTS:
	void runButtonClicked(bool toggled);
	void singleButtonClicked();

private Q_SLOTS:
	void onBackBtnPressed();

private:
	void connectSignalsAndSlots();
	void initTutorialProperties();
	QPushButton *createConfigBtn(QWidget *parent);

	ToolTemplate *m_tool;
	RunBtn *m_runBtn;
	SingleShotBtn *m_singleBtn;
	QPushButton *m_configBtn;

	QTimer *m_timer;
	QThread *m_thread;

	FaultsPage *m_faultsPage;

	ToolMenuEntry *m_tme;
};
} // namespace scopy::swiotrefactor
#endif // FAULTS_H
