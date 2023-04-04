#include "drawarea.h"
#include <QPainter>
#include <QPixmap>
#include <QtXml/QDomElement>
#include <QFile>
#include <QDebug>
#include <QtSvg/QSvgRenderer>
#include <QVBoxLayout>

using namespace adiscope::swiot;

DrawArea::DrawArea(QWidget *parent) :
        QWidget(parent),
        svgWidget(new QSvgWidget(filePath, this))
{
        this->setLayout(new QVBoxLayout(this));
        this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        this->setMinimumSize(300, 450);
        this->setMaximumSize(400, 600);

        this->layout()->addWidget(svgWidget);
}

DrawArea::~DrawArea()
{

}

void DrawArea::addLine(QLine *line) {
        this->lines.push_back(line);
}

void DrawArea::removeLine(int index) {
        this->lines.erase(this->lines.begin() + index);
}

std::vector<QLine*> DrawArea::getLines() {
        return this->lines;
}

void DrawArea::changeLine() {
        QDomDocument document("svgfile");
        QFile file(filePath);
        if (!file.open(QFile::ReadOnly | QFile::Text)) {
                qDebug() << "file not open";
                return;
        }
        if (!document.setContent(&file)) {
                qDebug() << "could not set content";
                file.close();
                return;
        }
        file.close();

        QDomElement dom_element = document.documentElement();
        QDomNode n = dom_element.firstChild();
        while (!n.isNull()) {
                QDomElement e = n.toElement();
//                if (!e.isNull()) {
//                        if (e.attribute("id") == "line1") {
//                                qDebug() << "before:" << e.attribute("stroke");
//                                e.setAttribute("stroke", "blue");
//                                qDebug() << "after:" << e.attribute("stroke");
//
//                                break;
//                        }
//                }
                n = n.nextSibling();
        }

//        qDebug() << document.toString();

        QDomElement elem = document.createElement("line");
        elem.setAttribute("x1", "10");
        elem.setAttribute("y1", "10");
        elem.setAttribute("x2", "100");
        elem.setAttribute("y2", "100");
        elem.setAttribute("stroke", "red");
        elem.setAttribute("stroke-width", "10");
        dom_element.appendChild(elem); // test stroke-opacity attribute

        this->svgWidget->renderer()->load(document.toByteArray());
        this->svgWidget->repaint();
}

void DrawArea::setSize(QSize newSize) {
        this->setBaseSize(newSize);
}
