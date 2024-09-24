#include "filebrowser.h"
#include "dac_logging_categories.h"

#include <pluginbase/preferences.h>
#include <menusectionwidget.h>
#include <menucollapsesection.h>
#include <stylehelper.h>

#include <QFileDialog>

using namespace scopy;
using namespace scopy::dac;
FileBrowser::FileBrowser(QWidget *parent)
	: QWidget(parent)
	, m_filename("")
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	auto m_layout = new QVBoxLayout();
	m_layout->setMargin(0);
	m_layout->setSpacing(10);
	setLayout(m_layout);

	MenuSectionWidget *fileBufferContainer = new MenuSectionWidget(this);
	MenuCollapseSection *fileBufferSection =
		new MenuCollapseSection("DAC Buffer Settings", MenuCollapseSection::MHCW_NONE, fileBufferContainer);
	fileBufferSection->contentLayout()->setSpacing(10);

	m_fileBufferPath = new ProgressLineEdit(this);
	m_fileBufferPath->getLineEdit()->setReadOnly(true);

	m_fileBufferBrowseBtn = new QPushButton("Browse", fileBufferSection);
	m_fileBufferLoadBtn = new QPushButton("Load", fileBufferSection);
	connect(m_fileBufferBrowseBtn, &QPushButton::clicked, this, &FileBrowser::chooseFile);
	connect(m_fileBufferLoadBtn, &QPushButton::clicked, this, &FileBrowser::loadFile);
	fileBufferSection->contentLayout()->addWidget(new QLabel("Choose file"));
	fileBufferSection->contentLayout()->addWidget(m_fileBufferPath);
	fileBufferSection->contentLayout()->addWidget(m_fileBufferBrowseBtn);
	fileBufferSection->contentLayout()->addWidget(m_fileBufferLoadBtn);
	fileBufferContainer->contentLayout()->addWidget(fileBufferSection);
	StyleHelper::BlueButton(m_fileBufferBrowseBtn);
	StyleHelper::BlueButton(m_fileBufferLoadBtn);

	m_layout->addWidget(fileBufferContainer);
}

FileBrowser::~FileBrowser() {}

QString FileBrowser::getFilePath() const { return m_filename; }

void FileBrowser::loadFile() { Q_EMIT load(m_filename); }

void FileBrowser::chooseFile()
{
	QString selectedFilter;

	bool useNativeDialogs = Preferences::get("general_use_native_dialogs").toBool();
	QString tmpFilename = QFileDialog::getOpenFileName(this, tr("Import"), "", tr("All Files(*)"), &selectedFilter,
							   (useNativeDialogs ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog));
	if(!tmpFilename.isEmpty()) {
		m_filename = tmpFilename;
		m_fileBufferPath->getLineEdit()->setText(m_filename);
	}
}
