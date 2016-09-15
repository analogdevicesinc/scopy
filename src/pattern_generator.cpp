#include "ui_pattern_generator.h"

#include "pattern_generator.hpp"
#include <iio.h>
#include <QDebug>

///* pulseview and sigrok */
#include "pulseview/pv/mainwindow.hpp"
#include "pulseview/pv/devices/binarybuffer.hpp"
#include "pulseview/pv/devicemanager.hpp"
#include "pulseview/pv/toolbars/mainbar.hpp"
#include "libsigrokcxx/libsigrokcxx.hpp"
#include "libsigrokdecode/libsigrokdecode.h"

/* gnuradio blocks */
#include <gnuradio/blocks/file_descriptor_sink.h>
#include "streams_to_short.h"

#include "boost/thread.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <vector>

#if _UNIX
    #include <unistd.h>     //check if UNIX or WIN and handle includes
#endif
#if _WIN32
    #include <windows.h>
    #include <io.h>
#endif

#include <boost/thread.hpp>

#define DATA_PIPE "/tmp/myfifo"

using namespace std;
using namespace adiscope;

namespace pv {
    class MainWindow;
    class DeviceManager;
    class Session;

    namespace toolbars {
    class MainBar;
    }

    namespace widgets {
    class DeviceToolButton;
    }

}

namespace sigrok {
    class Context;
}

namespace Glibmm {

}

namespace adiscope {


QStringList PatternGenerator::digital_trigger_conditions = QStringList()
    << "edge-rising"
    << "edge-falling"
    << "edge-any"
    << "level-low"
    << "level-high";

PatternGenerator::PatternGenerator(struct iio_context *ctx, Filter *filt, QPushButton *runBtn, QWidget *parent) :
    QWidget(parent),
    ctx(ctx), itemsize(sizeof(uint16_t)),
    dev(iio_context_find_device(ctx, "m2k-logic-analyzer-rx")),
    menuOpened(true), fd(0),
    settings_group(new QButtonGroup(this)), menuRunButton(runBtn),
    ui(new Ui::PatternGenerator)


{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose, true);

    this->settings_group->setExclusive(true);
    this->no_channels = get_no_channels(dev);

    //sigrok and sigrokdecode initialisation
    std::shared_ptr<sigrok::Context> context;
    std::string open_file, open_file_format;
    context = sigrok::Context::create();

    // Initialise libsigrokdecode
    if (srd_init(nullptr) != SRD_OK) {
            qDebug() << "ERROR: libsigrokdecode init failed.";
    }
    // Load the protocol decoders
    srd_decoder_load_all();

    pv::DeviceManager device_manager(context);
    //pv::MainWindow* w = new pv::MainWindow(device_manager, "test.bin", "binary", parent);

    for(int i=0;i<65536;i++)
    {
        buffer[i] = i;
    }
    pv::MainWindow* w = new pv::MainWindow(device_manager, open_file, open_file_format, parent);
    /*w->action_view_zoom_fit()->setEnabled(true);
    w->action_view_zoom_fit()->setChecked(true);*/
    //shared_ptr<pv::devices::Device> pattern_generator_device = ;
    std::shared_ptr<sigrok::InputFormat> binary_format = w->get_format_from_string("binary");

    /*{
        std::make_pair("numchannels",g_variant_new("i",(16))),
        std::make_pair("samplerate",g_variant_new("i",(16)))

    };*/


    /*options.insert(numchannelsstring,g_variant_new("i",(16)));
    options.insert(sampleratestring,g_variant_new("x",G_GINT64_CONSTANT(1000000)));*/
    //gint32 numchannels = 16;
    options["numchannels"] = Glib::Variant<gint32>(g_variant_new_int32(16),true);//(Glib::VariantBase)(gint32(16));
    options["samplerate"] = Glib::Variant<guint64>(g_variant_new_uint64(1000000),true);//(Glib::VariantBase)(gint64(1000000));




    std::shared_ptr<pv::devices::BinaryBuffer> patern_generator_ptr( new pv::devices::BinaryBuffer(device_manager.context(),buffer,sizeof(buffer),binary_format,options));

    w->select_device(patern_generator_ptr);




   /* Gnuradio Blocks */
   // this->sink_streams_to_short = adiscope::streams_to_short::make(itemsize, no_channels);


    /* setup plot view */
    main_win = w;
    ui->horizontalLayout->removeWidget(ui->centralWidget);
    ui->horizontalLayout->insertWidget(0, static_cast<QWidget* >(main_win));

    /* setup toolbar */
    pv::toolbars::MainBar* main_bar = main_win->main_bar_;
    QPushButton *btnDecoder = new QPushButton();
    btnDecoder->setIcon(QIcon::fromTheme("add-decoder", QIcon(":/icons/add-decoder.svg")));
    btnDecoder->setMenu(main_win->menu_decoder_add());
    ui->gridLayout->addWidget(btnDecoder);
    ui->gridLayout->addWidget(static_cast<QWidget *>(main_bar));


   /* int ret = mkfifo(DATA_PIPE, 0666);
    if( ret != 0){
        printf("ERROR CREATING pipe");
    }*/

    connect(ui->btnRunStop, SIGNAL(toggled(bool)), this, SLOT(startStop(bool)));
    connect(runBtn, SIGNAL(toggled(bool)), ui->btnRunStop, SLOT(setChecked(bool)));
    connect(ui->btnRunStop, SIGNAL(toggled(bool)), runBtn, SLOT(setChecked(bool)));

    connect(ui->btnSettings, SIGNAL(pressed()),
            this, SLOT(toggleRightMenu()));
}

/*PatternGenerator::ZoomToFit()
{

}*/

PatternGenerator::~PatternGenerator()
{
    this->startStop(false);

//    delete[] ids;
#ifdef _WIN32
//    CloseHandle(pipe);
    system("pause");
#else
    printf("unlinking\n");
//    unlink(DATA_PIPE);
#endif

    delete ui;
    // Destroy libsigrokdecode
    srd_exit();
}

void PatternGenerator::startStop(bool start)
{

    main_win->run_stop();/*session_.start_capture([&](QString message) {
        qDebug("Capture failed", message); });*/
#if 0
    if (start)
    {
        std::thread thr1 = std::thread(
                &PatternGenerator::create_fifo, this);

        main_win->run_stop();

        if(thr1.joinable()) {
            thr1.join();
        }

        this->sink_fd_block = gr::blocks::file_descriptor_sink::make(itemsize, fd);

        manager = iio_manager::get_instance(ctx, "m2k-logic-analyzer-rx");
        ids = new iio_manager::port_id[no_channels];

    /* connect the manager to the sink block: streams to short */
        for (int i = 0; i < no_channels; i++)
        {
            ids[i] = manager->connect(sink_streams_to_short, i, i, false); //add buffersize as parameter
        }

    /* connect the streams to short block to the file descriptor block */
        manager->connect(sink_streams_to_short, 0, sink_fd_block, 0);


        for ( int i = 0; i < no_channels; i++)
        {
            manager->start(ids[i]);
        }

        ui->btnRunStop->setText("Stop");
    }
    else
    {
        if (manager->started()){
            main_win->run_stop();
            manager->stop_all();
            manager->wait();
            manager->disconnect_all();
            manager.reset();
        }
        ui->btnRunStop->setText("Run");
    }
#endif

}

unsigned int PatternGenerator::get_no_channels(struct iio_device *dev)
{
    unsigned int nb = 0;

    for (unsigned int i = 0; i < iio_device_get_channels_count(dev); i++) {
        struct iio_channel *chn = iio_device_get_channel(dev, i);

        if (!iio_channel_is_output(chn) &&
                iio_channel_is_scan_element(chn))
            nb++;
    }
    qDebug("PatternGenerator number of channels - %d",nb);
    return nb;
}

void PatternGenerator::create_fifo() {

#ifdef _WIN32
    LPTSTR name = TEXT( "\\\\.\\pipe\\myfifo");
    HANDLE pipe = CreateNamedPipe(
            name, // name of the pipe
            PIPE_ACCESS_OUTBOUND, // 1-way pipe -- send only
            PIPE_TYPE_BYTE, // send data as a byte stream
            1, // only allow 1 instance of this pipe
            0, // no outbound buffer
            0, // no inbound buffer
            0, // use default wait time
            NULL // use default security attributes
            );

    fd = _open_osfhandle((long)pipe, O_WRONLY);

    printf("File descriptor = %d\n",fd);

    if (pipe == NULL || pipe == INVALID_HANDLE_VALUE) {
        printf("Failed to create outbound pipe instance.");
        // look up error code here using GetLastError()
        system("pause");
    }

    BOOL result = ConnectNamedPipe(pipe, NULL);
        if (!result) {
            printf("Failed to make connection on named pipe.");
            // look up error code here using GetLastError()
            CloseHandle(pipe); // close the pipe
            system("pause");
        }
#else

    fd = open(DATA_PIPE, O_WRONLY);
    printf("fd: %d\n", fd);
#endif
}



void PatternGenerator::toggleRightMenu(QPushButton *btn)
{

    bool open = !menuOpened;
    ui->rightWidget->toggleMenu(open);
    this->menuOpened = open;
}

void PatternGenerator::toggleRightMenu()
{
    toggleRightMenu(static_cast<QPushButton *>(QObject::sender()));
}

} /* namespace adiscope */

