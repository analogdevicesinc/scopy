#ifndef TWOBUTTONSWIDGET_H
#define TWOBUTTONSWIDGET_H

#include <QWidget>

namespace Ui {
class TwoButtonsWidget;
}

namespace scopy {
namespace gui {
class TwoButtonsWidget : public QWidget {
  Q_OBJECT

public:
  explicit TwoButtonsWidget(const QString &text, QWidget *parent = nullptr);
  virtual ~TwoButtonsWidget();

private:
  Ui::TwoButtonsWidget *ui;
};
} // namespace gui
} // namespace scopy
#endif // TWOBUTTONSWIDGET_H
