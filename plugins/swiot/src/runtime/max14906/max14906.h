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

#ifndef MAX14906TOOL_H
#define MAX14906TOOL_H

#include "diocontroller.h"
#include "diodigitalchannelcontroller.h"
#include "diosettingstab.h"
#include "pluginbase/toolmenuentry.h"
#include "src/runtime/readerthread.h"

#include "ui_max14906.h"

#include <gui/flexgridlayout.hpp>
#include <gui/tool_view.hpp>

namespace scopy::swiot {
#define MAX_NAME "max14906"
class DioDigitalChannel;
class DioController;

class Max14906 : public QWidget
{
	Q_OBJECT
public:
	explicit Max14906(struct iio_context *ctx, ToolMenuEntry *tme, QWidget *parent = nullptr);
	~Max14906() override;

Q_SIGNALS:
	void backBtnPressed();

public Q_SLOTS:
	void externalPowerSupply(bool ps);

private Q_SLOTS:
	void runButtonToggled();

	void timerChanged(double value);

	void onBackBtnPressed();

private:
	static QPushButton *createBackButton();
	void initChannels();
	void setupDynamicUi(QWidget *parent);
	void initMonitorToolView();
	scopy::gui::GenericMenu *createGeneralSettings(const QString &title, QColor *color);
	void connectSignalsAndSlots();

	static QFrame *createVLine(QWidget *parent);
	static QFrame *createHLine(QWidget *parent);
	static QMainWindow *createDockableMainWindow(const QString &title, DioDigitalChannel *digitalChannel,
						     QWidget *parent);

	struct iio_context *m_ctx;

	QPushButton *m_backButton;
	QWidget *m_statusContainer;
	QLabel *m_statusLabel;
	int m_nbDioChannels;

	DioController *max14906ToolController;
	Ui::Max14906 *ui;
	DioSettingsTab *m_max14906SettingsTab;
	scopy::gui::ToolView *m_toolView;
	scopy::gui::GenericMenu *m_generalSettingsMenu;
	scopy::gui::SubsectionSeparator *settingsWidgetSeparator;
	QTimer *m_qTimer;

	CommandQueue *m_cmdQueue;
	ReaderThread *m_readerThread;
	QMap<int, DioDigitalChannelController *> m_channelControls;

	ToolMenuEntry *m_tme;
};
} // namespace scopy::swiot
#endif // MAX14906TOOL_H
