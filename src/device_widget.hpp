/*
 * Copyright 2018 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef DEVICE_WIDGET_HPP
#define DEVICE_WIDGET_HPP

#include <QWidget>
#include <QPushButton>

#include "iio.h"

namespace Ui {
class Device;
}

namespace adiscope {
class InfoPage;
class ToolLauncher;

class DeviceWidget : public QWidget
{
        Q_OBJECT
public:
        explicit DeviceWidget(QString uri, int index = 0,
                              ToolLauncher *parent = nullptr);
        ~DeviceWidget();

        QPushButton* deviceButton() const;
        QPushButton* connectButton() const;

        void setChecked(bool);
        bool isChecked();

        QString uri() const;
        void setUri(const QString &uri);

        void highlightDevice();
        void click();

        int index() const;
        void setIndex(int index);

        InfoPage *infoPage() const;
        void setInfoPage(InfoPage *infoPage);

        bool connected() const;
        void setConnected(bool, bool, struct iio_context* ctx = nullptr);

        void setName(QString);

Q_SIGNALS:
        void selected(bool);
        void forgetDevice(int, QString);
        void identifyDevice(QString);

public Q_SLOTS:
        void on_btn_toggled(bool);
        void forgetDevice_clicked(bool);
        void identifyDevice_clicked(bool);

private:
        Ui::Device *m_ui;
        QString m_uri;
        int m_index;
        InfoPage *m_infoPage;
        bool m_connected;
        bool m_selected;
};

} /* namespace adiscope */

#endif // DEVICE_WIDGET_HPP
