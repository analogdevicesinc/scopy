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
 */

#ifndef AD9371_H
#define AD9371_H

#include "scopy-ad9371plugin_export.h"
#include <QBoxLayout>
#include <QGridLayout>
#include <QList>
#include <QPushButton>
#include <QTimer>
#include <QWidget>
#include <tooltemplate.h>
#include <iio-widgets/iiowidget.h>
#include <animatedrefreshbtn.h>
#include <gui/widgets/menuspinbox.h>
#include <gui/widgets/menusectionwidget.h>
#include <gui/widgets/menuonoffswitch.h>
#include "ad9371widgetfactory.h"

namespace scopy {
class IIOWidgetGroup;
}

namespace scopy::ad9371 {

class SCOPY_AD9371PLUGIN_EXPORT Ad9371 : public QWidget
{
	Q_OBJECT
	friend class Ad9371_API;

public:
	Ad9371(iio_context *ctx, IIOWidgetGroup *group = nullptr, QWidget *parent = nullptr);
	~Ad9371();

Q_SIGNALS:
	void readRequested();

private:
	iio_context *m_ctx = nullptr;
	IIOWidgetGroup *m_widgetGroup = nullptr;
	ToolTemplate *m_tool;
	QVBoxLayout *m_mainLayout;
	QWidget *m_centralWidget;
	QWidget *m_blockDiagramWidget = nullptr;
	AnimatedRefreshBtn *m_refreshButton;

	// Device pointers
	iio_device *m_dev = nullptr;
	iio_device *m_dds = nullptr;
	iio_device *m_cap = nullptr;

	// Live-refresh timer and widgets (RSSI + hardware gain)
	QTimer *m_liveTimer = nullptr;
	QList<IIOWidget *> m_liveWidgets;

	// Cross-connected widgets (FPGA freq -> sampling rate labels)
	IIOWidget *m_fpgaTxFreqCombo = nullptr;
	IIOWidget *m_fpgaRxFreqCombo = nullptr;
	IIOWidget *m_rxSampRateLabel = nullptr;
	IIOWidget *m_txSampRateLabel = nullptr;

	// Up/Down Converter (ADF4351)
	iio_device *m_udcRx = nullptr;
	iio_device *m_udcTx = nullptr;
	bool m_hasUdc = false;
	bool m_udcEnabled = false;
	IIOWidget *m_rxLoWidget = nullptr;
	IIOWidget *m_txLoWidget = nullptr;

	// Feature flags
	bool m_is2Rx2Tx = false;
	bool m_hasDpd = false;

	// Phase rotation spinboxes (computed from calibscale/calibphase, not IIOWidgets)
	gui::MenuSpinbox *m_phaseSpinbox1 = nullptr;
	gui::MenuSpinbox *m_phaseSpinbox2 = nullptr;

	// Calibration switches (MenuOnOffSwitch, not IIOWidgets)
	MenuOnOffSwitch *m_calRxQec = nullptr;
	MenuOnOffSwitch *m_calTxQec = nullptr;
	MenuOnOffSwitch *m_calTxLol = nullptr;
	MenuOnOffSwitch *m_calTxLolExt = nullptr;
	MenuOnOffSwitch *m_calDpd = nullptr;
	MenuOnOffSwitch *m_calClgc = nullptr;
	MenuOnOffSwitch *m_calVswr = nullptr;

	void setupUi();
	void detectDevices();

	// Section generators
	QWidget *generateGlobalSettingsWidget(QString title, QWidget *parent);
	QWidget *generateRxChainWidget(QString title, QWidget *parent);
	QWidget *generateTxChainWidget(QString title, QWidget *parent);
	QWidget *generateObsRxChainWidget(QString title, QWidget *parent);
	QWidget *generateFpgaSettingsWidget(QString title, QWidget *parent);

	// Helper to resolve LO frequency attribute name
	const char *resolveFreqAttrName(iio_channel *ch, const char *fallback);

	// Up/Down Converter helpers
	void onUdcToggled(bool enabled);
	void updateLoConversion(IIOWidget *loWidget, bool isRx);

	// Calibration read/write
	void readCalibrationFromHardware();
	void writeCalibrationToHardware();

private Q_SLOTS:
	void loadProfileFromFile(QString filePath);

	// FPGA Phase Rotation Helpers
	void writePhase(iio_device *fpgaDev, int channelIndex, int degrees);
	void readPhase(iio_device *fpgaDev, int channelIndex, gui::MenuSpinbox *spinBox);
};

} // namespace scopy::ad9371
#endif // AD9371_H
