/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef MEASURE_SETTINGS_H
#define MEASURE_SETTINGS_H

#include <memory>
#include <scopy/gui/generic_menu.hpp>

namespace Ui {
class MeasureSettings;
}

namespace scopy {
namespace gui {

class DropdownSwitchList;

class MeasurementItem
{
public:
	MeasurementItem(int id = 0, int chn_id = 0)
		: m_id(id)
		, m_chn_id(chn_id)
	{}

	int id() { return m_id; }
	int channel_id() { return m_chn_id; }

	bool operator==(const MeasurementItem& other) const
	{
		bool isEqual = (this->m_id == other.m_id) && (this->m_chn_id == other.m_chn_id);

		return isEqual;
	}
	bool operator!=(const MeasurementItem& other) const { return !(*this == other); }

private:
	int m_id;
	int m_chn_id;
};

struct StatisticSelection
{
	DropdownSwitchList* dropdown;
	MeasurementItem measurementItem;
};

class MeasureSettings : public GenericMenu
{
	Q_OBJECT

public:
	explicit MeasureSettings(GenericMenu* parent = 0);
	~MeasureSettings();

	QString channelName() const;
	void setChannelName(const QString& name);

	QColor channelUnderlineColor() const;
	void setChannelUnderlineColor(const QColor* color);

	void setHorizMeasurementActive(int idx, bool en);
	void setVertMeasurementActive(int idx, bool en);

	bool emitActivated() const;
	void setEmitActivated(bool en);

	bool emitStatsChanged() const;
	void setEmitStatsChanged(bool en);

	void disableDisplayAll();
	void activateDisplayAll();

	QList<MeasurementItem> measurementSelection();
	QList<MeasurementItem> statisticSelection();

	void setMenuButton(bool toggled) override;

private:
	void initUi();

private:
	Ui::MeasureSettings* m_ui;
	QString m_channelName;
	QColor m_chnUnderlineColor;
	DropdownSwitchList* m_horizMeasurements;
	DropdownSwitchList* m_vertMeasurements;
	bool m_emitActivated;
	bool m_emitStatsChanged;
	bool m_emitDeleteAll;
	bool m_emitStatsDeleteAll;
	bool m_are_dropdowns_filled;
	bool m_enableDisplayAll;

	int m_selectedChannel;
	QList<MeasurementItem> m_selectedMeasurements;
	QList<MeasurementItem> m_deleteAllBackup;
	QList<MeasurementItem> m_displayAllBackup;
	QList<int> m_measurePosInDropdowns;

	QList<struct StatisticSelection> m_selectedStatistics;
	QList<struct StatisticSelection> m_statsDeleteAllBackup;
};
} // namespace gui
} // namespace scopy

#endif // MEASURE_SETTINGS_H
