#include "analysismenu.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>

using namespace scopy::qiqplugin;

AnalysisMenu::AnalysisMenu(QWidget *parent)
	: QWidget(parent)
{
	m_factory = new MenuItemFactory(this);
	setupUI();
	connect(m_factory, &MenuItemFactory::itemParamChanged, this, &AnalysisMenu::updateAnalysisConfig);
}

AnalysisMenu::~AnalysisMenu() {}

void AnalysisMenu::setupUI()
{
	QVBoxLayout *layout = new QVBoxLayout(this);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	layout->setMargin(0);

	// Scroll area for analysis parameters
	QScrollArea *scrollArea = new QScrollArea();
	QWidget *scrollContent = new QWidget();
	scrollContent->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	scrollContent->setLayout(new QVBoxLayout());
	scrollArea->setWidget(scrollContent);
	scrollArea->setWidgetResizable(true);
	layout->addWidget(scrollArea);

	// Apply button
	QPushButton *applyButton = new QPushButton("Apply");
	layout->addWidget(applyButton);

	connect(applyButton, &QPushButton::clicked, this, &AnalysisMenu::applyPressed);
}

QVariantMap AnalysisMenu::getAnalysisConfig() { return m_analysisConfig; }

void AnalysisMenu::createMenu(const QVariantMap &params)
{
	QScrollArea *scrollArea = findChild<QScrollArea *>();
	QWidget *scrollContent = scrollArea->widget();
	QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(scrollContent->layout());

	// Clear existing items
	while(QLayoutItem *item = layout->takeAt(0)) {
		if(item->widget()) {
			item->widget()->deleteLater();
		}
		delete item;
	}

	// Create new menu items
	for(auto it = params.begin(); it != params.end(); ++it) {
		QWidget *menuItem = m_factory->createWidget(it.key(), it.value().toMap());
		if(menuItem) {
			layout->addWidget(menuItem);
		}
	}
}

void AnalysisMenu::updateAnalysisConfig(const QString &field, const QVariant &value)
{
	m_analysisConfig[field] = value;
}
