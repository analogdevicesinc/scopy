#ifndef VERTICALCHANNELMANAGER_H
#define VERTICALCHANNELMANAGER_H

#include <QVBoxLayout>
#include <QWidget>

#include <scopy-gui_export.h>
#include <compositewidget.h>

namespace scopy {
class SCOPY_GUI_EXPORT VerticalChannelManager : public QWidget, public CompositeWidget
{
	Q_OBJECT
public:
	VerticalChannelManager(QWidget *parent = nullptr);
	~VerticalChannelManager();
	void add(QWidget *ch) override;
	void remove(QWidget *ch) override;
	void addEnd(QWidget *ch);

private:
	QSpacerItem *spacer;
	QVBoxLayout *lay;
};
} // namespace scopy

#endif // VERTICALCHANNELMANAGER_H
