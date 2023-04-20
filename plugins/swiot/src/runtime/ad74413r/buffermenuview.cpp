#include "buffermenuview.hpp"
#include <QHBoxLayout>

using namespace scopy;

swiot::BufferMenuView::BufferMenuView(QWidget *parent):
	GenericMenu(parent)
      ,m_swiotAdvMenu(nullptr)
{}

swiot::BufferMenuView::~BufferMenuView()
{}

void swiot::BufferMenuView::init(QString title, QString function, QColor* color)
{
	initInteractiveMenu();
	setMenuHeader(title,color,false);

	m_advanceSettingsSection = new gui::SubsectionSeparator("Advance settings", false);
	insertSection(m_advanceSettingsSection);

	m_swiotAdvMenu = swiot::BufferMenuBuilder::newAdvMenu(m_advanceSettingsSection->getContentWidget(), function);

}

void swiot::BufferMenuView::initAdvMenu(QMap<QString, QStringList> values)
{
	m_swiotAdvMenu->setAttrValues(values);
	m_swiotAdvMenu->init();

	QVector<QHBoxLayout *> layers = m_swiotAdvMenu->getMenuLayers();
	for (int i = 0; i < layers.size(); i++) {
		m_advanceSettingsSection->getContentWidget()->layout()->addItem(layers[i]);
	}

}

swiot::BufferMenu* swiot::BufferMenuView::getAdvMenu()
{
	return m_swiotAdvMenu;
}
