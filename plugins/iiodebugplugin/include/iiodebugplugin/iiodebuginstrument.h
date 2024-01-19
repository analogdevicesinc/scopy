#ifndef IIODEBUGINSTRUMENT_H
#define IIODEBUGINSTRUMENT_H

#include "scopy-iiodebugplugin_export.h"
#include "iiomodel.h"
#include "detailsview.h"
#include "searchbar.h"

#include <iio.h>
#include <QWidget>
#include <QTreeView>

namespace scopy::iiodebugplugin {
class SCOPY_IIODEBUGPLUGIN_EXPORT IIODebugInstrument : public QWidget
{
	Q_OBJECT
public:
	IIODebugInstrument(struct iio_context *context, QWidget *parent = nullptr);
	~IIODebugInstrument();

private:
	void setupUi();

	struct iio_context *m_context;
	QTreeView *m_treeView;
	IIOModel *m_iioModel;
	DetailsView *m_detailsView;
	SearchBar *m_searchBar;
};
} // namespace scopy::iiodebugplugin
#endif // IIODEBUGINSTRUMENT_H
