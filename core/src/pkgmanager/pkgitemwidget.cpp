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

#include "pkgitemwidget.h"
#include "style.h"
#include <QJsonObject>
#include <QVBoxLayout>
#include <installpkgdialog.h>
#include <qapplication.h>
#include <stylehelper.h>
#include <common/scopyconfig.h>
#include "pkgmanifestfields.h"

using namespace scopy;

QVector<QString> PkgItemWidget::m_categories{};

PkgItemWidget::PkgItemWidget(QWidget *parent)
	: QFrame(parent)
{
	QVBoxLayout *lay = new QVBoxLayout(this);
	lay->setSpacing(10);
	QWidget *titleW = createTitleW(this);

	m_description = new QLabel(this);
	m_description->setWordWrap(true);

	QWidget *btnsW = new QWidget(this);
	QHBoxLayout *btnsLay = new QHBoxLayout(btnsW);
	btnsLay->setMargin(0);

	QWidget *categoryW = new QWidget(btnsW);
	m_categoryLay = new QHBoxLayout(categoryW);
	m_categoryLay->setMargin(0);

	m_installBtn = new InstallBtn(btnsW);
	connect(m_installBtn, &QPushButton::clicked, this, &PkgItemWidget::installClicked);

	m_uninstallBtn = new UninstallBtn(btnsW);
	m_uninstallBtn->setVisible(false);
	connect(m_uninstallBtn, &QPushButton::clicked, this, &PkgItemWidget::uninstallClicked);

	btnsLay->addWidget(categoryW);
	btnsLay->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
	btnsLay->addWidget(m_installBtn);
	btnsLay->addWidget(m_uninstallBtn);

	lay->addWidget(titleW);
	lay->addWidget(m_description);
	lay->addItem(new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));
	lay->addWidget(btnsW);

	Style::setStyle(this, style::properties::widget::pkgItem);
}

PkgItemWidget::~PkgItemWidget() {}

QString PkgItemWidget::id() const { return m_metadata[PkgManifest::PKG_ID].toString(); }

void PkgItemWidget::setVersions(QStringList versions)
{
	// remove the item from cb
	m_versCb->setEnabled(true);
	Style::setStyle(m_versCb, style::properties::combobox::versionCb, false);
	for(const QString &v : versions) {
		m_versCb->addItem(v);
	}
}

void PkgItemWidget::fillMetadata(QVariantMap metadata, bool installed)
{
	m_metadata = metadata;
	QStringList categories = metadata[PkgManifest::PKG_CATEGORY].toStringList();
	QString pkgTitle = metadata.contains(PkgManifest::PKG_TITLE) ? metadata[PkgManifest::PKG_TITLE].toString()
								     : metadata[PkgManifest::PKG_ID].toString();
	bool basePkg = categories.contains(PkgManifest::PKG_BASE);
	m_title->setText(pkgTitle);
	m_subtitle->setText("by " + metadata[PkgManifest::PKG_AUTHOR].toString());
	m_versCb->insertItem(0, "version " + metadata[PkgManifest::PKG_VERSION].toString());
	m_description->setText(metadata[PkgManifest::PKG_DESCRIPTION].toString());
	m_installBtn->setVisible(!installed);
	m_uninstallBtn->setVisible(installed && !basePkg);
	fillCategories(categories);
}

void PkgItemWidget::mousePressEvent(QMouseEvent *event)
{
	if(event->button() == Qt::LeftButton) {
		QVariant prop = this->property(style::properties::widget::pkgItem);
		if(prop.toString() == "selected") {
			Q_EMIT hidePreview();
		} else if(prop.toBool()) {
			Q_EMIT preview(m_metadata);
			Style::setStyle(this, style::properties::widget::pkgItem, "selected");
		}
	}
}

QWidget *PkgItemWidget::createTitleW(QWidget *parent)
{
	QWidget *w = new QWidget(this);
	w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	QGridLayout *lay = new QGridLayout(w);
	lay->setMargin(0);
	lay->setSpacing(0);

	m_title = new QLineEdit(w);
	m_title->setReadOnly(true);
	m_title->setEnabled(false);
	m_title->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	Style::setStyle(m_title, style::properties::lineedit::headerLineEdit);

	m_subtitle = new QLabel(w);
	Style::setStyle(m_subtitle, style::properties::label::subtle);

	m_versCb = new QComboBox(w);
	m_versCb->setDisabled(true);
	m_versCb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
	Style::setStyle(m_versCb, style::properties::combobox::versionCb);

	lay->addWidget(m_title, 0, 0, 1, Qt::AlignLeft);
	lay->addWidget(m_versCb, 0, 1, 0, Qt::AlignRight);
	lay->addWidget(m_subtitle, 1, 0, Qt::AlignLeft);

	return w;
}

QVariantMap PkgItemWidget::metadata() const { return m_metadata; }

void PkgItemWidget::installFinished(bool installed)
{
	m_installBtn->setDisabled(installed);
	if(installed) {
		m_installBtn->setText("Installled");
	}
}

void PkgItemWidget::uninstallFinished(bool uninstalled) { m_uninstallBtn->setUninstalled(uninstalled); }

void PkgItemWidget::fillCategories(QStringList categories)
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
		connect(catLbl, &InteractiveLabel::clickEvent, this, &PkgItemWidget::categorySelected);
	}
}

#include "moc_pkgitemwidget.cpp"
