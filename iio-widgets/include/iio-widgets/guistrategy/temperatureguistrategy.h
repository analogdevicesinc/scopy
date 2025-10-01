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

#ifndef SCOPY_TEMPERATUREGUISTRATEGY_H
#define SCOPY_TEMPERATUREGUISTRATEGY_H

#include "scopy-iio-widgets_export.h"
#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QTimer>
#include <QHBoxLayout>
#include <guistrategy/guistrategyinterface.h>
#include <infoiconwidget.h>

namespace scopy {

enum class TemperatureUnit
{
	Celsius,
	Fahrenheit,
	Kelvin
};

enum class WarningState
{
	Normal,
	Approaching,
	Critical
};

class SCOPY_IIO_WIDGETS_EXPORT TemperatureGuiStrategy : public QObject, public GuiStrategyInterface
{
	Q_OBJECT
	Q_INTERFACES(scopy::GuiStrategyInterface)

public:
	explicit TemperatureGuiStrategy(IIOWidgetFactoryRecipe recipe, bool isCompact = false,
					QWidget *parent = nullptr);
	~TemperatureGuiStrategy();

	// GuiStrategyInterface implementation
	QWidget *ui() override;
	bool isValid() override;
	void setCustomTitle(QString title) override;
	void setInfoMessage(QString infoMessage) override;

	// Simple API - Basic temperature monitoring
	void enableMonitoring(bool enabled = true);

	// Advanced configuration API - Fine-grained control
	void setCriticalTemperature(double tempCelsius, const QString &message = "Critical temperature reached!");
	void setWarningOffset(double offsetCelsius = 5.0);
	void setPeriodicUpdates(bool enabled, int intervalSeconds = 5);
	void clearWarning();

Q_SIGNALS:
	// GuiStrategyInterface signals
	void displayedNewData(QString data, QString optionalData) override;
	void emitData(QString data) override;
	void requestData() override;

	// Temperature-specific signals
	void approachingCriticalTemperature(double temperature, double warningThreshold, double criticalThreshold);
	void criticalTemperatureReached(double temperature, const QString &message);
	void temperatureUpdated(double temperature, TemperatureUnit unit);

public Q_SLOTS:
	// GuiStrategyInterface slots
	void receiveData(QString currentData, QString optionalData) override;

private Q_SLOTS:
	void onUnitChanged();
	void onPeriodicUpdate();
	void updateWarningState(double celsius);

private:
	// UI widget
	QWidget *m_ui;

	// GuiStrategyInterface recipe
	IIOWidgetFactoryRecipe m_recipe;

	// UI components
	QLabel *m_titleLabel;
	QLabel *m_temperatureLabel;
	QComboBox *m_unitCombo;
	InfoIconWidget *m_infoIcon;
	QHBoxLayout *m_mainLayout;

	// Update timer
	QTimer *m_updateTimer;

	// Warning system state
	bool m_warningEnabled;
	WarningState m_warningState;
	double m_criticalTempCelsius;
	double m_warningOffsetCelsius;
	QString m_warningMessage;
	void updateInfoIconForWarning();

	// Current state
	double m_lastTemperatureCelsius;
	TemperatureUnit m_currentUnit;

	// Helper functions
	void setupUI();
	void updateUI();
	double convertTemperature(double celsius, TemperatureUnit toUnit) const;

	// Data parsing
	double parseTemperatureData(const QString &data);
};

} // namespace scopy

#endif // SCOPY_TEMPERATUREGUISTRATEGY_H
