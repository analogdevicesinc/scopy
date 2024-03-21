#include "detailsview.h"
#include <QVBoxLayout>

using namespace scopy::iiodebugplugin;

DetailsView::DetailsView(QWidget *parent)
	: QWidget(parent)
	, m_titleLabel(new QLabel("Select an IIO item.", this))
	, m_guiDetailsView(new GuiDetailsView(this))
	, m_cliDetailsView(new CliDetailsView(this))
	, m_tabWidget(new QTabWidget(this))
	, m_guiView(new QWidget(this))
	, m_iioView(new QWidget(this))
{
	setupUi();
}

void DetailsView::setupUi()
{
	setLayout(new QVBoxLayout(this));
	layout()->setContentsMargins(0, 6, 0, 0);

	m_titleLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	m_titleLabel->setStyleSheet("color: white;");
	m_titleLabel->setAlignment(Qt::AlignCenter);
	m_titleLabel->setStyleSheet("font-size: 14pt");

	m_guiView->setLayout(new QVBoxLayout(m_guiView));
	m_iioView->setLayout(new QVBoxLayout(m_iioView));

	m_guiView->layout()->setContentsMargins(0, 0, 0, 0);
	m_iioView->layout()->setContentsMargins(0, 0, 0, 0);

	m_guiView->layout()->addWidget(m_guiDetailsView);
	m_iioView->layout()->addWidget(m_cliDetailsView);

	m_tabWidget->addTab(m_guiView, "GUI View");
	m_tabWidget->addTab(m_iioView, "IIO View");
	m_tabWidget->tabBar()->setDocumentMode(true);
	m_tabWidget->tabBar()->setExpanding(true);
	// TODO: this will move to StyleHelper
	QString style = "QTabBar::tab:selected { border-bottom-color: &&ScopyBlue&&; }";
	style.replace("&&ScopyBlue&&", StyleHelper::getColor("ScopyBlue"));
	m_tabWidget->setStyleSheet(style);

	layout()->addWidget(m_titleLabel);
	layout()->addWidget(m_tabWidget);
}

void DetailsView::setIIOStandardItem(IIOStandardItem *item)
{
	m_titleLabel->setText(item->path());
	m_guiDetailsView->setIIOStandardItem(item);
	m_cliDetailsView->setIIOStandardItem(item);
}

void DetailsView::refreshIIOView() { m_cliDetailsView->refreshView(); }
