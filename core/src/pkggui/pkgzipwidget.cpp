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

#include "pkgzipwidget.h"

#include <QVBoxLayout>
#include <infoheaderwidget.h>
#include <infopage.h>
#include <infopagekeyvaluewidget.h>
#include <menusectionwidget.h>
#include <style.h>

using namespace scopy;

PkgZipWidget::PkgZipWidget(QWidget *parent)
	: QWidget(parent)
{
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setMargin(0);

	QWidget *w = new QWidget(this);
	QVBoxLayout *wLay = new QVBoxLayout(w);
	wLay->setMargin(10);

	MenuSectionCollapseWidget *browserSection = createBrowserSection(w);
	QMargins browserMargins(0, 10, 10, 10);
	browserSection->menuSection()->layout()->setContentsMargins(browserMargins);
	Style::setBackgroundColor(browserSection, json::theme::background_primary);

	m_preview = new PkgPreviewPage(w);
	m_preview->setVisible(false);
	m_preview->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	Style::setBackgroundColor(m_preview, json::theme::background_primary);

	wLay->addWidget(browserSection);
	wLay->addWidget(m_preview);
	wLay->addItem(new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

	layout->addWidget(w);

	Style::setBackgroundColor(w, json::theme::background_subtle);
}

PkgZipWidget::~PkgZipWidget() {}

void PkgZipWidget::onZipMetadata(QVariantMap metadata)
{
	m_preview->updatePreview(metadata);
	m_preview->setVisible(true);
}

void PkgZipWidget::onInstallFinished() { m_installBtn->setInstalled(true); }

void PkgZipWidget::fileBrowserEvent(const QString &text)
{
	QLineEdit *fileBrowserEdit = m_fileBrowser->lineEdit();
	const QString &path = fileBrowserEdit->text();
	bool validPath = QFile::exists(path);
	m_warningLabel->setVisible(!validPath);
	m_warningLabel->setText("Invalid package location: " + path);
	m_preview->setVisible(false);
	if(!m_installBtn->isEnabled()) {
		m_installBtn->setInstalled(false);
	}
	Q_EMIT pkgPathSelected(fileBrowserEdit->text());
}

MenuSectionCollapseWidget *PkgZipWidget::createSection(QString title, QString description, QWidget *parent)
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

InfoHeaderWidget *PkgZipWidget::getInfoHeader(MenuCollapseSection *section)
{
	MenuCollapseHeader *header = dynamic_cast<MenuCollapseHeader *>(section->header());
	if(!header) {
		return nullptr;
	}
	return dynamic_cast<InfoHeaderWidget *>(header->headerWidget());
}

MenuSectionCollapseWidget *PkgZipWidget::createBrowserSection(QWidget *parent)
{
	MenuSectionCollapseWidget *browserSection = createSection(
		"Install Package", "Use the file browser to choose the desired package and then install it!", parent);

	QWidget *w = new QWidget(browserSection);
	QGridLayout *wLay = new QGridLayout(w);
	wLay->setMargin(0);

	m_fileBrowser = new FileBrowserWidget(FileBrowserWidget::OPEN_FILE);
	m_fileBrowser->setFilter("*.zip");
	m_fileBrowser->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

	QLineEdit *fileBrowserEdit = m_fileBrowser->lineEdit();
	connect(fileBrowserEdit, &QLineEdit::textChanged, this, &PkgZipWidget::fileBrowserEvent);

	m_installBtn = new InstallBtn(w);
	connect(m_installBtn, &QPushButton::clicked, this,
		[this, fileBrowserEdit]() { Q_EMIT installClicked(fileBrowserEdit->text()); });

	m_warningLabel = new QLabel(w);
	Style::setStyle(m_warningLabel, style::properties::label::warning);
	m_warningLabel->setVisible(false);
	m_warningLabel->setWordWrap(true);

	wLay->addWidget(m_fileBrowser, 0, 0);
	wLay->addWidget(m_installBtn, 0, 1);
	wLay->addWidget(m_warningLabel, 1, 0);

	browserSection->contentLayout()->addWidget(w);
	browserSection->contentLayout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	return browserSection;
}

#include "moc_pkgzipwidget.cpp"
