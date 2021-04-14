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
#ifndef CHANNEL_WIDGET_HPP
#define CHANNEL_WIDGET_HPP

#include <QAbstractButton>
#include <QString>
#include <QWidget>

namespace Ui {
class Channel;
}

namespace scopy {
namespace gui {

class ChannelWidget : public QWidget
{
	Q_OBJECT

public:
	explicit ChannelWidget(int id, bool deletable, bool simplified, QColor color, QWidget* parent = 0);
	~ChannelWidget();

	QAbstractButton* enableButton();
	QAbstractButton* nameButton();
	QAbstractButton* menuButton();
	QAbstractButton* deleteButton();

	int id() const;
	void setId(int);

	QColor color() const;

	QString fullName() const;
	void setFullName(const QString&);

	QString shortName() const;
	void setShortName(const QString&);

	QString function() const;
	void setFunction(const QString&);

	bool isMathChannel() const;
	void setMathChannel(const bool&);

	bool isReferenceChannel() const;
	void setReferenceChannel(const bool&);

protected:
	bool eventFilter(QObject* object, QEvent* event) override;

Q_SIGNALS:
	void enabled(bool en);
	void selected(bool on);
	void menuToggled(bool checked);
	void deleteClicked();

public Q_SLOTS:
	void setColor(QColor color);

private Q_SLOTS:
	void on_checkBox_toggled(bool);
	void on_btnName_toggled(bool);
	void on_btn_toggled(bool);
	void on_btnDel_clicked();

private:
	Ui::Channel* m_ui;
	int m_id;
	bool m_deletable;
	bool m_simplified;
	QColor m_color;
	QString m_fullName;
	QString m_shortName;
	bool m_math;
	QString m_function;
	bool m_ref;
	QString m_stylesheet;

	void init();
	void setButtonNoGroup(QAbstractButton* btn);
};

} // namespace gui
} // namespace scopy

#endif // CHANNEL_WIDGET_HPP
