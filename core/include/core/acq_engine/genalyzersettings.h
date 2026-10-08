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

#pragma once

#include "scopy-core_export.h"
#include "genalyzerconfig.h"

#include <QWidget>

class QSpinBox;
class QComboBox;
class QLineEdit;

namespace scopy {
namespace acq {

class SCOPY_CORE_EXPORT GenalyzerSettings : public QWidget
{
	Q_OBJECT
public:
	explicit GenalyzerSettings(QWidget *parent = nullptr);
	~GenalyzerSettings() override;

	GenalyzerConfig getConfig() const;
	void setConfig(const GenalyzerConfig &config);
	void enableAnalysis(bool en);

Q_SIGNALS:
	void configChanged(const scopy::acq::GenalyzerConfig &config);

private Q_SLOTS:
	void onUIChanged();
	void updateUIFromConfig();

private:
	void setupUI();

	QComboBox *m_modeCombo{nullptr};
	QSpinBox *m_ssbWidthSpinbox{nullptr};
	QWidget *m_autoModeContainer{nullptr};
	QWidget *m_fixedToneContainer{nullptr};

	QLineEdit *m_expectedFreqEdit{nullptr};
	QSpinBox *m_harmonicOrderSpinbox{nullptr};
	QSpinBox *m_ssbFundamentalSpinbox{nullptr};
	QSpinBox *m_ssbDefaultSpinbox{nullptr};

	GenalyzerConfig m_config;
};

} // namespace acq
} // namespace scopy
