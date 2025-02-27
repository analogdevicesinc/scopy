#include "pkgitemwidget.h"
#include "style.h"
#include <QJsonObject>
#include <QVBoxLayout>
#include <installpkgdialog.h>
#include <pkginstaller.h>
#include <pkgutil.h>
#include <common/scopyconfig.h>

using namespace scopy;

PkgItemWidget::PkgItemWidget(QWidget *parent)
	: QWidget(parent)
{
	QVBoxLayout *lay = new QVBoxLayout(this);
	lay->setMargin(0);

	QWidget *titleW = new QWidget(this);
	QHBoxLayout *titleLay = new QHBoxLayout(titleW);
	titleLay->setMargin(0);
	m_title = new QLabel(titleW);
	Style::setStyle(m_title, style::properties::label::menuSmall);
	m_versCb = new QComboBox(titleW);
	m_versCb->setFixedWidth(60);
	m_versCb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

	titleLay->addWidget(m_title, Qt::AlignLeft);
	titleLay->addWidget(m_versCb, Qt::AlignRight);

	lay->addWidget(titleW);

	m_description = new QLabel(this);
	m_description->setWordWrap(true);

	m_installBtn = new QPushButton("Install", this);
	Style::setStyle(m_installBtn, style::properties::button::basicButton);
	m_installBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	connect(m_installBtn, &QPushButton::pressed, this, &PkgItemWidget::installClicked);

	m_uninstallBtn = new QPushButton("Uninstall", this);
	Style::setStyle(m_uninstallBtn, style::properties::button::basicButton);
	m_uninstallBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	m_uninstallBtn->setVisible(false);
	connect(m_uninstallBtn, &QPushButton::pressed, this, &PkgItemWidget::uninstallClicked);

	lay->addWidget(titleW);
	lay->addWidget(m_description);
	lay->addWidget(m_installBtn, Qt::AlignLeft);
	lay->addWidget(m_uninstallBtn, Qt::AlignLeft);
	lay->addItem(new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));
}

PkgItemWidget::~PkgItemWidget() {}

QString PkgItemWidget::name() const { return m_metadata.value("name").toString(); }

QString PkgItemWidget::zipPath() const { return m_zipPath; }

void PkgItemWidget::setZipPath(const QString &newZipPath) { m_zipPath = newZipPath; }

void PkgItemWidget::setUninstalled()
{
	m_uninstallBtn->setDisabled(true);
	m_uninstallBtn->setText("Uninstalled");
}

void PkgItemWidget::setInstalled()
{
	m_installBtn->setDisabled(true);
	m_installBtn->setText("Installled");
}

void PkgItemWidget::fillMetadata(QVariantMap metadata, bool installed)
{
	m_metadata = metadata;
	m_title->setText(metadata.value("name").toString() + " by " + metadata.value("author").toString());
	m_versCb->insertItem(0, metadata.value("version").toString());
	m_description->setText(metadata.value("description").toString());
	m_installBtn->setVisible(!installed);
	m_uninstallBtn->setVisible(installed);
}
