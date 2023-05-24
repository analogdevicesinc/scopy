#include "swiotinfopage.h"
#include <QWidget>

using namespace scopy::swiot;

SwiotInfoPage::SwiotInfoPage(QWidget* parent) : QWidget(parent), ui(new Ui::SWIOTInfoPage) {
	this->ui->setupUi(this);
}

QPushButton *SwiotInfoPage::getCtxAttrsButton() const {
	return this->ui->pushButton;
}

SwiotInfoPage::~SwiotInfoPage() {

}

void SwiotInfoPage::setText(const QString &text) {
	this->ui->textBrowser->setText(text);
}
