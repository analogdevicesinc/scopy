#include "pkgitemwidget.h"
#include "style.h"
#include <QJsonObject>
#include <QVBoxLayout>
#include <installpkgdialog.h>
#include <stylehelper.h>
#include <common/scopyconfig.h>

using namespace scopy;

PkgItemWidget::PkgItemWidget(QWidget *parent)
	: QFrame(parent)
{
	QVBoxLayout *lay = new QVBoxLayout(this);
	QWidget *titleW = createTitleW(this);

	m_description = new QLabel(this);
	m_description->setWordWrap(true);

	m_btnsW = new QWidget(this);
	QHBoxLayout *btnsLay = new QHBoxLayout(m_btnsW);
	btnsLay->setMargin(0);

	m_installBtn = new InstallBtn(this);
	connect(m_installBtn, &QPushButton::clicked, this, &PkgItemWidget::installClicked);

	m_uninstallBtn = new UninstallBtn(this);
	m_uninstallBtn->setVisible(false);
	connect(m_uninstallBtn, &QPushButton::clicked, this, &PkgItemWidget::uninstallClicked);

	btnsLay->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
	btnsLay->addWidget(m_installBtn);
	btnsLay->addWidget(m_uninstallBtn);

	lay->addWidget(titleW);
	lay->addWidget(m_description);
	lay->addItem(new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));
	lay->addWidget(m_btnsW);
}

PkgItemWidget::~PkgItemWidget() {}

QString PkgItemWidget::name() const { return m_metadata.value("name").toString(); }

QString PkgItemWidget::zipPath() const { return m_zipPath; }

void PkgItemWidget::setZipPath(const QString &newZipPath) { m_zipPath = newZipPath; }

void PkgItemWidget::setSingleVersion(bool en) { m_versCb->setDisabled(en); }

void PkgItemWidget::fillMetadata(QVariantMap metadata, bool installed)
{
	m_metadata = metadata;
	m_title->setText(metadata.value("name").toString() + " by " + metadata.value("author").toString());
	m_versCb->insertItem(0, metadata.value("version").toString());
	m_description->setText(metadata.value("description").toString());
	m_installBtn->setVisible(!installed);
	m_uninstallBtn->setVisible(installed);
}

QWidget *PkgItemWidget::createTitleW(QWidget *parent)
{
	QWidget *w = new QWidget(this);
	w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	QHBoxLayout *lay = new QHBoxLayout(w);
	lay->setMargin(0);

	m_title = new QLabel(w);
	Style::setStyle(m_title, style::properties::label::menuSmall);

	m_versCb = new QComboBox(w);
	m_versCb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
	Style::setStyle(m_versCb, style::properties::checkbox::squareCB);

	lay->addWidget(m_title, Qt::AlignLeft);
	lay->addWidget(m_versCb, Qt::AlignRight);

	return w;
}

void PkgItemWidget::installFinished(bool installed)
{
	m_installBtn->setDisabled(installed);
	if(installed) {
		m_installBtn->setText("Installled");
	}
}

void PkgItemWidget::uninstallFinished(bool uninstalled) { m_uninstallBtn->setUninstalled(uninstalled); }

void PkgItemWidget::setPreview(bool en) { m_btnsW->setVisible(!en); }
