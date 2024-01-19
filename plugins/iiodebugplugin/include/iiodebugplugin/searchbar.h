#ifndef SCOPY_SEARCHBAR_H
#define SCOPY_SEARCHBAR_H

#include <QWidget>
#include <QLineEdit>
#include <QCompleter>
#include <QStandardItemModel>
#include <QPushButton>
#include <QLabel>

namespace scopy::iiodebugplugin {
class SearchBar : public QWidget
{
	Q_OBJECT
public:
	explicit SearchBar(const QStringList &options, QWidget *parent = nullptr);

	QLineEdit *getLineEdit();

private:
	QLabel *m_label;
	QLineEdit *m_lineEdit;
	QCompleter *m_completer;
};
} // namespace scopy::iiodebugplugin

#endif // SCOPY_SEARCHBAR_H
