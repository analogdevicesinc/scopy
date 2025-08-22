#ifndef FMCOMMS5ADVANCED_H
#define FMCOMMS5ADVANCED_H

#include "scopy-ad936x_export.h"
#include <QWidget>
#include <animatedrefreshbtn.h>
#include <tooltemplate.h>
#include <iio.h>

#include "auxadcdaciowidget.h"
#include "elnawidget.h"
#include "ensmmodeclockswidget.h"
#include "gainwidget.h"
#include "rssiwidget.h"
#include "txmonitorwidget.h"
#include "miscwidget.h"
#include "bistwidget.h"

namespace scopy {
namespace ad936x {
class SCOPY_AD936X_EXPORT Fmcomms5Advanced : public QWidget
{
    Q_OBJECT
public:
    explicit Fmcomms5Advanced(iio_context *ctx, QWidget *parent = nullptr);
    ~Fmcomms5Advanced();


Q_SIGNALS:
        void readRequested();

private:
        void init();

        QPushButton *m_ensmModeClocksBtn = nullptr;
        QPushButton *m_eLnaBtn = nullptr;
        QPushButton *m_rssiBtn = nullptr;
        QPushButton *m_gainBtn = nullptr;
        QPushButton *m_txMonitorBtn = nullptr;
        QPushButton *m_auxAdcDacIioBtn = nullptr;
        QPushButton *m_miscBtn = nullptr;
        QPushButton *m_bistBtn = nullptr;
        QPushButton *m_fmcomms5Btn = nullptr;


        QPushButton *m_syncBtn = nullptr;
        QPushButton *m_saveSettingsBtn = nullptr;

        iio_context *m_ctx = nullptr;
        ToolTemplate *m_tool;
        QVBoxLayout *m_mainLayout;
        AnimatedRefreshBtn *m_refreshButton;
        EnsmModeClocksWidget *m_ensmModeClocks;
        ElnaWidget *m_elna;
        RssiWidget *m_rssi;
        GainWidget *m_gainWidget;
        TxMonitorWidget *m_txMonitor;
        AuxAdcDacIoWidget *m_auxAdcDacIo;
        MiscWidget *m_misc;
        BistWidget *m_bist;

        iio_device *m_plutoDevice = nullptr;
        QStackedWidget *m_centralWidget = nullptr;

        bool m_isToolInitialized;
        void showEvent(QShowEvent *event) override;
};

} // namespace ad936x
} // namespace scopy
#endif // FMCOMMS5ADVANCED_H
