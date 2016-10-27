#ifndef MEASURE_SETTINGS_H
#define MEASURE_SETTINGS_H

#include <QWidget>

namespace Ui {
	class MeasureSettings;
}

class QStandardItem;
class QStandardItemModel;

namespace adiscope {

class DropdownSwitchList;

class MeasureSettings : public QWidget
{
	Q_OBJECT
public:
	explicit MeasureSettings(QWidget *parent = 0);

	QString channelName() const;
	void setChannelName(const QString& name);

	QColor channelUnderlineColor() const;
	void setChannelUnderlineColor(const QColor& color);

	void addHorizontalMeasurement(const QString& name);
	void addVerticalMeasurement(const QString& name);

	void setHorizMeasurementActive(int idx, bool en);
	void setVertMeasurementActive(int idx, bool en);

	bool emitActivated() const;
	void setEmitActivated(bool en);

	bool emitStatsChanged()const;
	void setEmitStatsChanged(bool en);

Q_SIGNALS:
	void measurementActiveChanged(const QString& name, bool en);
	void measurementStatsChanged(const QString& name, bool en);
	void measurementDeleteAllOrRecover(bool);

private Q_SLOTS:
	void onMeasurementPropertyChanged(QStandardItem *item);
	void on_button_MeasurementsEn_toggled(bool checked);
	void on_button_measDeleteAll_toggled(bool checked);

private:
	void setColumnData(QStandardItemModel *model, int column, bool en);
	void setAllMeasurements(int col, bool en);

private:
	Ui::MeasureSettings *m_ui;
	QString m_channelName;
	QColor m_chnUnderlineColor;
	DropdownSwitchList *m_horizMeasurements;
	DropdownSwitchList *m_vertMeasurements;
	bool m_emitActivated;
	bool m_emitStatsChanged;
	bool m_emitDeleteAll;

};
} // namespace adiscope

#endif // MEASURE_SETTINGS_H
