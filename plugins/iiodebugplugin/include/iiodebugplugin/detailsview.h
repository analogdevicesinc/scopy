#ifndef SCOPY_DETAILSVIEW_H
#define SCOPY_DETAILSVIEW_H

#include <QWidget>
#include <QLabel>
#include "iiostandarditem.h"
#include <gui/subsection_separator.hpp>

namespace scopy::iiodebugplugin {
class DetailsView : public QWidget
{
	Q_OBJECT
public:
	explicit DetailsView(QWidget *parent);

	void setTitle(QString title);
	void setIIOStandardItem(IIOStandardItem *item);

private:
	QLabel *m_titleLabel;
	gui::SubsectionSeparator *m_topSeparator;
};
} // namespace scopy::iiodebugplugin

#endif // SCOPY_DETAILSVIEW_H
