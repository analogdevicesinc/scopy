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

#ifndef GENALYZERSETTINGS_H
#define GENALYZERSETTINGS_H

#include "scopy-adc_export.h"
#include <gr-util/genalyzer.h>
#include <QWidget>
#include <QSpinBox>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>

namespace scopy {
namespace adc {

class SCOPY_ADC_EXPORT GenalyzerSettings : public QWidget
{
	Q_OBJECT
public:
	explicit GenalyzerSettings(QWidget *parent = nullptr);
	~GenalyzerSettings();

	grutil::GenalyzerConfig getConfig() const;
	void setConfig(const grutil::GenalyzerConfig& config);

Q_SIGNALS:
	void configChanged(const grutil::GenalyzerConfig& config);

private Q_SLOTS:
	void onUIChanged();
	void updateUIFromConfig();
	void enableAnalysis(bool en);

private:
	void setupUI();

	QComboBox *m_modeCombo;
	QSpinBox *m_ssbWidthSpinbox;
	QWidget *m_autoModeContainer;
	QWidget *m_fixedToneContainer;

	QLineEdit *m_expectedFreqEdit;
	QSpinBox *m_harmonicOrderSpinbox;
	QSpinBox *m_ssbFundamentalSpinbox;
	QSpinBox *m_ssbHarmonicsSpinbox;
	QSpinBox *m_ssbDefaultSpinbox;
	QCheckBox *m_coherentSamplingCheckbox;

	grutil::GenalyzerConfig m_config;
};

} // namespace adc
} // namespace scopy

#endif // GENALYZERSETTINGS_H
