#ifndef INFOPAGESTACK_H
#define INFOPAGESTACK_H

#include "mapstackedwidget.h"

namespace adiscope {

// implement slide to-from  next/prev page - emits to button selection group

class InfoPageStack : public MapStackedWidget
{
	Q_OBJECT
public:
	explicit InfoPageStack(QWidget *parent = nullptr);
	~InfoPageStack();

Q_SIGNALS:

};
}
#endif // INFOPAGESTACK_H
