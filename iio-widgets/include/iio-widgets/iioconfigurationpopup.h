#ifndef IIOCONFIGURATIONPOPUP_H
#define IIOCONFIGURATIONPOPUP_H

#include "iiowidgetselector.h"
#include "scopy-iio-widgets_export.h"
#include <gui/tintedoverlay.h>
#include <iioutil/iioitem.h>
#include <iio.h>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTreeView>

namespace scopy {
class SCOPY_IIO_WIDGETS_EXPORT IIOConfigurationPopup : public QWidget
{
	Q_OBJECT
public:
	explicit IIOConfigurationPopup(iio_context *ctx, QWidget *parent = nullptr);
	explicit IIOConfigurationPopup(iio_device *dev, QWidget *parent = nullptr);
	explicit IIOConfigurationPopup(iio_channel *chnl, QWidget *parent = nullptr);
	~IIOConfigurationPopup();

	void enableTintedOverlay(bool enable = true);

Q_SIGNALS:
	void selectButtonClicked(IIOItem *selected);
	void exitButtonClicked();

protected Q_SLOTS:
	void modelSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

protected:
	void init();
	void initUI();

	gui::TintedOverlay *m_tintedOverlay;
	QLabel *m_titleLabel;
	IIOWidgetSelector *m_widgetSelector;
	QPushButton *m_exitButton;
	QPushButton *m_selectButton;
	IIOItem *m_root;
};
} // namespace scopy

#endif // IIOCONFIGURATIONPOPUP_H
