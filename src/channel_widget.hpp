#ifndef CHANNEL_WIDGET_HPP
#define CHANNEL_WIDGET_HPP

#include <QWidget>
#include <QString>
#include <QAbstractButton>

namespace Ui {
class Channel;
}

namespace adiscope {

class ChannelWidget: public QWidget
{
	Q_OBJECT

public:
	explicit ChannelWidget(int id, bool deletable, bool simplified,
		QColor color, QWidget *parent = 0);
	~ChannelWidget();

	QAbstractButton* enableButton() const;
	QAbstractButton* nameButton() const;
	QAbstractButton* menuButton() const;
	QAbstractButton* deleteButton() const;

	int id() const;
	void setId(int);

	QColor color() const;

	QString fullName() const;
	void setFullName(const QString&);

	QString shortName() const;
	void setShortName(const QString&);

Q_SIGNALS:
	void enabled(bool en);
	void selected(bool on);
	void menuToggled(bool checked);
	void deleteClicked();

public Q_SLOTS:
	void setColor(QColor color);

private Q_SLOTS:
	void on_box_toggled(bool);
	void on_name_toggled(bool);
	void on_btn_toggled(bool);
	void on_delBtn_clicked();

private:
	Ui::Channel *m_ui;
	int m_id;
	bool m_deletable;
	bool m_simplified;
	QColor m_color;
	QString m_fullName;
	QString m_shortName;

	void init();
	void setButtonNoGroup(QAbstractButton *btn);
};

} /* namespace adiscope */

#endif // CHANNEL_WIDGET_HPP
