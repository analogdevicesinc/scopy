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


#ifndef INFO_PAGE_HPP
#define INFO_PAGE_HPP

#include <QWidget>
#include <QMap>
#include <QPushButton>

#include "iio.h"

namespace Ui {
class InfoPage;
}

namespace adiscope {
class Preferences;
class InfoPage : public QWidget
{
        Q_OBJECT

public:
        explicit InfoPage(QString uri,
                          Preferences* prefPanel,
                          struct iio_context *ctx = nullptr,
                          QWidget *parent = 0);
        ~InfoPage();


        struct iio_context *ctx() const;
        void setCtx(iio_context *ctx);

        QString uri() const;
        void setUri(QString uri);

        QPushButton *forgetDeviceButton();
        QPushButton *identifyDeviceButton();
        QPushButton *connectButton();

        void getDeviceInfo();
        void refreshInfoWidget();

        void setConnectionStatus(bool);

public Q_SLOTS:
        void readPreferences();

private:
        QPair<bool, QString> translateInfoParams(QString);

private:
        Ui::InfoPage *ui;
        QString m_uri;
        struct iio_context *m_ctx;
        bool m_advanced;
        Preferences *prefPanel;
        QMap<QString, QString> m_info_params;
        QMap<QString, QString> m_info_params_advanced;
};
}
#endif // INFO_PAGE_HPP
