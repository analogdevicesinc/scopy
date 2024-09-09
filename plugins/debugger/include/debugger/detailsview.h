#ifndef SCOPY_DETAILSVIEW_H
#define SCOPY_DETAILSVIEW_H

#include <QWidget>
#include <QTabWidget>
#include <QTextBrowser>
#include "iiostandarditem.h"
#include "guidetailsview.h"
#include "clidetailsview.h"
#include "pathtitle.h"

namespace scopy::debugger {
class DetailsView : public QWidget
{
	Q_OBJECT
public:
	explicit DetailsView(QWidget *parent = nullptr);
	void setIIOStandardItem(IIOStandardItem *item);
	void refreshIIOView();

	QPushButton *readBtn();
	QPushButton *addToWatchlistBtn();

	// add:true = +, add:false = X
	void setAddToWatchlistState(bool add);

Q_SIGNALS:
	void pathSelected(QString path);

private:
	void setupUi();

	IIOStandardItem *m_currentItem;
	GuiDetailsView *m_guiDetailsView;
	CliDetailsView *m_cliDetailsView;
	QTabWidget *m_tabWidget;
	QWidget *m_guiView;
	QWidget *m_iioView;
	QWidget *m_titleContainer;
	PathTitle *m_titlePath;
	QPushButton *m_readBtn;
	QPushButton *m_addToWatchlistBtn;
};
} // namespace scopy::debugger

#endif // SCOPY_DETAILSVIEW_H
