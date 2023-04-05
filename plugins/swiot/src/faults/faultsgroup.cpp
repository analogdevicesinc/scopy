#include "faultsgroup.hpp"

using namespace adiscope::swiot;

#define MAX_COLS_IN_GRID 100

FaultsGroup::FaultsGroup(QString name, const QString& path, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::FaultsGroup),
	m_name(std::move(name)),
	m_customColGrid(new CustomColQGridLayout(MAX_COLS_IN_GRID, false, this))
{
	ui->setupUi(this);

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
	if (device_value == QJsonValue::Undefined) { qCritical(CAT_SWIOT_FAULTS) << "Invalid json: Could not extract value " << m_name; }

	QJsonObject device_object = device_value.toObject();
	QJsonValue max_faults_value = device_object["max_faults"]; // max number of faults
	m_max_faults = max_faults_value.toInt(-1);

	QJsonValue faults_json = device_object["faults"];
	QJsonArray faults_obj = faults_json.toArray();

	this->setupDynamicUi();

	for (int i = 0; i < m_max_faults; ++i) {
		QJsonObject fault_object = faults_obj[i].toObject();
		QString fault_name = fault_object.value("name").toString();
		QString fault_description = fault_object.value("description").toString();
		auto fault_widget = new FaultWidget(i, fault_name, fault_description, this);
		connect(fault_widget, &FaultWidget::faultSelected, this, [this] (unsigned int id_){
			bool added = m_currentlySelected.insert(id_).second;
			if (!added) {
				m_currentlySelected.erase(id_);
				m_faults.at(id_)->setPressed(false);
			}
			Q_EMIT selectionUpdated();
		});
		m_faults.push_back(fault_widget);
		m_customColGrid->addQWidgetToList(fault_widget);
	}
	m_customColGrid->toggleAll(true);

	m_customColGrid->itemSizeChanged();
	this->ui->horizontalLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Maximum, QSizePolicy::Minimum));
	connect(m_customColGrid, &CustomColQGridLayout::reqestLayoutUpdate, this, [this] (){
		if (this->ui->activeStoredLayout->count() != m_customColGrid->rows()) {
			while (this->ui->activeStoredLayout->count() < m_customColGrid->rows()) {
				this->ui->activeStoredLayout->addWidget(this->buildActiveStoredWidget());
			}
			while (this->ui->activeStoredLayout->count() > m_customColGrid->rows()) {
				QWidget *widgetToDelete = this->ui->activeStoredLayout->itemAt(this->ui->activeStoredLayout->count() - 1)->widget();
				this->ui->activeStoredLayout->removeWidget(widgetToDelete);
				delete widgetToDelete;
			}
		}

		this->setMinimumHeight(m_customColGrid->minimumHeight());

		Q_EMIT minimumSizeChanged();
	});
}

FaultsGroup::~FaultsGroup()
{
	delete ui;
}

const std::vector<FaultWidget *> &FaultsGroup::getFaults() const {
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
	for (unsigned int i : m_currentlySelected) {
		m_faults[i]->setPressed(false);
	}

	m_currentlySelected.clear();
	Q_EMIT selectionUpdated();
}

void FaultsGroup::update(uint32_t faults_numeric) {
	uint32_t aux = faults_numeric;
	for (int i = (int)(m_faults.size() - 1); i >= 0; --i) {
		bool bit = (bool) (aux & 0b1);
		if (m_faults.at(i)->isActive() && bit) { // if we get 2 active signals, we set the stored to 1
			m_faults.at(i)->setStored(true);
		}
		m_faults.at(i)->setActive(bit);
		aux >>= 1;
	}
}

QString FaultsGroup::getExplanations() {
	QString res = "";
	if (!m_currentlySelected.empty()) {
		for (unsigned int i : m_currentlySelected) {
			if (m_faults.at(i)->isActive()) {
				res += QString("Bit%1 (%2): %3<br>").arg(QString::number(i), m_faults.at(i)->getName(), m_faults.at(i)->getFaultExplanation());
			}
		}
	} else {
		for (auto fault : m_faults) {
			if (fault->isActive()) {
				res += QString("Bit%1 (%2): %3<br>").arg(QString::number(fault->getId()), fault->getName(), fault->getFaultExplanation());
			} else {
				res += QString("<font color=\"#5c5c5c\">Bit%1 (%2): %3</font><br>").arg(QString::number(fault->getId()), fault->getName(), fault->getFaultExplanation());
			}
		}
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

