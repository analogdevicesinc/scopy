#include "detailsview.h"
#include <QVBoxLayout>

#define ADD_ICON ":/gui/icons/green_add.svg"
#define REMOVE_ICON ":/gui/icons/orange_close.svg"

using namespace scopy::iiodebugplugin;

DetailsView::DetailsView(QWidget *parent)
	: QWidget(parent)
	, m_titleLabel(new QLabel("Select an IIO item.", this))
	, m_guiDetailsView(new GuiDetailsView(this))
	, m_cliDetailsView(new CliDetailsView(this))
	, m_tabWidget(new QTabWidget(this))
	, m_guiView(new QWidget(this))
	, m_iioView(new QWidget(this))
	, m_readBtn(new QPushButton("Read", this))
	, m_addToWatchlistBtn(new QPushButton(this))
	, m_titleContainer(new QWidget(this))
{
	setupUi();
}

void DetailsView::setupUi()
{
	setLayout(new QVBoxLayout(this));
	layout()->setContentsMargins(0, 6, 0, 0);

	m_titleContainer->setLayout(new QHBoxLayout(m_titleContainer));
	m_titleContainer->layout()->setContentsMargins(0, 0, 0, 0);

	m_titleLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	m_titleLabel->setStyleSheet("color: white;");
	m_titleLabel->setAlignment(Qt::AlignCenter);
	m_titleLabel->setStyleSheet("font-size: 14pt");

	m_readBtn->setMaximumWidth(90);
	m_addToWatchlistBtn->setMaximumSize(25, 25);
	m_addToWatchlistBtn->setDisabled(true);

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

	StyleHelper::TabWidgetBarUnderline(m_tabWidget, "DetailsTabWidget");
	StyleHelper::BlueButton(m_readBtn, "ReadCurrentSelectionButton");
	m_addToWatchlistBtn->setStyleSheet("QPushButton { background-color: transparent; border: 0px; }");

	m_titleContainer->layout()->addWidget(m_titleLabel);
	m_titleContainer->layout()->addWidget(m_addToWatchlistBtn);
	m_titleContainer->layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Preferred));
	m_titleContainer->layout()->addWidget(m_readBtn);

	layout()->addWidget(m_titleContainer);
	layout()->addWidget(m_tabWidget);
}

void DetailsView::setIIOStandardItem(IIOStandardItem *item)
{
	m_titleLabel->setText(item->path());
	m_guiDetailsView->setIIOStandardItem(item);
	m_cliDetailsView->setIIOStandardItem(item);
}

void DetailsView::refreshIIOView() { m_cliDetailsView->refreshView(); }

QPushButton *DetailsView::readBtn() { return m_readBtn; }

QPushButton *DetailsView::addToWatchlistBtn() { return m_addToWatchlistBtn; }

void DetailsView::setAddToWatchlistState(bool add)
{
	m_addToWatchlistBtn->setEnabled(true);
	if(add) {
		m_addToWatchlistBtn->setIcon(QIcon(ADD_ICON));
		m_addToWatchlistBtn->setToolTip("Add to Watchlist");
	} else {
		m_addToWatchlistBtn->setIcon(QIcon(REMOVE_ICON));
		m_addToWatchlistBtn->setToolTip("Remove from Watchlist");
	}
}
