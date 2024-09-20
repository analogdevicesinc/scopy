#include "deviceiconimpl.h"

#include "gui/dynamicWidget.h"
#include "stylehelper.h"

#include "ui_devicebutton.h"

#include <QDebug>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

using namespace scopy;
DeviceIconImpl::DeviceIconImpl(Device *d, QWidget *parent)
	: DeviceIcon{parent}
{
	ui = new Ui::DeviceButton;
	ui->setupUi(this);
	StyleHelper::DeviceIconBackgroundShadow(this);
	ui->description->setText(d->displayParam());
	ui->name->setText(d->displayName());
	ui->name->setStyleSheet("border: none;");
	ui->name->setReadOnly(true);
	ui->name->home(false);
	connect(ui->name, &QLineEdit::editingFinished, this, &DeviceIconImpl::onEditFinished);
	createPenBtn();

	ui->iconPlaceHolder->layout()->addWidget(d->icon());
	ui->iconPlaceHolder->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	setCheckable(true);
}

DeviceIconImpl::~DeviceIconImpl() {}

void DeviceIconImpl::setConnected(bool val)
{
	setDynamicProperty(ui->line, "connected", val);
	//	ensurePolished();
}

void DeviceIconImpl::createPenBtn()
{
	QPushButton *penBtn = new QPushButton();
	penBtn->setMaximumSize(20, 20);
	penBtn->setIcon(QIcon(":/gui/icons/edit_pen.svg"));
	connect(penBtn, &QPushButton::clicked, this, &DeviceIconImpl::onPenBtnPressed);
	HoverWidget *penHover = new HoverWidget(penBtn, ui->name, this);
	penHover->setStyleSheet("background-color: transparent; border: 0px;");
	penHover->setAnchorPos(HoverPosition::HP_RIGHT);
	penHover->setContentPos(HoverPosition::HP_RIGHT);
	penHover->setVisible(true);
	penHover->raise();
}

void DeviceIconImpl::onPenBtnPressed()
{
	ui->name->setReadOnly(false);
	ui->name->end(false);
	ui->name->setFocus();
}

void DeviceIconImpl::onEditFinished()
{
	ui->name->setReadOnly(true);
	Q_EMIT displayNameChanged(ui->name->text());
}

#include "moc_deviceiconimpl.cpp"
