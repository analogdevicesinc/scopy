#ifndef CHANNELMONITORCOMPONENT_H
#define CHANNELMONITORCOMPONENT_H

#include <QWidget>
#include <QResizeEvent>

namespace Ui {
class ChannelMonitorComponent;
}

namespace adiscope {
class ChannelMonitorComponent : public QWidget
{
	Q_OBJECT

public:
	explicit ChannelMonitorComponent(QWidget *parent = nullptr);
	~ChannelMonitorComponent();

	void init(double value, QString nameOfUnitOfMeasure,QString symbolOfUnitOfMeasure, QString title, QColor color);
	std::string getChannelId();
	void setChannelId(std::string channelId);
	void checkPeakValues(double value, QString unitOfMeasure);
	void updateUnitOfMeasure(QString unitOfMeasure);
	QString getUnitOfMeasure();

	QString getTitle();

	QColor getColor();
	void setColor(QColor color);

	void setID(int id);
	int getID();

Q_SIGNALS:
	void contentChanged();


public Q_SLOTS:
	void displayPeakHold(bool checked);
	void resetPeakHolder();
	void displayHistory(bool checked);
	void displayScale(bool checked);
	void updateLcdNumberPrecision(int precision);
	void setMonitorColor(QString color);
	void setHistoryDuration(int duration);
	void setRecordingInterval(double interval);
	void setLineStyle(Qt::PenStyle lineStyle);
	void updateValue(double value, QString nameOfUnitOfMeasure,QString symbolOfUnitOfMeasure);

private:
	Ui::ChannelMonitorComponent *ui;
	std::string m_channelId;
	double m_minValue;
	double m_maxValue;
	QString m_unitOfMeasure;
	QColor m_color;
	int m_id;
	void resizeEvent(QResizeEvent *event);
};
}
#endif // CHANNELMONITORCOMPONENT_H
