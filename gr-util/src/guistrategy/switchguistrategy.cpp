#include "guistrategy/switchguistrategy.h"
#include <QLoggingCategory>

using namespace scopy::attr;

Q_LOGGING_CATEGORY(CAT_SWITCHGUISTRATEGY, "SwitchGuiStrategy")

SwitchAttrUi::SwitchAttrUi(AttributeFactoryRecipe recipe, QObject *parent)
	: m_ui(new QWidget(nullptr))
	, m_optionsList(new QStringList)
{
	setParent(parent);
	m_ui->setLayout(new QVBoxLayout(m_ui));
	m_ui->layout()->setMargin(0);
	m_menuBigSwitch = new CustomSwitch(m_ui);	
	m_ui->layout()->addWidget(m_menuBigSwitch);
	Q_EMIT requestData();

	connect(m_menuBigSwitch, &QPushButton::clicked, this, [this]() {
		QString currentSelection = (m_menuBigSwitch->isChecked()) ? m_optionsList->at(0) : m_optionsList->at(1);
		Q_EMIT emitData(currentSelection);
	});
}

SwitchAttrUi::~SwitchAttrUi()
{
	delete m_optionsList;
	m_ui->deleteLater();
}

QWidget *SwitchAttrUi::ui() { return m_ui; }

bool SwitchAttrUi::isValid()
{
	if(m_recipe.channel != nullptr && m_recipe.data != "" && m_recipe.dataOptions != "") {
		return true;
	}
	return false;
}

void SwitchAttrUi::receiveData(QString currentData, QString optionalData)
{
	QSignalBlocker blocker(m_menuBigSwitch);
	QStringList optionsList = QString(optionalData).split(" ", Qt::SkipEmptyParts);
	if(optionsList.size() < 2) {
		qWarning(CAT_SWITCHGUISTRATEGY) << "Received less than 2 options, should this be a switch?";
		return;
	}
	if(optionsList.size() > 2) {
		qDebug(CAT_SWITCHGUISTRATEGY) << "Received more than 2 options, only the first 2 will be used.";
	}
	*m_optionsList = optionsList;
	m_menuBigSwitch->setOnText(optionsList[0]);
	m_menuBigSwitch->setOffText(optionsList[1]);
	if(optionsList[0] == QString(currentData)) {
		m_menuBigSwitch->setChecked(true);
	} else {
		m_menuBigSwitch->setChecked(false);
	}
}

#include "moc_switchguistrategy.cpp"
