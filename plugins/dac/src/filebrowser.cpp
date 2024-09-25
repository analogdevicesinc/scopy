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
	fileBufferContainer->contentLayout()->setSpacing(10);

	m_fileBufferPath = new ProgressLineEdit(this);
	m_fileBufferPath->getLineEdit()->setReadOnly(true);

	m_fileBufferBrowseBtn = new QPushButton("Browse", fileBufferContainer);
	m_fileBufferLoadBtn = new QPushButton("Load", fileBufferContainer);
	connect(m_fileBufferBrowseBtn, &QPushButton::clicked, this, &FileBrowser::chooseFile);
	connect(m_fileBufferLoadBtn, &QPushButton::clicked, this, &FileBrowser::loadFile);
	fileBufferContainer->contentLayout()->addWidget(m_fileBufferPath);
	fileBufferContainer->contentLayout()->addWidget(m_fileBufferBrowseBtn);
	fileBufferContainer->contentLayout()->addWidget(m_fileBufferLoadBtn);
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
