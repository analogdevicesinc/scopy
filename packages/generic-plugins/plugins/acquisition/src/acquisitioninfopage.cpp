/*
 * Copyright (c) 2026 Analog Devices Inc.
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

#include "acquisitioninfopage.h"

#include <QDebug>
#include <QVBoxLayout>

#include <component/attribute.h>
#include <component/attributereader.h>
#include <component/context.h>
#include <component/device.h>

using namespace scopy;
using namespace scopy::acquisition;

AcquisitionInfoPage::AcquisitionInfoPage(component::Context *ctx, QWidget *parent)
	: QWidget(parent)
	, m_ctx(ctx)
	, m_infoPage(new InfoPage(this))
	, m_title(new QLabel("Context Attributes", this))
	, m_contextInfo(new QLabel(this))
{
	setupUi();
	setupInfoPage();
}

void AcquisitionInfoPage::setupUi()
{
	m_infoPage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	// The context attribute names are backend-specific and are not listed in
	// gui's infoPageMap.json, which InfoPage::update() filters against unless
	// advanced mode is on. Without this every row would be silently dropped.
	m_infoPage->setAdvancedMode(true);
	setLayout(new QVBoxLayout(this));
	m_title->setStyleSheet("font-weight: bold;");
	layout()->addWidget(m_contextInfo);
	layout()->addWidget(m_title);
	layout()->addWidget(m_infoPage);
}

void AcquisitionInfoPage::setupInfoPage()
{
	if(!m_ctx) {
		qWarning() << "Error, invalid context, cannot create info page.";
		m_contextInfo->setText("No device-controller context available.");
		return;
	}

	const QList<component::Device *> devices = m_ctx->findChildren<component::Device *>(Qt::FindDirectChildrenOnly);

	QString contextText = "Context created by the device controller.\n"
			      "URI: %1\n"
			      "Name: %2\n"
			      "Description: %3\n"
			      "Devices: %4";
	m_contextInfo->setText(contextText.arg(m_ctx->uri())
				       .arg(m_ctx->name())
				       .arg(m_ctx->description())
				       .arg(QString::number(devices.count())));

	// Context-level attributes are direct Attribute children of the Context.
	// Read each through its capability and show the freshly cached value - same
	// shape as IIODeviceImpl::readDeviceInfo().
	const QList<component::Attribute *> attributes =
		m_ctx->findChildren<component::Attribute *>(Qt::FindDirectChildrenOnly);
	for(component::Attribute *attr : attributes) {
		if(!attr->readCapability()) {
			continue;
		}
		QCoro::waitFor(attr->readCapability()->readAsync());
		m_infoPage->update(attr->name(), attr->cachedValue());
	}
}

#include "moc_acquisitioninfopage.cpp"
