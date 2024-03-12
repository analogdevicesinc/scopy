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

#ifndef SCOPY_DIODIGITALCHANNEL_H
#define SCOPY_DIODIGITALCHANNEL_H

#include "ui_diodigitalchannel.h"
#include "scopy-swiotrefactor_export.h"
#include <QDockWidget>
#include <QWidget>
#include <gui/generic_menu.hpp>

namespace scopy::swiotrefactor {
class SCOPY_SWIOTREFACTOR_EXPORT DioDigitalChannel : public QWidget
{
	Q_OBJECT
public:
	explicit DioDigitalChannel(const QString &deviceName, const QString &deviceType, QWidget *parent = nullptr);
	~DioDigitalChannel() override;

	void updateTimeScale(double newMax);
	void addDataSample(double value);
	const std::vector<std::string> getConfigModes() const;
	void setConfigModes(std::vector<std::string> &configModes);
	const QString &getSelectedConfigMode() const;
	void setSelectedConfigMode(const QString &selectedConfigMode);
	void resetSismograph();

private:
	void connectSignalsAndSlots();

	Ui::DioDigitalChannel *ui;
	QString m_deviceName;
	QString m_deviceType;

	std::vector<std::string> m_configModes;
	QString m_selectedConfigMode;

	friend class DioDigitalChannelController;

Q_SIGNALS:
	void outputValueChanged(bool value);
};
} // namespace scopy::swiotrefactor

#endif // SCOPY_DIODIGITALCHANNEL_H
