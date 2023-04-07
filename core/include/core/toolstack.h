#ifndef TOOLSTACK_H
#define TOOLSTACK_H

#include "mapstackedwidget.h"
#include <QMap>
#include "scopycore_export.h"

namespace Ui {
class SCOPYCORE_EXPORT ToolStack;
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

public Q_SLOTS:

	bool show(QString key) override;
	void detachTool(QString key);
	void attachTool(QString key);
	void setAttached(QString key, bool b);
	bool isAttached(QString key);

Q_SIGNALS:
	void detachSuccesful(QString tool);
	void attachSuccesful(QString tool);

};
}

#endif // TOOLSTACK_H
