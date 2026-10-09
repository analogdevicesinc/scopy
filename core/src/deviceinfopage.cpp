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

#include <component/context.h>
#include <component/attribute.h>
#include <component/backends/iio/iiocontext.h>

#include <QVBoxLayout>

using namespace scopy;

DeviceInfoPage::DeviceInfoPage(component::Context *ctx, QWidget *parent)
	: QWidget(parent)
	, m_infoPage(new InfoPage(this))
	, m_title(new QLabel("Device Info", this))
	, m_backendInfo(new QLabel(this))
{
	setupUi();
	setupInfoPage(ctx);
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

void DeviceInfoPage::setupInfoPage(component::Context *ctx)
{
	if(!ctx) {
		qWarning() << "Error, invalid context, cannot create info page.";
		return;
	}

	component::iio::IIOContext::Version v;
	if(auto *iioCtx = qobject_cast<component::iio::IIOContext *>(ctx)) {
		v = iioCtx->version();
	}
	QString backendText = "IIO context created with %1 backend.\n"
			      "Backend version: %2.%3 (git tag: %4)\n"
			      "Backend description string: %5";
	m_backendInfo->setText(backendText.arg(ctx->name())
				       .arg(QString::number(v.major))
				       .arg(QString::number(v.minor))
				       .arg(v.gitTag)
				       .arg(ctx->description()));

	// Context attributes are read-only component::Attribute children, their values cached
	// eagerly by the builder.
	const QList<component::Attribute *> attrs =
		ctx->findChildren<component::Attribute *>(Qt::FindDirectChildrenOnly);
	for(component::Attribute *a : attrs) {
		m_infoPage->update(a->name(), a->cachedValue());
	}
}

#include "moc_deviceinfopage.cpp"
