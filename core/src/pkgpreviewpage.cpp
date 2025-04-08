/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#include "pkgpreviewpage.h"

using namespace scopy;

PkgPreviewPage::PkgPreviewPage(QWidget *parent)
	: QFrame(parent)
{
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setMargin(0);

	MenuSectionCollapseWidget *section =
		createSection("Preview", "Details about the selected package are displayed here.", this);
	QMargins sectionMargins(0, 10, 10, 10);
	section->menuSection()->layout()->setContentsMargins(sectionMargins);
	section->menuSection()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	m_infoPage = new InfoPage(this);
	m_infoPage->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	m_infoPage->layout()->setMargin(0);

	QScrollArea *scrollArea = new QScrollArea(section);
	scrollArea->setWidget(m_infoPage);
	scrollArea->setWidgetResizable(true);

	section->add(scrollArea);

	layout->addWidget(section);
}

PkgPreviewPage::~PkgPreviewPage() {}

void PkgPreviewPage::updatePreview(const QVariantMap &metadata)
{
	m_infoPage->clear();
	for(auto it = metadata.begin(); it != metadata.end(); ++it) {
		QVariant variant = it.value();
		QString value = it.value().toString();
		if(variant.type() == QVariant::List) {
			value = variant.toStringList().join(", ");
		} else {
			value = variant.toString();
		}
		if(value.isEmpty()) {
			value = "Empty field";
		}
		m_infoPage->update(it.key(), value);
	}
}

MenuSectionCollapseWidget *PkgPreviewPage::createSection(QString title, QString description, QWidget *parent)
{
	MenuSectionCollapseWidget *section = new MenuSectionCollapseWidget(title, MenuCollapseSection::MHCW_NONE,
									   MenuCollapseSection::MHW_INFOWIDGET, parent);
	QMargins leftMargin(10, 0, 0, 0);
	InfoHeaderWidget *infoHeader = getInfoHeader(section->collapseSection());
	if(infoHeader) {
		section->collapseSection()->header()->setCheckable(false);
		infoHeader->setDescription(description);
		leftMargin.setLeft(infoHeader->infoBtn()->sizeHint().width());
	}
	section->contentLayout()->setContentsMargins(leftMargin);

	return section;
}

InfoHeaderWidget *PkgPreviewPage::getInfoHeader(MenuCollapseSection *section)
{
	MenuCollapseHeader *header = dynamic_cast<MenuCollapseHeader *>(section->header());
	if(!header) {
		return nullptr;
	}
	return dynamic_cast<InfoHeaderWidget *>(header->headerWidget());
}

#include "moc_pkgpreviewpage.cpp"
