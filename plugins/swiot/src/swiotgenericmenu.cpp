#include "swiotgenericmenu.hpp"
#include <QHBoxLayout>

using namespace adiscope::gui;

SwiotGenericMenu::SwiotGenericMenu(QWidget *parent):
	GenericMenu(parent)
      ,m_swiotAdvMenu(nullptr)
{}

SwiotGenericMenu::~SwiotGenericMenu()
{}

void SwiotGenericMenu::init(QString title, QString function, QColor* color)
{
	initInteractiveMenu();
	setMenuHeader(title,color,false);

	m_advanceSettingsSection = new adiscope::gui::SubsectionSeparator("Advance settings", false);
	insertSection(m_advanceSettingsSection);

	m_swiotAdvMenu = SwiotAdvMenuBuilder::newAdvMenu(m_advanceSettingsSection->getContentWidget(), function);

}
void SwiotGenericMenu::initAdvMenu(QMap<QString, QStringList> values)
{
	m_swiotAdvMenu->setAttrValues(values);
	m_swiotAdvMenu->init();

	QVector<QHBoxLayout *> layers = m_swiotAdvMenu->getMenuLayers();
	for (int i = 0; i < layers.size(); i++) {
		m_advanceSettingsSection->getContentWidget()->layout()->addItem(layers[i]);
	}

}
void SwiotGenericMenu::connectMenuToOsc()
{}

SwiotAdvMenu* SwiotGenericMenu::getAdvMenu()
{
	return m_swiotAdvMenu;
}
