#include "swiotfaultsdevice.hpp"
#include <core/logging_categories.h>
#include <utility>

namespace adiscope::gui {
FaultsDevice::FaultsDevice(QString name, QString path, QWidget *parent)
	: ui(new Ui::FaultsDevice),
	  QWidget(parent),
	  m_faults_explanation(new QTextEdit(this)),
	  m_subsectionSeparator(new SubsectionSeparator("Faults Explanation", true, this)),
	  m_faultsGroup(new FaultsGroup(name, path, this)),
	  m_name(std::move(name)),
	  m_path(std::move(path)) {

	ui->setupUi(this);

	m_faults_explanation->setReadOnly(true);
        m_faults_explanation->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_faults_explanation->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_faults_explanation->setFixedHeight(m_faults_explanation->document()->size().toSize().height() + 3);
        m_faults_explanation->setHtml(this->m_faultsGroup->getExplanations());
        m_subsectionSeparator->setContent(m_faults_explanation);

        this->ui->label_name->setText(m_name);
        this->ui->faults_layout->addWidget(this->m_faultsGroup);
        this->ui->faults_explanation->layout()->addWidget(m_subsectionSeparator);

	connect(this->ui->clear_selection_button, &QPushButton::clicked, this->m_faultsGroup, &FaultsGroup::clearSelection);
	connect(this->ui->reset_button, &QPushButton::clicked, this, &adiscope::gui::FaultsDevice::resetStored);
	connect(m_faultsGroup, &FaultsGroup::selectionUpdated, this, &FaultsDevice::updateExplanations);
	connect(m_faultsGroup, &FaultsGroup::minimumSizeChanged, this, &FaultsDevice::updateMinimumHeight);
	connect(m_faults_explanation, &QTextEdit::textChanged, this, [=](){ // TODO: make this a separate slot
		m_faults_explanation->setFixedHeight(m_faults_explanation->document()->size().toSize().height() + 3);
	});
}

FaultsDevice::~FaultsDevice() {
	delete ui;
}

void FaultsDevice::resetStored() {
	for (auto fault : this->m_faultsGroup->getFaults()) {
		fault->setStored(false);
	}
	this->m_faults_explanation->clear();
}

void FaultsDevice::update(uint32_t faults_numeric) {
	this->ui->lineEdit_numeric->setText(QString("0x%1").arg(faults_numeric, 8, 16, QLatin1Char('0')));

	this->m_faultsGroup->update(faults_numeric);

	this->updateExplanations();
}

void FaultsDevice::updateExplanations() {
	m_faults_explanation->clear();
	this->m_faults_explanation->setHtml(this->m_faultsGroup->getExplanations());
}

void FaultsDevice::updateMinimumHeight() {
	this->setMinimumHeight(this->sizeHint().height());
	this->m_faultsGroup->ensurePolished();
}

void FaultsDevice::resizeEvent(QResizeEvent *event) {
	m_faults_explanation->setFixedHeight(m_faults_explanation->document()->size().toSize().height() + 3);

	QWidget::resizeEvent(event);
}
} // adiscope::gui
