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

#include "deviceinfopage.h"
#include <QVBoxLayout>
#include <QDebug>

using namespace scopy;

DeviceInfoPage::DeviceInfoPage(Connection *conn, QWidget *parent)
	: QWidget(parent)
	, m_conn(conn)
	, m_infoPage(new InfoPage(this))
	, m_title(new QLabel("Device Info", this))
	, m_backendInfo(new QLabel(this))
{
	setupUi();
	setupInfoPage();
}

void DeviceInfoPage::setupUi()
{
	m_infoPage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	setLayout(new QVBoxLayout(this));
	m_title->setStyleSheet("font-weight: bold;");
	layout()->addWidget(m_backendInfo);
	layout()->addWidget(m_title);
	layout()->addWidget(m_infoPage);
}

void DeviceInfoPage::setupInfoPage()
{
	if(!m_conn) {
		qWarning() << "Error, invalid connection, cannot create info page.";
		return;
	}

	QString backendName, description;
	unsigned int major, minor;
	char git_tag[8] = "0000000";
	int ret = EXIT_FAILURE;
	ret = iio_context_get_version(m_conn->context(), &major, &minor, git_tag);
	if(ret) {
		qWarning() << "Error, cannot read backend version.";
		major = 'x';
		minor = 'x';
	}

	backendName = iio_context_get_name(m_conn->context());
	description = iio_context_get_description(m_conn->context());

	QString backendText = "IIO context created with %1 backend.\n"
			      "Backend version: %2.%3 (git tag: %4)\n"
			      "Backend description string: %5";
	m_backendInfo->setText(backendText.arg(backendName)
				       .arg(QString::number(major))
				       .arg(QString::number(minor))
				       .arg(git_tag)
				       .arg(description));

	const char *name;
	const char *value;
	for(int i = 0; i < iio_context_get_attrs_count(m_conn->context()); ++i) {
		int ret = iio_context_get_attr(m_conn->context(), i, &name, &value);
		if(ret != 0)
			continue;

		m_infoPage->update(name, value);
	}
}

#include "moc_deviceinfopage.cpp"
