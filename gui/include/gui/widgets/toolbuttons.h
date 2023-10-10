#ifndef TOOLBUTTONS_H
#define TOOLBUTTONS_H

#include "semiexclusivebuttongroup.h"

#include <QPushButton>

#include <menu_anim.hpp>
#include <scopy-gui_export.h>
#include <utils.h>

namespace scopy {

class SCOPY_GUI_EXPORT PrintBtn : public QPushButton
{
	Q_OBJECT
public:
	PrintBtn(QWidget *parent = nullptr);
};

class SCOPY_GUI_EXPORT OpenLastMenuBtn : public QPushButton
{
	Q_OBJECT
public:
	OpenLastMenuBtn(MenuHAnim *menu, bool opened, QWidget *parent = nullptr);
	QButtonGroup *getButtonGroup();

private:
	MenuHAnim *m_menu;
	SemiExclusiveButtonGroup *grp;
};

/* Refactor these as Stylehelper ? */
class SCOPY_GUI_EXPORT GearBtn : public QPushButton
{
	Q_OBJECT
public:
	GearBtn(QWidget *parent = nullptr);
};

class SCOPY_GUI_EXPORT InfoBtn : public QPushButton
{
	Q_OBJECT
public:
	InfoBtn(QWidget *parent = nullptr);
};

class SCOPY_GUI_EXPORT RunBtn : public QPushButton
{
	Q_OBJECT
public:
	RunBtn(QWidget *parent = nullptr);
};

class SCOPY_GUI_EXPORT SingleShotBtn : public QPushButton
{
	Q_OBJECT
public:
	SingleShotBtn(QWidget *parent = nullptr);
};

} // namespace scopy

#endif
