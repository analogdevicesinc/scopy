#ifndef TOOL_LAUNCHER_DRAWAREA_H
#define TOOL_LAUNCHER_DRAWAREA_H

#include <QWidget>
#include <QtSvg/QSvgWidget>


namespace scopy::swiot {
class DrawArea : public QWidget {
	Q_OBJECT

public:
	explicit DrawArea(QWidget *parent = nullptr);

	~DrawArea() override;

	void addLine(QLine *line);

	void removeLine(int index);

	std::vector<QLine *> getLines();

	void changeLine();

	void setSize(QSize newSize);

private:
	QString filePath;
	QString redLine;

	std::vector<QLine *> lines;
	QSvgWidget *svgWidget;
};
}

#endif //TOOL_LAUNCHER_DRAWAREA_H
