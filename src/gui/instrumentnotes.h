#ifndef INSTRUMENTNOTES_H
#define INSTRUMENTNOTES_H

#include <QWidget>

namespace Ui {
class InstrumentNotes;
}
namespace adiscope {
class InstrumentNotes : public QWidget
{
	Q_OBJECT

public:
	explicit InstrumentNotes(QWidget *parent = nullptr);
	~InstrumentNotes();

public Q_SLOTS:
	QString getNotes();
	void setNotes(QString);

private:
	Ui::InstrumentNotes *ui;
};
}

#endif // INSTRUMENTNOTES_H
