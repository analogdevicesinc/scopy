#ifndef SEARCHBARWIDGET_HPP
#define SEARCHBARWIDGET_HPP

#include "scopy-regmapplugin_export.h"

#include <QWidget>

class QHBoxLayout;
class QPushButton;
class QLineEdit;
namespace scopy::regmap {
class SCOPY_REGMAPPLUGIN_EXPORT SearchBarWidget : public QWidget
{
	Q_OBJECT
public:
	explicit SearchBarWidget(QWidget *parent = nullptr);
	~SearchBarWidget();

	void setEnabled(bool enabled);

Q_SIGNALS:
	void requestSearch(QString searchParam);

private:
	QLineEdit *searchBar;
	QPushButton *searchButton;
	QHBoxLayout *layout;
	void applyStyle();
};
} // namespace scopy::regmap
#endif // SEARCHBARWIDGET_HPP
