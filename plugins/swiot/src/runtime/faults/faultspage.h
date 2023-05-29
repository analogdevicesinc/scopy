#ifndef FAULTSPAGE_H
#define FAULTSPAGE_H

#include <QWidget>
#include <QPushButton>
#include <QTextEdit>

#include <iio.h>

#include "faultsgroup.h"
#include "faultsdevice.h"

namespace Ui { class FaultsPage; }

namespace scopy::swiot {
class FaultsDevice;

class FaultsPage : public QWidget {
	Q_OBJECT
public:
	explicit FaultsPage(struct iio_context* context, QWidget *parent = nullptr);
	~FaultsPage() override;

	void update();

private:
	struct iio_context* m_context;

	Ui::FaultsPage *ui;

	FaultsDevice* m_ad74413rFaultsDevice;
	FaultsDevice* m_max14906FaultsDevice;

	void setupDevices();
};
}
#endif // FAULTSPAGE_H
