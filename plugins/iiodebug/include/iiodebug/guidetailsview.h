#ifndef GUIDETAILSVIEW_H
#define GUIDETAILSVIEW_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include "iiostandarditem.h"
#include <gui/subsection_separator.hpp>

namespace scopy::iiodebugplugin {
class GuiDetailsView : public QWidget
{
	Q_OBJECT
public:
	explicit GuiDetailsView(QWidget *parent = nullptr);

	void setupUi();
	void setIIOStandardItem(IIOStandardItem *item);

private:
	IIOStandardItem *m_currentItem;
	gui::SubsectionSeparator *m_attrSeparator;
	gui::SubsectionSeparator *m_detailsSeparator;
	QScrollArea *m_scrollArea;
	QWidget *m_scrollAreaContents;
	QList<IIOWidget *> m_currentWidgets;
	QList<QLabel *> m_detailsList;

	void clearWidgets();
};
} // namespace scopy::iiodebugplugin

#endif // GUIDETAILSVIEW_H
