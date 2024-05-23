#ifndef CLIDETAILSVIEW_H
#define CLIDETAILSVIEW_H

#include <QWidget>
#include <QTextBrowser>
#include "iiostandarditem.h"

namespace scopy::iiodebugplugin {
class CliDetailsView : public QWidget
{
	Q_OBJECT
public:
	explicit CliDetailsView(QWidget *parent = nullptr);
	void setIIOStandardItem(IIOStandardItem *item);
	void refreshView();

Q_SIGNALS:
	void addToWatchlist(IIOStandardItem *item);
	void removeFromWatchlist(IIOStandardItem *item);

private:
	void setupUi();
	void setupChannelAttr();
	void setupChannel();
	void setupDeviceAttr();
	void setupDevice();
	void setupContextAttr();
	void setupContext();

	QString tabs(int level);

	IIOStandardItem *m_currentItem;
	QTextBrowser *m_textBrowser;

	// used for creating the iio info view
	IIOStandardItem *m_channelIIOItem;
	IIOStandardItem *m_deviceIIOItem;
	IIOStandardItem *m_contextIIOItem;

	QString m_currentText;
	QString m_deviceAttrsString;

	int m_noCtxAttributes;
	int m_noDevices;
	int m_noDevAttributes;
	int m_noChannels;
	int m_noChnlAttributes;
	int m_globalLevel;
};
} // namespace scopy::iiodebugplugin

#endif // CLIDETAILSVIEW_H
