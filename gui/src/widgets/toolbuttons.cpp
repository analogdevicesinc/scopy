
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
	QString style = QString(R"css(
			QPushButton {
				width: 88px;
				height: 48px;
					border-radius: 2px;
					text-align: left;
					padding-left: 20px;
					padding-right: 20px;
				color: white;
					font-weight: 700;
					font-size: 14px;
			}

			QPushButton:!checked {
				background-color: #27b34f;
			}

			  QPushButton:checked {
				background-color: #F45000;
			}

			QPushButton:disabled {
				background-color: grey;
			})css");

	setStyleSheet(style);
	QIcon icon1;
	icon1.addPixmap(Style::getPixmap(":/gui/icons/play.svg"), QIcon::Normal, QIcon::Off);
	icon1.addPixmap(Style::getPixmap(":/gui/icons/scopy-default/icons/play_stop.svg"),
			QIcon::Normal, QIcon::On);
	setIcon(icon1);
	setLayoutDirection(Qt::RightToLeft);
}

SingleShotBtn::SingleShotBtn(QWidget *parent)
	: QPushButton(parent)
{
	setObjectName("single_btn");
	setCheckable(true);
	setChecked(false);
	setText("Single");
	connect(this, &QPushButton::toggled, this, [=](bool b) { setText(b ? "Stop" : "Single"); });
	QString style = QString(R"css(
			QPushButton {
				width: 88px;
				height: 48px;
					border-radius: 2px;
					text-align: left;
					padding-left: 20px;
					padding-right: 20px;
				color: white;
					font-weight: 700;
					font-size: 14px;
				opacity: 80;
			}

			QPushButton:!checked {
				background-color: #ff7200;
			}

			  QPushButton:checked {
				background-color: #F45000;
			}

			QPushButton:disabled {
				background-color: grey;
			}
			)css");

	setStyleSheet(style);
	QIcon icon1;
	icon1.addPixmap(Style::getPixmap(":/gui/icons/play_oneshot.svg"), QIcon::Normal, QIcon::Off);
	icon1.addPixmap(Style::getPixmap(":/gui/icons/scopy-default/icons/play_stop.svg"),
			QIcon::Normal, QIcon::On);
	setIcon(icon1);
	setLayoutDirection(Qt::RightToLeft);
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
