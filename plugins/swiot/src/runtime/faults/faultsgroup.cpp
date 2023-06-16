#include <QWidget>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <utility>

#include "faultsgroup.h"
#include "src/swiot_logging_categories.h"


using namespace scopy::swiot;

#define MAX_COLS_IN_GRID 100

FaultsGroup::FaultsGroup(QString name, const QString &path, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::FaultsGroup),
	m_name(std::move(name)),
    m_customColGrid(new FlexGridLayout(MAX_COLS_IN_GRID, this))
{
	ui->setupUi(this);

	connect(m_customColGrid, &FlexGridLayout::reqestLayoutUpdate, this, [this]() {
		if (this->ui->activeStoredLayout->count() != m_customColGrid->rows()) {
			while (this->ui->activeStoredLayout->count() < m_customColGrid->rows()) {
				this->ui->activeStoredLayout->addWidget(this->buildActiveStoredWidget());
			}
			while (this->ui->activeStoredLayout->count() > m_customColGrid->rows()) {
				QWidget *widgetToDelete = this->ui->activeStoredLayout->itemAt(
					this->ui->activeStoredLayout->count() - 1)->widget();
				this->ui->activeStoredLayout->removeWidget(widgetToDelete);
				delete widgetToDelete;
			}
		}

		this->ui->activeStoredWidget->setMaximumHeight(m_customColGrid->sizeHint().height());
		this->setMaximumHeight(this->sizeHint().height() + 1);

		Q_EMIT minimumSizeChanged();
	});

	QJsonArray *faults_obj = this->getJsonArray(path);
	m_max_faults = faults_obj->size();
	this->setupDynamicUi();

	for (int i = 0; i < m_max_faults; ++i) {
		QJsonObject fault_object = faults_obj->at(i).toObject();
		QString fault_name = fault_object.value("name").toString();
		QString fault_description = fault_object.value("description").toString();
		auto fault_widget = new FaultWidget(i, fault_name, fault_description, this);

		if (fault_object.contains("condition")) {
			QJsonObject condition = fault_object["condition"].toObject();
			fault_widget->setFaultExplanationOptions(condition);
			connect(this, &FaultsGroup::specialFaultsUpdated, fault_widget, &FaultWidget::specialFaultUpdated);
//				if (condition.contains(m_specialFaults.value(i))) {
//					// this logic works because the index variable will be the same as when creating the
//					// m_specialFaults map since the json is read in the same order for both cases
//					// ex: VI_ERR_A matches channel 0, so it will always have the 0 index in m_specialFaults
//					if (
//						(fault_name == " VI_ERR_A " && m_specialFaults.contains(0)) ||
//						(fault_name == " VI_ERR_B " && m_specialFaults.contains(1)) ||
//						(fault_name == " VI_ERR_C " && m_specialFaults.contains(2)) ||
//						(fault_name == " VI_ERR_D " && m_specialFaults.contains(3))
//						)
//					{
//						fault_description = condition[m_specialFaults.value(i)].toString();
		}

		connect(fault_widget, &FaultWidget::faultSelected, this, [this](unsigned int id_) {
			bool added = m_currentlySelected.insert(id_).second;
			if (!added) {
				m_currentlySelected.erase(id_);
				m_faults.at((int) (id_))->setPressed(false);
			}
			Q_EMIT selectionUpdated();
		});
		m_faults.push_back(fault_widget);
		m_customColGrid->addQWidgetToList(fault_widget);
	}
	delete faults_obj;

	m_customColGrid->toggleAll(true);

	m_customColGrid->itemSizeChanged();
}

FaultsGroup::~FaultsGroup() {
	delete ui;
}

//void FaultsGroup::updateSpecialFault(int specialChnIdx)
//{
//	m_faults.at(specialChnIdx)->setFaultExplanation()
//}

const QVector<FaultWidget *> &FaultsGroup::getFaults() const {
	return m_faults;
}

const QString &FaultsGroup::getName() const {
	return m_name;
}

void FaultsGroup::setName(const QString &name_) {
	FaultsGroup::m_name = name_;
}

void FaultsGroup::setupDynamicUi() {
	m_customColGrid->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	this->ui->horizontalLayout->addWidget(m_customColGrid);
}

void FaultsGroup::clearSelection() {
	for (unsigned int i: m_currentlySelected) {
		m_faults[(int) (i)]->setPressed(false);
	}

	m_currentlySelected.clear();
	Q_EMIT selectionUpdated();
}

void FaultsGroup::update(uint32_t faults_numeric) {
	for (int i = (int) (m_faults.size() - 1); i >= 0; --i) {
		bool bit = (bool) (faults_numeric & 0b1);
		if (m_faults.at(i)->isActive() && bit) { // if we get 2 active signals, we set the stored to 1
			m_faults.at(i)->setStored(true);
		}
		m_faults.at(i)->setActive(bit);

		if (bit) {
			m_actives.insert(i);
		} else {
			m_actives.erase(i);
		}

		faults_numeric >>= 1;
	}
}

QStringList FaultsGroup::getExplanations() {
	QStringList res;
	for (auto fault: m_faults) {
		res += QString("Bit%1 (%2): %3").arg(QString::number(fault->getId()),
						     fault->getName(),
						     fault->getFaultExplanation());
	}

	return res;
}

QWidget *FaultsGroup::buildActiveStoredWidget() {
	auto widget = new QWidget(this);
	auto storedLabel = new QLabel(this);
	storedLabel->setText("Stored");
	auto activeLabel = new QLabel(this);
	activeLabel->setText("Active");
	auto spacer = new QSpacerItem(0, 30, QSizePolicy::Fixed, QSizePolicy::Fixed);

	widget->setLayout(new QVBoxLayout(widget));
	widget->layout()->addWidget(storedLabel);
	widget->layout()->addWidget(activeLabel);
	widget->layout()->addItem(spacer);
	widget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	return widget;
}

QJsonArray *FaultsGroup::getJsonArray(const QString &path) {
	QString contents;
	QFile file;
	file.setFileName(path);
	file.open(QIODevice::ReadOnly | QIODevice::Text);
	if (!file.isOpen()) {
		qCritical(CAT_SWIOT_FAULTS) << "File could not be opened (read): " << path;
	} else {
		qDebug(CAT_SWIOT_FAULTS) << "File opened (read): " << path;
	}
	contents = file.readAll();
	file.close();

	QJsonParseError parse_error{};
	QJsonDocument document = QJsonDocument::fromJson(contents.toUtf8(), &parse_error);
	if (document.isNull()) { qCritical(CAT_SWIOT_FAULTS) << "Invalid json: " << parse_error.errorString(); }

	QJsonObject document_object = document.object();
	QJsonValue device_value = document_object.value(m_name);
	if (device_value == QJsonValue::Undefined) {
		qCritical(CAT_SWIOT_FAULTS) << "Invalid json: Could not extract value " << m_name;
	}

	QJsonObject device_object = device_value.toObject();
	QJsonValue max_faults_value = device_object["max_faults"]; // max number of faults
	m_max_faults = max_faults_value.toInt(-1);

	QJsonValue faults_json = device_object["faults"];

	auto *array = new QJsonArray();
	*array = faults_json.toArray();

	return array;
}

std::set<unsigned int> FaultsGroup::getSelectedIndexes() {
	return this->m_currentlySelected;
}

std::set<unsigned int> FaultsGroup::getActiveIndexes() {
	return m_actives;
}
