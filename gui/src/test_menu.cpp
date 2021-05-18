#include <QVBoxLayout>

#include <scopy/gui/test_menu.hpp>

using namespace scopy::gui;

TestMenu::TestMenu(QWidget* parent)
	: GenericMenu(parent)
{
	GenericMenu::initInteractiveMenu();

	GenericMenu::setMenuHeader("My test menu", new QColor("pink"), true);

	SubsectionSeparator* firstSec = new SubsectionSeparator(parent);
	firstSec->setLabel("First");
	firstSec->setContent(new QPushButton);
	GenericMenu::insertSection(firstSec);

	SubsectionSeparator* secondSec = new SubsectionSeparator(parent);
	secondSec->setLabel("Second");
	secondSec->setContent(new QPushButton);
	GenericMenu::insertSection(secondSec);

	SubsectionSeparator* thirdSec = new SubsectionSeparator(parent);
	thirdSec->setLabel("Third");
	thirdSec->setContent(new QPushButton);
	GenericMenu::insertSection(thirdSec);
}
