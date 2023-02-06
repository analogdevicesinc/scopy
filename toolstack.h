#ifndef TOOLSTACK_H
#define TOOLSTACK_H

#include <QStackedWidget>
#include <QMap>

namespace Ui {
class ToolStack;
}
/**
 * @brief The ToolStack class
 */
namespace adiscope {
class ToolStack : public QStackedWidget
{
	Q_OBJECT

public:
	explicit ToolStack(QWidget *parent = nullptr);
	~ToolStack();

public Q_SLOTS:

	void addTool(QString tool, QWidget* w);
	void showTool(QString tool);
	void removeTool(QString tool);
	void detachTool(QString tool);
	void attachTool(QString tool);

Q_SIGNALS:
	void detachSuccesful(QString tool);
	void attachSuccesful(QString tool);

private:
	QMap<QString, QWidget*> map;

};
}

#endif // TOOLSTACK_H
