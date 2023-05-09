#include "faultwidget.h"

#include <utility>
#include "ui_faultwidget.h"
#include "core/logging_categories.h"

using namespace scopy::swiot;

FaultWidget::FaultWidget(unsigned int id, QString name, QString faultExplanation, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::FaultWidget),
	m_stored(false),
	m_active(false),
	m_pressed(false),
	m_id(id),
	m_name(std::move(name)),
	m_faultExplanation(std::move(faultExplanation)) {
	ui->setupUi(this);

	installEventFilter(this);

	this->ui->title->setText("Bit" + QString::number(this->m_id));

	this->ui->mainFrame->setMinimumSize(70, 90);
}

FaultWidget::~FaultWidget() {
	delete ui;
}

bool FaultWidget::isStored() const {
	return m_stored;
}

void FaultWidget::setStored(bool stored) {
	FaultWidget::m_stored = stored;
	this->ui->stored->setProperty("high", this->m_stored);
	this->ui->stored->setStyle(this->ui->stored->style());
}

bool FaultWidget::isActive() const {
	return m_active;
}

void FaultWidget::setActive(bool active) {
	FaultWidget::m_active = active;
	this->ui->active->setProperty("high", m_active);
	this->ui->active->setStyle(this->ui->active->style());
}

const QString &FaultWidget::getName() const {
	return m_name;
}

void FaultWidget::setName(const QString &name) {
	FaultWidget::m_name = name;
}

const QString &FaultWidget::getFaultExplanation() const {
	return m_faultExplanation;
}

void FaultWidget::setFaultExplanation(const QString &faultExplanation) {
	FaultWidget::m_faultExplanation = faultExplanation;
}

unsigned int FaultWidget::getId() const {
	return m_id;
}

void FaultWidget::setId(unsigned int id) {
	FaultWidget::m_id = id;
}

bool FaultWidget::isPressed() const {
	return m_pressed;
}

void FaultWidget::setPressed(bool pressed) {
	FaultWidget::m_pressed = pressed;
	scopy::setDynamicProperty(this->ui->mainFrame, "pressed", pressed);
}

bool FaultWidget::eventFilter(QObject *object, QEvent *event) {
	if (event->type() == QEvent::MouseButtonPress) {
		this->setPressed( !scopy::getDynamicProperty(this->ui->mainFrame, "pressed") );

		Q_EMIT faultSelected(this->m_id);
	}

	if (event->type() == QEvent::ToolTip) {
		this->setToolTip(this->m_name);
	}

	return QWidget::eventFilter(object, event);
}
