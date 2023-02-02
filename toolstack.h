#ifndef TOOLSTACK_H
#define TOOLSTACK_H

#include <QStackedWidget>
#include <QMap>

namespace Ui {
class ToolStack;
}

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

private:
	QMap<QString, QWidget*> map;

};

#endif // TOOLSTACK_H
