#ifndef DACADDON_H_
#define DACADDON_H_

#include <QWidget>
#include <QMap>
#include <menucontrolbutton.h>

namespace scopy {
namespace dac {
class DacAddon : public QWidget
{
	Q_OBJECT
public:
	DacAddon(QWidget *parent = nullptr);
	virtual ~DacAddon();
	virtual void enable(bool enable);
	virtual QMap<QString, MenuControlButton *> getChannelBtns();
	virtual QMap<QString, QWidget *> getChannelMenus();
	virtual void setRunning(bool running);
	virtual bool isRunning();
Q_SIGNALS:
	void running(bool enabled);
	void requestChannelMenu(QString uuid);

protected:
	QMap<QString, MenuControlButton *> m_channelBtns;
	QMap<QString, QWidget *> m_channelMenus;
	bool m_isRunning;
};
} // namespace dac
} // namespace scopy

#endif // DACADDON_H_
