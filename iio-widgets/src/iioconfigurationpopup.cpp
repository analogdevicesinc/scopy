#include "iioconfigurationpopup.h"

#include <QVBoxLayout>
#include <QLoggingCategory>
#include <gui/stylehelper.h>
#include <iioutil/connectionprovider.h>

using namespace scopy;

Q_LOGGING_CATEGORY(CAT_IIOCONFIGURATIONPOPUP, "IIOConfigurationPopup")

IIOConfigurationPopup::IIOConfigurationPopup(iio_context *ctx, QWidget *parent)
	: QWidget{parent}
	, m_tintedOverlay(nullptr)
	, m_widgetSelector(nullptr)
{
	Connection *conn = ConnectionProvider::open(ctx);
	IIOTreeScan *scan = conn->iioTreeScan();
	m_root = scan->getRoot();
	m_widgetSelector = new IIOWidgetSelector(m_root, this);
	connect(m_widgetSelector, &IIOWidgetSelector::itemSelectionChanged, this,
		&IIOConfigurationPopup::modelSelectionChanged);
	m_widgetSelector->hide();

	init();
}

IIOConfigurationPopup::IIOConfigurationPopup(iio_device *dev, QWidget *parent)
	: QWidget{parent}
	, m_tintedOverlay(nullptr)
	, m_widgetSelector(nullptr)
{
	const iio_context *ctx = iio_device_get_context(dev);
	Connection *conn = ConnectionProvider::open(const_cast<iio_context *>(ctx));
	IIOTreeScan *scan = conn->iioTreeScan();
	m_root = scan->getRoot();
	m_widgetSelector = new IIOWidgetSelector(m_root, this);
	connect(m_widgetSelector, &IIOWidgetSelector::itemSelectionChanged, this,
		&IIOConfigurationPopup::modelSelectionChanged);
	m_widgetSelector->hide();

	init();
}

IIOConfigurationPopup::IIOConfigurationPopup(iio_channel *chnl, QWidget *parent)
	: QWidget{parent}
	, m_tintedOverlay(nullptr)
	, m_widgetSelector(nullptr)
{
	const iio_device *dev = iio_channel_get_device(chnl);
	const iio_context *ctx = iio_device_get_context(dev);
	Connection *conn = ConnectionProvider::open(const_cast<iio_context *>(ctx));
	IIOTreeScan *scan = conn->iioTreeScan();
	m_root = scan->getRoot();
	m_widgetSelector = new IIOWidgetSelector(m_root, this);
	connect(m_widgetSelector, &IIOWidgetSelector::itemSelectionChanged, this,
		&IIOConfigurationPopup::modelSelectionChanged);
	m_widgetSelector->hide();

	init();
}

IIOConfigurationPopup::~IIOConfigurationPopup() { delete m_tintedOverlay; }

void IIOConfigurationPopup::enableTintedOverlay(bool enable)
{
	if(enable) {
		delete m_tintedOverlay;

		m_widgetSelector->show();
		m_tintedOverlay = new gui::TintedOverlay(parentWidget());
		m_tintedOverlay->show();
		raise();
		show();
		move(parentWidget()->rect().center() - rect().center());
	} else {
		delete m_tintedOverlay;
		m_tintedOverlay = nullptr;
	}
}

void IIOConfigurationPopup::modelSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
	Q_UNUSED(deselected)

	if(selected.indexes().isEmpty()) {
		qWarning(CAT_IIOCONFIGURATIONPOPUP) << "Selected index cannot be found.";
		return;
	}

	// There will be only one selected
	QModelIndex index = selected.indexes().first();
	QStandardItem *item = m_widgetSelector->getModel()->itemFromIndex(index);

	IIOItem *iioItem = dynamic_cast<IIOItem *>(item);
	if(!iioItem) {
		qWarning(CAT_IIOCONFIGURATIONPOPUP) << "Received item is not an IIOItem.";
		return;
	}

	switch(iioItem->type()) {
	case IIOItem::CHANNEL_ATTR:
	case IIOItem::DEVICE_ATTR:
	case IIOItem::CONTEXT_ATTR:
		m_selectButton->setEnabled(true);
		break;
	default:
		m_selectButton->setEnabled(false);
		break;
	}
}

void IIOConfigurationPopup::init()
{
	initUI();
	QObject::connect(m_selectButton, &QPushButton::clicked, this, [&] {
		const QModelIndex currentIndex = m_widgetSelector->getTree()->selectionModel()->currentIndex();
		if(!currentIndex.isValid()) {
			qWarning(CAT_IIOCONFIGURATIONPOPUP) << "Current selection index is not valid.";
			return;
		}

		QStandardItem *item = m_widgetSelector->getModel()->itemFromIndex(currentIndex);
		if(!item) {
			qWarning(CAT_IIOCONFIGURATIONPOPUP) << "Cannot get current item from index.";
			return;
		}

		IIOItem *iioItem = dynamic_cast<IIOItem *>(item);
		if(!iioItem) {
			qWarning(CAT_IIOCONFIGURATIONPOPUP) << "Cannot cast QStandardItem to IIOItem.";
			return;
		}

		Q_EMIT selectButtonClicked(iioItem);
	});
	QObject::connect(m_emptyButton, &QPushButton::clicked, this, &IIOConfigurationPopup::emptyButtonClicked);
	QObject::connect(m_exitButton, &QPushButton::clicked, this, &IIOConfigurationPopup::exitButtonClicked);
	m_selectButton->setDisabled(true);
}

void IIOConfigurationPopup::initUI()
{
	this->setObjectName("PopupWidget");
	this->setStyleSheet("");
	this->resize(500, 300);
	auto verticalLayout = new QVBoxLayout(this);
	verticalLayout->setContentsMargins(0, 0, 0, 0);
	this->setLayout(verticalLayout);

	auto backgroundWidget = new QWidget(this);
	auto backgroundLayout = new QVBoxLayout(backgroundWidget);
	verticalLayout->addWidget(backgroundWidget);

	m_titleLabel = new QLabel(backgroundWidget);
	m_titleLabel->setObjectName("titleLabel");
	m_titleLabel->setText("Select a replacement attribute.");

	auto buttonGroup = new QWidget(backgroundWidget);
	auto buttonGroupLayout = new QHBoxLayout(buttonGroup);
	buttonGroupLayout->setContentsMargins(0, 0, 0, 0);
	buttonGroupLayout->setSpacing(10);

	m_selectButton = new QPushButton("Select", buttonGroup);
	m_emptyButton = new QPushButton("No Data", buttonGroup);
	m_exitButton = new QPushButton("Exit", buttonGroup);

	buttonGroupLayout->addWidget(m_exitButton);
	buttonGroupLayout->addWidget(m_emptyButton);
	buttonGroupLayout->addWidget(m_selectButton);

	backgroundLayout->addWidget(m_titleLabel);
	backgroundLayout->addWidget(m_widgetSelector);
	backgroundLayout->addWidget(buttonGroup);

	backgroundWidget->setLayout(backgroundLayout);

	StyleHelper::TutorialChapterTitleLabel(m_titleLabel, "TitleLabel");
	StyleHelper::BlueButton(m_selectButton, "SelectButton");
	StyleHelper::BlueButton(m_emptyButton, "EmptyButton");
	StyleHelper::BlueButton(m_exitButton, "ExitButton");
	StyleHelper::OverlayMenu(this, "IIOConfigurationPopupOverlay");
}

#include "moc_iioconfigurationpopup.cpp"
