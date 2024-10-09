#include <style.h>
#include <QStylePainter>
#include <QStyleOptionTab>
#include <QPainter>
#include <verticaltabbar.h>

using namespace scopy;

VerticalTabBar::VerticalTabBar(QWidget *parent)
	: QTabBar(parent)
{}

void VerticalTabBar::paintEvent(QPaintEvent *event)
{
	QStylePainter painter(this);
	QStyleOptionTab opt;

	QColor tab_selected =
		Style::getColor(json::theme::interactive_accent_pressed); // Background color of the selected tab
	QColor tab_text = Style::getColor(json::theme::content_default);     // Text color of the tabs
	int borderRadius = Style::getDimension(json::global::radius_1); // Radius for the rounded corners
	int padding = Style::getDimension(json::global::padding_1);	// Padding for the tab

	for(int i = 0; i < count(); ++i) {
		initStyleOption(&opt, i);

		QRect rect = tabRect(i);
		rect.adjust(padding, padding, -padding, -padding); // Adjust the rectangle to add padding

		if(currentIndex() == i) {
			painter.setBrush(tab_selected);
			painter.setPen(Qt::NoPen);
			painter.drawRoundedRect(rect, borderRadius, borderRadius);
			painter.setPen(tab_text);
		}

		painter.drawText(rect, Qt::AlignCenter, tabText(i));
	}
}
