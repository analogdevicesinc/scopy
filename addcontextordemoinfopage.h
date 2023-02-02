#ifndef ADDCONTEXTORDEMOINFOPAGE_H
#define ADDCONTEXTORDEMOINFOPAGE_H

#include <QWidget>

namespace Ui {
class AddContextOrDemoInfoPage;
}

class AddContextOrDemoInfoPage : public QWidget
{
	Q_OBJECT

public:
	explicit AddContextOrDemoInfoPage(QWidget *parent = nullptr);
	~AddContextOrDemoInfoPage();

private:
	Ui::AddContextOrDemoInfoPage *ui;
};

#endif // ADDCONTEXTORDEMOINFOPAGE_H
