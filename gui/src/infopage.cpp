#include "infopage.h"

using namespace adiscope;

InfoPage::InfoPage(QWidget *parent) :
	QWidget(parent)
{
	scroll = new QScrollArea(this);
	lay = new QVBoxLayout(this);
}

InfoPage::~InfoPage()
{

}

void InfoPage::update(QString key, QString value)
{
	if(!map.contains(key)) {
		map.insert(key,value);
		InfoPageKeyValueWidget *w = new InfoPageKeyValueWidget(key,value, this);
		w->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
		wmap.insert(key,w);
		lay->addWidget(w);

	} else {
		map[key] = value;
		wmap[key]->updateValue(value);
	}
}

