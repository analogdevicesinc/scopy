#ifndef VERTICALCHANNELMANAGER_H
#define VERTICALCHANNELMANAGER_H

#include <QVBoxLayout>
#include <QWidget>

#include <scopy-gui_export.h>
#include <compositewidget.h>

class QScrollArea;
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
	QVBoxLayout *m_contLayout;
	QScrollArea *m_scrollArea;
	QWidget *m_container;
};
} // namespace scopy

#endif // VERTICALCHANNELMANAGER_H
