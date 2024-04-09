#ifndef SCOPY_DETAILSVIEW_H
#define SCOPY_DETAILSVIEW_H

#include <QWidget>
#include <QTabWidget>
#include <QTextBrowser>
#include <gui/subsection_separator.hpp>
#include "iiostandarditem.h"
#include "guidetailsview.h"
#include "clidetailsview.h"

namespace scopy::iiodebugplugin {
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

private:
	void setupUi();

	IIOStandardItem *m_currentItem;
	GuiDetailsView *m_guiDetailsView;
	CliDetailsView *m_cliDetailsView;
	QTabWidget *m_tabWidget;
	QWidget *m_guiView;
	QWidget *m_iioView;
	QWidget *m_titleContainer;
	QLabel *m_titleLabel;
	QPushButton *m_readBtn;
	QPushButton *m_addToWatchlistBtn;
};
} // namespace scopy::iiodebugplugin

#endif // SCOPY_DETAILSVIEW_H
