
#include <style.h>
#include <stylehelper.h>
#include <widgets/toolbuttons.h>

using namespace scopy;

PrintBtn::PrintBtn(QWidget *parent)
	: QPushButton(parent)
{
	StyleHelper::BlueGrayButton(this, "print_btn");
	setFixedWidth(128);
	setCheckable(false);
	setText("Print");
}

OpenLastMenuBtn::OpenLastMenuBtn(MenuHAnim *menu, bool opened, QWidget *parent)
	: QPushButton(parent)
	, m_menu(menu)
{
	QIcon icon1;
	icon1.addPixmap(Style::getPixmap(":/gui/icons/scopy-default/icons/setup3_unchecked_hover.svg"));
	setIcon(icon1);
	StyleHelper::SquareToggleButtonWithIcon(this, "open_last_menu_btn", true);
	setChecked(opened);
	grp = new SemiExclusiveButtonGroup(this);
	connect(this, &QPushButton::toggled, m_menu, &MenuHAnim::toggleMenu);
	connect(grp, &SemiExclusiveButtonGroup::buttonSelected, this, [=](QAbstractButton *btn) {
		if(btn == nullptr) {
			this->setChecked(false);
		} else {
			this->setChecked(true);
		}
	});
	connect(this, &QAbstractButton::toggled, this, [=](bool b) {
		if(b) {
			grp->getLastButton()->setChecked(true);
		} else {
			grp->getLastButton()->setChecked(false);
		}
	});
}

QButtonGroup *OpenLastMenuBtn::getButtonGroup() { return grp; }

GearBtn::GearBtn(QWidget *parent)
	: QPushButton(parent)
{
	QIcon icon1;
	icon1.addPixmap(Style::getPixmap(":/gui/icons/scopy-default/icons/gear_wheel.svg"));
	StyleHelper::SquareToggleButtonWithIcon(this, "gear_btn", true);
	setIcon(icon1);
}

InfoBtn::InfoBtn(QWidget *parent)
	: QPushButton(parent)
{
	QIcon icon1;
	icon1.addPixmap(Style::getPixmap(":/gui/icons/scopy-default/icons/info.svg"));
	StyleHelper::SquareToggleButtonWithIcon(this, "info_btn", false);
	setIcon(icon1);
}

RunBtn::RunBtn(QWidget *parent)
	: QPushButton(parent)
{
	setObjectName("run_btn");
	setCheckable(true);
	setChecked(false);
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	setText("Run");
	connect(this, &QPushButton::toggled, this, [=](bool b) { setText(b ? "Stop" : "Run"); });
	Style::setStyle(this, style::button::runSingle);
	setStyleSheet("background-color: " + Style::getAttribute(json::theme::highlight_color) + ";");

	QIcon icon1;
	icon1.addPixmap(Style::getPixmap(":/gui/icons/play.svg", Style::getColor(json::theme::focus_item)),
			QIcon::Normal, QIcon::Off);
	icon1.addPixmap(Style::getPixmap(":/gui/icons/scopy-default/icons/play_stop.svg",
					 Style::getColor(json::theme::focus_item)),
			QIcon::Normal, QIcon::On);
	setIcon(icon1);
}

SingleShotBtn::SingleShotBtn(QWidget *parent)
	: QPushButton(parent)
{
	setObjectName("single_btn");
	setCheckable(true);
	setChecked(false);
	setText("Single");
	connect(this, &QPushButton::toggled, this, [=](bool b) { setText(b ? "Stop" : "Single"); });
	Style::setStyle(this, style::button::runSingle);
	setStyleSheet("background-color: #ff7200;");

	QIcon icon1;
	icon1.addPixmap(
		Style::getPixmap(":/gui/icons/play_oneshot.svg", Style::getColor(json::theme::focus_item)),
		QIcon::Normal, QIcon::Off);
	icon1.addPixmap(Style::getPixmap(":/gui/icons/scopy-default/icons/play_stop.svg",
					 Style::getColor(json::theme::focus_item)),
			QIcon::Normal, QIcon::On);
	setIcon(icon1);
}

#include "moc_toolbuttons.cpp"

AddBtn::AddBtn(QWidget *parent)
{
	QIcon icon1;
	icon1.addPixmap(Style::getPixmap(":/gui/icons/add.svg"));
	StyleHelper::SquareToggleButtonWithIcon(this, "add_btn", false);
	setIcon(icon1);
}

RemoveBtn::RemoveBtn(QWidget *parent)
{
	QIcon icon1;
	icon1.addPixmap(Style::getPixmap(":/gui/icons/red_x.svg"));
	StyleHelper::SquareToggleButtonWithIcon(this, "remove_btn", false);
	setIcon(icon1);
}

SyncBtn::SyncBtn(QWidget *parent)
{
	QIcon icon1;
	icon1.addPixmap(Style::getPixmap(":/gui/icons/scopy-default/icons/gear_wheel.svg"));
	StyleHelper::BlueGrayButton(this, "sync_btn");
	setText("Sync");
	setCheckable(true);
	setIcon(icon1);
}
