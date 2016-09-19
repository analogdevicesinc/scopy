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

extern "C" {
  struct iio_context;
  struct iio_device;
  struct iio_channel;
  struct iio_buffer;
}

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
    void singleRun();
    void singleRunStop();
    void toggleRightMenu();

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
    struct iio_device *channel_manager_dev;
    struct iio_buffer *txbuf;
    int no_channels;
    bool buffer_created;
    uint16_t channel_enable_mask;
    uint64_t sample_rate;

    std::map<std::string, Glib::VariantBase> options;

    #define BUFFER_SIZE 65535
    short buffer[BUFFER_SIZE];

    uint64_t number_of_samples;
    uint64_t buffersize;
    bool startPatternGeneration(bool cyclic);
    void stopPatternGeneration();
    void dataChanged();

    pv::MainWindow* main_win;

    void toggleRightMenu(QPushButton *btn);
    bool menuOpened;

    static QStringList digital_trigger_conditions;

};
} /* namespace adiscope */

#endif // LOGIC_ANALYZER_H

