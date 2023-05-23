#include "faultsdevice.h"
#include <utility>
#include "faultsgroup.h"

#include "src/swiot_logging_categories.h"

using namespace scopy::swiot;

FaultsDevice::FaultsDevice(QString name, QString path, QWidget *parent)
	: ui(new Ui::FaultsDevice),
	  QWidget(parent),
	  m_faults_explanation(new QWidget(this)),
	  m_subsectionSeparator(new scopy::gui::SubsectionSeparator("Faults Explanation", true, this)),
	  m_faultsGroup(new FaultsGroup(name, path, this)),
	  m_name(std::move(name)),
	  m_path(std::move(path)) {
	ui->setupUi(this);
	this->connectSignalsAndSlots();
	this->initFaultExplanations();

	setDynamicProperty(m_subsectionSeparator->getButton(), "subsection_arrow_button", true);
	m_subsectionSeparator->setContent(m_faults_explanation);

	this->ui->label_name->setText(m_name);
	this->ui->faults_layout->addWidget(this->m_faultsGroup);
	this->ui->faults_explanation->layout()->addWidget(m_subsectionSeparator);
	m_faults_explanation->ensurePolished();
}

FaultsDevice::~FaultsDevice() {
	delete ui;
}

void FaultsDevice::resetStored() {
	for (auto fault: this->m_faultsGroup->getFaults()) {
		fault->setStored(false);
	}
	this->updateExplanations();
}

void FaultsDevice::update(uint32_t faults_numeric) {
	this->ui->lineEdit_numeric->setText(QString("0x%1").arg(faults_numeric, 8, 16, QLatin1Char('0')));
	this->m_faultsGroup->update(faults_numeric);
	this->updateExplanations();
}

void FaultsDevice::updateExplanations() {
	std::set<unsigned int> selected = m_faultsGroup->getSelectedIndexes();
	std::set<unsigned int> actives = m_faultsGroup->getActiveIndexes();
	if (selected.empty()) {
		for (int i = 0; i < m_faultExplanationWidgets.size(); ++i) {
			m_faultExplanationWidgets[i]->show();

			if (actives.contains(i)) {
				setDynamicProperty(m_faultExplanationWidgets[i], "highlighted", true);
			} else {
				setDynamicProperty(m_faultExplanationWidgets[i], "highlighted", false);
			}
		}
	} else {
		for (int i = 0; i < m_faultExplanationWidgets.size(); ++i) {
			if (selected.contains(i)) {
				m_faultExplanationWidgets[i]->show();
			} else {
				m_faultExplanationWidgets[i]->hide();
			}

			if (actives.contains(i)) {
				setDynamicProperty(m_faultExplanationWidgets[i], "highlighted", true);
			} else {
				setDynamicProperty(m_faultExplanationWidgets[i], "highlighted", false);
			}
		}
	}

	m_faults_explanation->ensurePolished();
}

void FaultsDevice::updateMinimumHeight() {
	this->ensurePolished();
	this->m_faults_explanation->ensurePolished();
	qWarning() << "FaultsDevice::updateMinimumHeight()";
	this->m_faultsGroup->ensurePolished();
}

void FaultsDevice::resizeEvent(QResizeEvent *event) {
	qDebug(CAT_SWIOT_FAULTS) << "resize faults explanation:" << m_faults_explanation->height(); // TODO: delete this if not used

	QWidget::resizeEvent(event);
}

void FaultsDevice::initFaultExplanations() {
	m_faults_explanation->setLayout(new QVBoxLayout(m_faults_explanation));
	m_faults_explanation->layout()->setContentsMargins(0, 0, 0, 0);
	m_faults_explanation->layout()->setSpacing(0);
	m_faults_explanation->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
	m_faults_explanation->layout()->setSizeConstraint(QLayout::SetMinimumSize);

	QStringList l = this->m_faultsGroup->getExplanations();
	for (const auto &item: l) {
		auto widget = new QLabel(item, m_faults_explanation);
		widget->setTextFormat(Qt::PlainText);
		widget->setStyleSheet("QWidget[highlighted=true]{color:white;} QWidget{color:#5c5c5c;}");
//		widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
		widget->setWordWrap(true);
		m_faultExplanationWidgets.push_back(widget);
		m_faults_explanation->layout()->addWidget(widget);
	}
	m_faults_explanation->ensurePolished();
}

void FaultsDevice::connectSignalsAndSlots() {
	connect(this->ui->clear_selection_button, &QPushButton::clicked, this->m_faultsGroup,
		&FaultsGroup::clearSelection);
	connect(this->ui->reset_button, &QPushButton::clicked, this, &FaultsDevice::resetStored);
	connect(m_faultsGroup, &FaultsGroup::selectionUpdated, this, &FaultsDevice::updateExplanations);
	connect(m_faultsGroup, &FaultsGroup::minimumSizeChanged, this, &FaultsDevice::updateMinimumHeight);
}
