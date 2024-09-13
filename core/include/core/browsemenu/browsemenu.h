#ifndef BROWSEMENU_H
#define BROWSEMENU_H

#include "instrumentmenu.h"

#include <QLabel>
#include <QPushButton>
#include <QWidget>
#include <scopy-core_export.h>

namespace scopy {

class SCOPY_CORE_EXPORT BrowseMenu : public QWidget
{
	Q_OBJECT
public:
	enum MenuAlignment
	{
		MA_TOPLAST,
		MA_BOTTOMLAST
	};

	BrowseMenu(QWidget *parent = nullptr);
	~BrowseMenu();

	InstrumentMenu *instrumentMenu() const;

Q_SIGNALS:
	void requestTool(QString tool);
	void requestSave();
	void requestLoad();
	void collapsed(bool collapsed);

private:
	void add(QWidget *w, QString name, MenuAlignment position);
	void toggleCollapsed();
	QPushButton *createBtn(QString name, QString iconPath, QWidget *parent = nullptr);
	QFrame *createHLine(QWidget *parent = nullptr);
	QWidget *createHeader(QWidget *parent = nullptr);
	QLabel *createScopyLogo(QWidget *parent = nullptr);

	QWidget *m_content;
	QVBoxLayout *m_contentLay;
	QSpacerItem *m_spacer;
	InstrumentMenu *m_instrumentMenu;
	QPushButton *m_btnCollapse;
	bool m_collapsed;
};
} // namespace scopy

#endif // BROWSEMENU_H
