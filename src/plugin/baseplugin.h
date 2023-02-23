#ifndef BASEPLUGIN_H
#define BASEPLUGIN_H

#include <QObject>
#include <QWidget>

namespace adiscope {
class BasePlugin : public QObject
{
	Q_OBJECT
protected:
	QWidget *parent;
	bool dockable;

public:
	explicit BasePlugin(QWidget *parent = nullptr, bool dockable = false);
	~BasePlugin();

	virtual void init();
};
}

#endif // BASEPLUGIN_H
