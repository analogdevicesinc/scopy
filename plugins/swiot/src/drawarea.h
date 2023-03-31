//
// Created by andrei-fabian on 30/03/23.
//

#ifndef TOOL_LAUNCHER_DRAWAREA_H
#define TOOL_LAUNCHER_DRAWAREA_H

#include <QWidget>
#include <QtSvg/QSvgWidget>

class DrawArea : public QWidget
{
        Q_OBJECT

public:
        explicit DrawArea(QWidget* parent = nullptr);
        ~DrawArea() override;

        void addLine(QLine* line);
        void removeLine(int index);
        std::vector<QLine*> getLines();

        void changeLine();
        void setSize(QSize newSize);

private:
        QString filePath = ":/swiot/1_adobe_express.svg";
        QString redLine = R"(<line x1="10" y1="10" x2="100" y2="100" stroke="red" stroke-width="10" />)";

        std::vector<QLine*> lines;
        QSvgWidget *svgWidget;
};


#endif //TOOL_LAUNCHER_DRAWAREA_H
