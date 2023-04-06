#ifndef FAULTSPAGE_HPP
#define FAULTSPAGE_HPP

#include <QWidget>
#include <QPushButton>
#include <QTextEdit>

#include "faultsgroup.hpp"
#include "faultsdevice.hpp"

namespace Ui { class FaultsPage; }

namespace adiscope::swiot {
class FaultsDevice;

class FaultsPage : public QWidget {
	Q_OBJECT
public:
	explicit FaultsPage(QWidget *parent = nullptr);
	~FaultsPage() override;

	void update(uint32_t ad74413r_faults, uint32_t max14906_faults);

private:
	Ui::FaultsPage *ui;

	FaultsDevice* m_ad74413rFaultsDevice;
	FaultsDevice* m_max14906FaultsDevice;
};
}
#endif // FAULTSPAGE_HPP
