#include "infopage.h"

using namespace scopy;

InfoPage::InfoPage(QWidget *parent) :
	QWidget(parent)
{
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

void InfoPage::clear()
{
	if (!map.empty()) {
		map.clear();
	}

	if (!wmap.empty()) {
		qDeleteAll(wmap);
		wmap.clear();
	}
}


#include "moc_infopage.cpp"
