#ifndef TOOLSTACK_H
#define TOOLSTACK_H

#include "mapstackedwidget.h"
#include <QMap>

namespace Ui {
class ToolStack;
}
/**
 * @brief The ToolStack class
 */
namespace adiscope {
class ToolStack : public MapStackedWidget
{
	Q_OBJECT

public:
	explicit ToolStack(QWidget *parent = nullptr);
	~ToolStack();

	bool show(QString key) override;

public Q_SLOTS:
	void detachTool(QString tool);
	void attachTool(QString tool);

Q_SIGNALS:
	void detachSuccesful(QString tool);
	void attachSuccesful(QString tool);

};
}

#endif // TOOLSTACK_H
