#ifndef FMCOMMS5_H
#define FMCOMMS5_H

#include "scopy-ad936x_export.h"
#include <QBoxLayout>
#include <QWidget>
#include <tooltemplate.h>

#include <iio-widgets/iiowidgetbuilder.h>

#include <animatedrefreshbtn.h>
#include <ad936xhelper.h>

namespace scopy {
namespace ad936x {
class SCOPY_AD936X_EXPORT FMCOMMS5 : public QWidget
{
    Q_OBJECT
public:
    explicit FMCOMMS5(iio_context *ctx, QWidget *parent = nullptr);
    ~FMCOMMS5();


Q_SIGNALS:
        void readRequested();


private:
        iio_context *m_ctx = nullptr;
        ToolTemplate *m_tool;
        QVBoxLayout *m_mainLayout;
        QWidget *m_controlsWidget;
        QWidget *m_blockDiagramWidget;
        AnimatedRefreshBtn *m_refreshButton;

        QWidget *generateRxChainWidget(iio_device *dev, QString title, QWidget *parent);
        QWidget *generateTxChainWidget(iio_device *dev, QString title, QWidget *parent);

        AD936xHelper *m_helper;
};
} // namespace ad936x
} // namespace scopy
#endif // FMCOMMS5_H
