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

#ifndef MEASUREMENTSETTINGS_H
#define MEASUREMENTSETTINGS_H

#include "menusectionwidget.h"
#include "scopy-gui_export.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <menuonoffswitch.h>
#include <QSpinBox>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>

namespace scopy {
class SCOPY_GUI_EXPORT MeasurementSettings : public QWidget
{
	Q_OBJECT
public:
	typedef enum
	{
		MPM_SORT_CHANNEL,
		MPM_SORT_TYPE
	} MeasurementSortingType;
	MeasurementSettings(QWidget *parent = nullptr);
	~MeasurementSettings();

	bool measurementEnabled();
	bool statsEnabled();
	bool markerEnabled();
	bool genalyzerEnabled();
	uint8_t ssbWidth() const;  // Deprecated

	// Get complete genalyzer configuration as variant map
	QVariantMap getGenalyzerConfig() const;
	// Set configuration from external source
	void setGenalyzerConfig(const QVariantMap& config);

	MenuSectionWidget *getMarkerSection() const;

	MenuSectionWidget *getStatsSection() const;

	MenuSectionWidget *getMeasureSection() const;

	MenuSectionWidget *getGenalyzerSection() const;

Q_SIGNALS:
	void toggleAllMeasurements(bool);
	void toggleAllStats(bool);
	void sortMeasurements(MeasurementSortingType type);
	void sortStats(MeasurementSortingType type);
	void enableMeasurementPanel(bool b);
	void enableStatsPanel(bool b);
	void enableMarkerPanel(bool b);
	void enableGenalyzerPanel(bool b);
	void ssbWidthChanged(uint8_t value);  // Deprecated
	void genalyzerConfigChanged(const QVariantMap& config);

private Q_SLOTS:
	void onGenalyzerUIChanged();
	void updateUIFromConfig();

private:
	MenuOnOffSwitch *measurePanelSwitch;
	MenuOnOffSwitch *statsPanelSwitch;
	MenuOnOffSwitch *markerPanelSwitch;
	MenuOnOffSwitch *genalyzerPanelSwitch;
	QSpinBox *ssbWidthSpinbox;

	// Fixed tone UI controls
	QComboBox *modeCombo;
	QLineEdit *expectedFreqEdit;
	QSpinBox *harmonicOrderSpinbox;
	QSpinBox *ssbFundamentalSpinbox;
	QSpinBox *ssbHarmonicsSpinbox;
	QSpinBox *ssbDefaultSpinbox;
	QCheckBox *coherentSamplingCheckbox;

	// Store configuration values individually to avoid circular dependency
	int m_mode;  // 0 = AUTO, 1 = FIXED_TONE
	uint8_t m_auto_ssb_width;
	double m_expected_freq;
	int m_harmonic_order;
	int m_ssb_fundamental;
	int m_ssb_harmonics;
	int m_ssb_default;
	bool m_coherent_sampling;

	MenuSectionWidget *markerSection;
	MenuSectionWidget *statsSection;
	MenuSectionWidget *measureSection;
	MenuSectionWidget *genalyzerSection;
};
} // namespace scopy

#endif // MEASUREMENTSETTINGS_H
