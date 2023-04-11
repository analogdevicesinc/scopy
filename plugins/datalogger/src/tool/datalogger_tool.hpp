#ifndef DATALOGGERTOOL_HPP
#define DATALOGGERTOOL_HPP

#include <QPushButton>
#include <QMap>
#include <QScrollArea>
#include "src/refactoring/tool/tool_view.hpp"
#include "datalogger.hpp"

extern "C"{
	struct iio_context;
}

namespace adiscope::datalogger {

class DataloggerTool: public QWidget
{
public:
	explicit DataloggerTool(struct iio_context *ctx, QWidget *parent = nullptr);
	~DataloggerTool();

private:
	QWidget* parent;
	QMap<QString, struct iio_device*> m_iioDevices;
	struct iio_context *ctx;
	adiscope::gui::ToolView* m_toolView;

	QScrollArea* m_scrollArea;

	DataLogger* datalogger;

protected:
	bool eventFilter(QObject *object, QEvent * event) override;

private:
	void init();
};
}


#endif // DATALOGGERTOOL_HPP
