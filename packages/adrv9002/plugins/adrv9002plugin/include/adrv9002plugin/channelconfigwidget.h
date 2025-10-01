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

#ifndef CHANNELCONFIGWIDGET_H
#define CHANNELCONFIGWIDGET_H

#include "scopy-adrv9002plugin_export.h"
#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QCheckBox>
#include <QComboBox>
#include <QString>
#include <QStringList>
#include <style.h>

namespace scopy::adrv9002 {

class SCOPY_ADRV9002PLUGIN_EXPORT ChannelConfigWidget : public QWidget
{
	Q_OBJECT

public:
	enum ChannelMode
	{
		RX_MODE,
		TX_MODE
	};

	// Channel data structure - unified for all channels
	struct ChannelData
	{
		bool enabled = true;
		bool freqOffsetCorrection = false;
		QString bandwidth = "38000000";
		QString sampleRate = "61440000";
		QString rfInput = "Rx1A"; // Only used for RX channels
	};

	explicit ChannelConfigWidget(const QString &title, ChannelMode mode, const QStringList &rfInputOptions = {},
				     QWidget *parent = nullptr);
	~ChannelConfigWidget();

	// Data interface
	void setChannelData(const ChannelData &data);
	ChannelData getChannelData() const;

	// Mode control for LTE vs Live Device behavior
	void updateControlsVisibility(bool lteMode);
	void setBandwidthReadOnly(bool readOnly);
	void setSampleRateOptions(const QStringList &options, bool forceValue = false);
	void setBandwidthOptions(const QStringList &options, bool forceValue = false);

	// Access to individual controls for external connections
	QComboBox *getSampleRateCombo() const { return m_sampleRateCombo; }

Q_SIGNALS:
	void channelDataChanged();
	void sampleRateChanged(const QString &newRate);
	void enabledChanged(bool enabled);

private Q_SLOTS:
	void onEnabledChanged();
	void onSampleRateChanged();
	void onBandwidthChanged();
	void onFreqOffsetChanged();

private:
	void setupUI();
	void connectSignals();

	// Configuration
	QString m_title;
	ChannelMode m_mode;
	QStringList m_rfInputOptions;

	// UI components
	QGridLayout *m_layout;
	QLabel *m_titleLabel;
	QCheckBox *m_enabledCb;
	QCheckBox *m_freqOffsetCb;
	QComboBox *m_bandwidthCombo;
	QComboBox *m_sampleRateCombo;
	QComboBox *m_rfInputCombo; // nullptr for TX channels

	// State
	bool m_updatingData;
	bool m_isLTEMode; // Track current mode for mode-aware control logic
};

} // namespace scopy::adrv9002

#endif // CHANNELCONFIGWIDGET_H
