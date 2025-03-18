#include "pkgzipwidget.h"

#include <QVBoxLayout>
#include <infoheaderwidget.h>
#include <menusectionwidget.h>
#include <pkgitemwidget.h>
#include <style.h>

using namespace scopy;

PkgZipWidget::PkgZipWidget(QWidget *parent)
	: QFrame(parent)
{
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setMargin(0);

	QWidget *container = new QWidget(this);
	container->setLayout(new QVBoxLayout(container));
	container->layout()->setContentsMargins(10, 10, 10, 0);

	MenuCollapseSection *section = new MenuCollapseSection("Include ZIP packages", MenuCollapseSection::MHCW_NONE,
							       MenuCollapseSection::MHW_BASEWIDGET, container);
	Style::setStyle(section->header(), style::properties::widget::bottomBorder);

	QWidget *hWidget = new QWidget(section);
	QHBoxLayout *hLay = new QHBoxLayout(hWidget);
	hLay->setMargin(0);

	QWidget *browserSection = createBrowserSection(hWidget);

	m_previewSection = createSection("Preview", "Details about the selected package are displayed here.", hWidget);

	QFrame *vLine = new QFrame(hWidget);
	StyleHelper::VerticalLine(vLine);

	hLay->addWidget(browserSection);
	hLay->addWidget(vLine);
	hLay->addWidget(m_previewSection);

	section->contentLayout()->addWidget(hWidget);

	container->layout()->addWidget(section);
	layout->addWidget(container);

	Style::setStyle(container, style::properties::widget::border_interactive);
	Style::setBackgroundColor(container, json::theme::background_primary);
}

PkgZipWidget::~PkgZipWidget() {}

void PkgZipWidget::onZipMetadata(QVariantMap metadata)
{
	QLineEdit *fileBrowserEdit = m_fileBrowser->lineEdit();
	PkgItemWidget *pkgItem = new PkgItemWidget(m_previewSection);
	pkgItem->fillMetadata(metadata);
	pkgItem->setZipPath(fileBrowserEdit->text());
	pkgItem->setSingleVersion(true);
	pkgItem->setPreview(true);

	m_previewSection->contentLayout()->addWidget(pkgItem);
	connect(this, &PkgZipWidget::pkgPathSelected, pkgItem, &PkgItemWidget::deleteLater);
}

void PkgZipWidget::onInstallFinished() { m_installBtn->setInstalled(true); }

MenuCollapseSection *PkgZipWidget::createSection(QString title, QString description, QWidget *parent)
{
	MenuCollapseSection *section = new MenuCollapseSection(title, MenuCollapseSection::MHCW_NONE,
							       MenuCollapseSection::MHW_INFOWIDGET, parent);
	InfoHeaderWidget *infoHeader = getInfoHeader(section);
	if(infoHeader) {
		section->header()->setCheckable(false);
		infoHeader->setDescription(description);
	}

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

QWidget *PkgZipWidget::createBrowserSection(QWidget *parent)
{
	MenuCollapseSection *browserSection = createSection(
		"Install Package", "Use the file browser to choose the desired package and then install it!", parent);

	QWidget *w = new QWidget(browserSection);
	QGridLayout *wLay = new QGridLayout(w);

	m_fileBrowser = new FileBrowserWidget(FileBrowserWidget::OPEN_FILE);
	m_fileBrowser->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	QLineEdit *fileBrowserEdit = m_fileBrowser->lineEdit();
	connect(fileBrowserEdit, &QLineEdit::textChanged, this, [this, fileBrowserEdit]() {
		const QString &path = fileBrowserEdit->text();
		bool validPath = QFile::exists(path);
		m_warningLabel->setVisible(!validPath);
		m_warningLabel->setText("Invalid package location: " + path);
		if(!m_installBtn->isEnabled()) {
			m_installBtn->setInstalled(false);
		}
		Q_EMIT pkgPathSelected(fileBrowserEdit->text());
	});

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
