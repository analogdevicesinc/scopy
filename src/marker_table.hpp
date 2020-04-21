#ifndef MARKER_TABLE_HPP
#define MARKER_TABLE_HPP

#include <QWidget>

class QStandardItemModel;

namespace Ui {
class MarkerTable;
}

namespace adiscope {

class MarkerTable : public QWidget {
	Q_OBJECT

public:
	explicit MarkerTable(QWidget *parent = 0);
	~MarkerTable();

	void addMarker(int mkIdx, int chIdx, const QString &name,
		       double frequency, double magnitude, const QString &type);
	void removeMarker(int mkIdx, int chIdx);
	void updateMarker(int mkIdx, int chIdx, double frequency,
			  double magnitude, const QString &type);
	bool isMarker(int mkIdx, int chIdx);

private:
	enum Columns {
		COL_ID = 0,
		COL_NAME = 1,
		COL_CH = 2,
		COL_FREQ = 3,
		COL_MAGN = 4,
		COL_TYPE = 5,
		NUM_COLUMNS
	};

	Ui::MarkerTable *ui;
	QStandardItemModel *model;

	int rowOfMarker(int mkIdx, int chIdx) const;
};

} /* namespace adiscope */

#endif // MARKER_TABLE_HPP
