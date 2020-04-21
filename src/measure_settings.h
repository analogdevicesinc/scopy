#ifndef MEASURE_SETTINGS_H
#define MEASURE_SETTINGS_H

#include <QWidget>

#include <memory>

namespace Ui {
class MeasureSettings;
}

class QStandardItem;
class QStandardItemModel;

namespace adiscope {

class CapturePlot;
class DropdownSwitchList;
class MeasurementData;

class MeasurementItem {
public:
	MeasurementItem(int id = 0, int chn_id = 0)
		: m_id(id), m_chn_id(chn_id) {}

	int id() { return m_id; }
	int channel_id() { return m_chn_id; }

	bool operator==(const MeasurementItem &other) const {
		bool isEqual = (this->m_id == other.m_id) &&
			(this->m_chn_id == other.m_chn_id);

		return isEqual;
	}
	bool operator!=(const MeasurementItem &other) const {
		return !(*this == other);
	}

private:
	int m_id;
	int m_chn_id;
};

struct StatisticSelection {
	DropdownSwitchList *dropdown;
	MeasurementItem measurementItem;
};

class MeasureSettings : public QWidget {
	friend class Oscilloscope_API;

	Q_OBJECT

public:
	explicit MeasureSettings(CapturePlot *plot, QWidget *parent = 0);
	~MeasureSettings();

	QString channelName() const;
	void setChannelName(const QString &name);

	QColor channelUnderlineColor() const;
	void setChannelUnderlineColor(const QColor &color);

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
	void loadMeasurementStatesFromData();

Q_SIGNALS:
	void measurementActivated(int id, int chIdx);
	void measurementDeactivated(int id, int chIdx);
	void measurementSelectionListChanged();

	void statisticActivated(int id, int chIdx);
	void statisticDeactivated(int id, int chIdx);
	void statisticSelectionListChanged();
	void statisticsEnabled(bool en);
	void statisticsReset();

	void gatingEnabled(bool en);

public Q_SLOTS:
	void onChannelAdded(int);
	void onChannelRemoved(int);
	void setSelectedChannel(int);
	void onMeasurementActivated(int chnIdx, int id, bool en);
private Q_SLOTS:
	void onMeasurementPropertyChanged(QStandardItem *item);
	void on_button_measDisplayAll_toggled(bool checked);
	void on_button_measDeleteAll_toggled(bool checked);
	void onStatisticActivated(DropdownSwitchList *dropdown, int id,
				  bool en);
	void on_button_StatisticsEn_toggled(bool checked);
	void on_button_StatisticsReset_pressed();
	void on_button_statsDeleteAll_toggled(bool checked);
	void on_button_GatingEnable_toggled(bool checked);

private:
	void deleteAllMeasurements();
	void recoverAllMeasurements();
	void displayAllMeasurements();
	void disableDisplayAllMeasurements();
	void loadStatisticStatesForChannel(int chnIdx);
	void setColumnData(QStandardItemModel *model, int column, bool en);
	void setAllMeasurements(int col, bool en);
	void deleteMeasurementsOfChannel(QList<MeasurementItem> &list,
					 int chnIdx);
	void updateMeasurementsOnChannelDel(QList<MeasurementItem> &list,
					    int chnIdx);
	void deleteStatisticsOfChannel(QList<struct StatisticSelection> &list,
				       int chnIdx);
	void deleteAllStatistics();
	void recoverAllStatistics();
	void addStatistic(int measure_id, int ch_id);
	void removeStatistic(int measure_id, int ch_id);

private:
	Ui::MeasureSettings *m_ui;
	QString m_channelName;
	QColor m_chnUnderlineColor;
	DropdownSwitchList *m_horizMeasurements;
	DropdownSwitchList *m_vertMeasurements;
	bool m_emitActivated;
	bool m_emitStatsChanged;
	bool m_emitDeleteAll;
	bool m_emitStatsDeleteAll;
	bool m_are_dropdowns_filled;
	bool m_enableDisplayAll;

	CapturePlot *m_plot;
	int m_selectedChannel;
	QList<MeasurementItem> m_selectedMeasurements;
	QList<MeasurementItem> m_deleteAllBackup;
	QList<MeasurementItem> m_displayAllBackup;
	QList<int> m_measurePosInDropdowns;

	QList<struct StatisticSelection> m_selectedStatistics;
	QList<struct StatisticSelection> m_statsDeleteAllBackup;
};
} // namespace adiscope

#endif // MEASURE_SETTINGS_H
