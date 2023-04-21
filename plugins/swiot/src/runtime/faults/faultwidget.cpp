#include "faultwidget.h"

#include <utility>
#include "ui_faultwidget.h"
#include "core/logging_categories.h"

using namespace scopy::swiot;

FaultWidget::FaultWidget(unsigned int id, QString name, QString faultExplanation, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::FaultWidget),
	stored(false),
	active(false),
	pressed(false),
	id(id),
	name(std::move(name)),
	faultExplanation(std::move(faultExplanation)) {
	ui->setupUi(this);

	installEventFilter(this);

	this->ui->title->setText("Bit" + QString::number(this->id));

	this->ui->mainFrame->setMinimumSize(70, 90);
}

FaultWidget::~FaultWidget() {
	delete ui;
}

bool FaultWidget::isStored() const {
	return stored;
}

void FaultWidget::setStored(bool stored_) {
	FaultWidget::stored = stored_;
	this->ui->stored->setProperty("high", this->stored);
	this->ui->stored->setStyle(this->ui->stored->style());
}

bool FaultWidget::isActive() const {
	return active;
}

void FaultWidget::setActive(bool active_) {
	FaultWidget::active = active_;
	this->ui->active->setProperty("high", this->active);
	this->ui->active->setStyle(this->ui->active->style());
}

const QString &FaultWidget::getName() const {
	return name;
}

void FaultWidget::setName(const QString &name_) {
	FaultWidget::name = name_;
}

const QString &FaultWidget::getFaultExplanation() const {
	return faultExplanation;
}

void FaultWidget::setFaultExplanation(const QString &faultExplanation_) {
	FaultWidget::faultExplanation = faultExplanation_;
}

unsigned int FaultWidget::getId() const {
	return id;
}

void FaultWidget::setId(unsigned int id_) {
	FaultWidget::id = id_;
}

bool FaultWidget::isPressed() const {
	return pressed;
}

void FaultWidget::setPressed(bool pressed_) {
	FaultWidget::pressed = pressed_;
	scopy::setDynamicProperty(this->ui->mainFrame, "pressed", pressed_);
}

bool FaultWidget::eventFilter(QObject *object, QEvent *event) {
	if (event->type() == QEvent::MouseButtonPress) {
		this->setPressed( !scopy::getDynamicProperty(this->ui->mainFrame, "pressed") );

		Q_EMIT faultSelected(this->id);
	}

	if (event->type() == QEvent::ToolTip) {
		this->setToolTip(this->name);
	}

	return QWidget::eventFilter(object, event);
}
