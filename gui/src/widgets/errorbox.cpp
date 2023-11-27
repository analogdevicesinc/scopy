#include "widgets/errorbox.h"

using namespace scopy;

ErrorBox::ErrorBox(QWidget *parent)
	: QFrame(parent)
{
	setStyleSheet("background-color: transparent; border: 1px solid black;");
	setFixedSize(10, 10);
}

void ErrorBox::changeColor(AvailableColors color)
{
	QString colorString;
	switch(color) {
	case Green:
		colorString = "green";
		break;
	case Yellow:
		colorString = "yellow";
		break;
	case Red:
		colorString = "red";
		break;
	case Transparent:
		colorString = "transparent";
		break;
	}
	setStyleSheet("background-color: " + colorString + "; border: 1px solid black;");
}

#include "moc_errorbox.cpp"
