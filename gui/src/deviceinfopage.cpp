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
#include <iioutil/iiocpp/iiocontext.h>
#include <iioutil/iiocpp/iioattribute.h>
#include <iioutil/iiocpp/iioresult.h>
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
	const char *versionTag;

	major = IIOContext::get_version_major(m_conn->context());
	minor = IIOContext::get_version_minor(m_conn->context());
	versionTag = IIOContext::get_version_tag(m_conn->context());

	backendName = IIOContext::get_name(m_conn->context());
	description = IIOContext::get_description(m_conn->context());

	QString backendText = "IIO context created with %1 backend.\n"
			      "Backend version: %2.%3 (git tag: %4)\n"
			      "Backend description string: %5";
	m_backendInfo->setText(backendText.arg(backendName)
				       .arg(QString::number(major))
				       .arg(QString::number(minor))
				       .arg(versionTag)
				       .arg(description));

	const char *name;
	const char *value;
	for(int i = 0; i < IIOContext::get_attrs_count(m_conn->context()); ++i) {
		IIOResult<const iio_attr *> res = IIOContext::get_attr(m_conn->context(), i, &name, &value);
		if(!res.ok()) {
			qDebug() << "Error getting attribute at index" << i << "error code:" << res.error();
			continue;
		}

		// TODO: check that get_static_value_works
		name = IIOAttribute::get_name(res.data());
		value = IIOAttribute::get_static_value(res.data());

		m_infoPage->update(name, value);
	}
}

#include "moc_deviceinfopage.cpp"
