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

#include "pkgcard.h"
#include "style.h"
#include <QJsonObject>
#include <QVBoxLayout>
#include <installpkgdialog.h>
#include <qapplication.h>
#include <stylehelper.h>
#include <common/scopyconfig.h>
#include "pkgmanifestfields.h"

using namespace scopy;

QVector<QString> PkgCard::m_categories{};

PkgCard::PkgCard(QWidget *parent)
	: CardWidget(parent)
{
	QWidget *btnsW = new QWidget(this);
	QHBoxLayout *btnsLay = new QHBoxLayout(btnsW);
	btnsLay->setMargin(0);

	QWidget *categoryW = new QWidget(btnsW);
	m_categoryLay = new QHBoxLayout(categoryW);
	m_categoryLay->setMargin(0);

	m_installBtn = new InstallBtn(btnsW);
	connect(m_installBtn, &QPushButton::clicked, this, &PkgCard::installClicked);

	m_uninstallBtn = new UninstallBtn(btnsW);
	m_uninstallBtn->setVisible(false);
	connect(m_uninstallBtn, &QPushButton::clicked, this, &PkgCard::uninstallClicked);

	btnsLay->addWidget(categoryW);
	btnsLay->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
	btnsLay->addWidget(m_installBtn);
	btnsLay->addWidget(m_uninstallBtn);

	m_layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));
	m_layout->addWidget(btnsW);
}

PkgCard::~PkgCard() {}

void PkgCard::setVersions(QStringList versions)
{
	// remove the item from cb
	versCb()->setEnabled(true);
	Style::setStyle(versCb(), style::properties::combobox::versionCb, false);
	for(const QString &v : versions) {
		versCb()->addItem(v);
	}
}

void PkgCard::fillMetadata(QVariantMap metadata, bool installed)
{
	m_metadata = metadata;
	QStringList categories = metadata[PkgManifest::PKG_CATEGORY].toStringList();
	QString pkgTitle = metadata.contains(PkgManifest::PKG_TITLE) ? metadata[PkgManifest::PKG_TITLE].toString()
								     : metadata[PkgManifest::PKG_ID].toString();
	bool basePkg = categories.contains(PkgManifest::PKG_BASE);
	m_id = m_metadata[PkgManifest::PKG_ID].toString();
	title()->setText(pkgTitle);
	subtitle()->setText("by " + metadata[PkgManifest::PKG_AUTHOR].toString());
	versCb()->insertItem(0, "version " + metadata[PkgManifest::PKG_VERSION].toString());
	description()->setText(metadata[PkgManifest::PKG_DESCRIPTION].toString());
	m_installBtn->setVisible(!installed);
	m_uninstallBtn->setVisible(installed && !basePkg);
	fillCategories(categories);
}

void PkgCard::mousePressEvent(QMouseEvent *event)
{
	if(event->button() == Qt::LeftButton) {
		QVariant prop = this->property(style::properties::widget::cardWidget);
		if(prop.toString() == "selected") {
			Q_EMIT hidePreview();
		} else if(prop.toBool()) {
			Q_EMIT preview(m_metadata);
			Style::setStyle(this, style::properties::widget::cardWidget, "selected");
		}
	}
}

QVariantMap PkgCard::metadata() const { return m_metadata; }

void PkgCard::installFinished(bool installed)
{
	m_installBtn->setDisabled(installed);
	if(installed) {
		m_installBtn->setText("Installled");
	}
}

void PkgCard::uninstallFinished(bool uninstalled) { m_uninstallBtn->setUninstalled(uninstalled); }

void PkgCard::fillCategories(QStringList categories)
{
	for(const QString &cat : categories) {
		InteractiveLabel *catLbl = new InteractiveLabel(cat, this);
		if(!m_categories.contains(cat)) {
			m_categories.push_back(cat);
		}
		int index = m_categories.indexOf(cat);
		catLbl->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		m_categoryLay->addWidget(catLbl);
		StyleHelper::ColoredInteractiveLabel(catLbl, index);
		connect(catLbl, &InteractiveLabel::clickEvent, this, &PkgCard::categorySelected);
	}
}

#include "moc_pkgcard.cpp"
