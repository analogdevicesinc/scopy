#ifndef PATTERN_GENERATOR_H
#define PATTERN_GENERATOR_H

#include <QWidget>
#include <QPushButton>
#include <vector>

#include "src/pulseview/pv/devices/binarybuffer.hpp"
#include "iio_manager.hpp"
#include "filter.hpp"
#include "streams_to_short.h"
#include <gnuradio/blocks/file_descriptor_sink.h>


namespace pv {
    class MainWindow;
    class DeviceManager;
    namespace toolbars {
        class MainBar;
    }
}

namespace sigrok {
    class Context;
}

namespace Glibmm {

}

namespace Ui {
    class PatternGenerator;
}

namespace adiscope {
class PatternGenerator : public QWidget
{
    Q_OBJECT

public:
    explicit PatternGenerator(struct iio_context *ctx, Filter* filt, QPushButton *runButton, QWidget *parent = 0);
    ~PatternGenerator();

private Q_SLOTS:
    void startStop(bool start);

private:
//    pv::devices::PatternGenerator *pattern_generator_device;

    std::shared_ptr<pv::devices::BinaryBuffer> pattern_generator_ptr;
    Ui::PatternGenerator *ui;
    QButtonGroup *settings_group;
    QPushButton *menuRunButton;
    boost::shared_ptr<iio_manager> manager;
    iio_manager::port_id* ids;
    struct iio_context *ctx;
    struct iio_device *dev;
    unsigned int no_channels;
    unsigned int itemsize;

    std::map<std::string, Glib::VariantBase> options;

    short buffer[65536];
    int fd;
    adiscope::streams_to_short::sptr sink_streams_to_short;
    gr::blocks::file_descriptor_sink::sptr sink_fd_block;
    pv::MainWindow* main_win;

    void disconnectAll();
    static unsigned int get_no_channels(struct iio_device *dev);
    void create_fifo();

    void toggleRightMenu(QPushButton *btn);
    bool menuOpened;

    static QStringList digital_trigger_conditions;

private Q_SLOTS:
    void toggleRightMenu();
};
} /* namespace adiscope */

#endif // LOGIC_ANALYZER_H

