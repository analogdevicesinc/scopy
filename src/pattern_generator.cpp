#include "ui_pattern_generator.h"

#include "pattern_generator.hpp"
#include <iio.h>
#include <QDebug>
#include <QTimer>

///* pulseview and sigrok */
#include "pulseview/pv/mainwindow.hpp"
#include "pulseview/pv/devices/binarybuffer.hpp"
#include "pulseview/pv/devicemanager.hpp"
#include "pulseview/pv/toolbars/mainbar.hpp"
#include "libsigrokcxx/libsigrokcxx.hpp"
#include "libsigrokdecode/libsigrokdecode.h"

#include "boost/thread.hpp"
#include <libserialport.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <vector>
#include <string.h>

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
    ctx(ctx),
    dev(iio_context_find_device(ctx, "m2k-logic-analyzer-tx")),
    channel_manager_dev(iio_context_find_device(ctx, "m2k-logic-analyzer")),
    menuOpened(true),
    settings_group(new QButtonGroup(this)), menuRunButton(runBtn),
    ui(new Ui::PatternGenerator),
    txbuf(0), sample_rate(100000), channel_enable_mask(0xffff),
    buffer_created(0)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose, true);

    this->settings_group->setExclusive(true);
    this->no_channels = iio_device_get_channels_count(channel_manager_dev);

    //sigrok and sigrokdecode initialisation
    context = sigrok::Context::create();
    pv::DeviceManager device_manager(context);


    // Initialise libsigrokdecode
    /*if (srd_init(nullptr) != SRD_OK) {
            qDebug() << "ERROR: libsigrokdecode init failed.";
    }
    // Load the protocol decoders
    srd_decoder_load_all();*/



    std::string open_file, open_file_format;
    pv::MainWindow* w = new pv::MainWindow(device_manager, "pattern_generator", open_file_format, parent);
    binary_format = w->get_format_from_string("binary");



    /* setup plot view */
    main_win = w;
    ui->horizontalLayout->removeWidget(ui->centralWidget);
    ui->horizontalLayout->insertWidget(0, static_cast<QWidget* >(main_win));
    ui->UARTFormat->setText("115200/8n1");
    ui->sampleRate->setText("100000");
    ui->holdOff->setText("10000");

    /* setup toolbar */
    pv::toolbars::MainBar* main_bar = main_win->main_bar_;
    QPushButton *btnDecoder = new QPushButton();
    btnDecoder->setIcon(QIcon::fromTheme("add-decoder", QIcon(":/icons/add-decoder.svg")));
    btnDecoder->setMenu(main_win->menu_decoder_add());
    ui->gridLayout->addWidget(btnDecoder);
    ui->gridLayout->addWidget(static_cast<QWidget *>(main_bar));


    connect(ui->btnRunStop, SIGNAL(toggled(bool)), this, SLOT(startStop(bool)));
    connect(runBtn, SIGNAL(toggled(bool)), ui->btnRunStop, SLOT(setChecked(bool)));
    connect(ui->btnRunStop, SIGNAL(toggled(bool)), runBtn, SLOT(setChecked(bool)));

    connect(ui->btnSingleRun, SIGNAL(pressed()),this,SLOT(singleRun()));
    connect(ui->btnSettings, SIGNAL(pressed()),
            this, SLOT(toggleRightMenu()));
}


PatternGenerator::~PatternGenerator()
{
    stopPatternGeneration();

    delete ui;
    // Destroy libsigrokdecode
    srd_exit();
}

void PatternGenerator::createBinaryCounter(int maxCount, uint64_t sample_rate_)
{
    if(buffer)
        delete buffer;

    number_of_samples = maxCount;
    buffersize = number_of_samples*sizeof(short);
    buffer = new short[number_of_samples];

    for(int i=0;i<maxCount;i++)
    {
        buffer[i] = i;
    }


    sample_rate = sample_rate_;
    createBinaryBuffer();
    dataChanged();
    main_win->action_view_zoom_fit()->trigger();
}


// https://github.com/analogdevicesinc/libiio/blob/master/serial.c#L426

int PatternGenerator::parseParamsUart(const char *params,
                                      unsigned int *baud_rate, unsigned int *bits,
                                      enum sp_parity *parity, unsigned int *stop_bits)
{
    char *end;

    *baud_rate = strtoul(params, &end, 10);
    if (params == end)
        return -EINVAL;

    auto uart_format = strchr(params,'/');
    if(uart_format == NULL) /* / not found, use default settings*/
    {
        *bits = 8;
        *parity = SP_PARITY_NONE;
        *stop_bits = 1;
        return 0;
    }

    uart_format++;

    *bits = strtoul(uart_format, &end, 10);
    if (params == end)
        return -EINVAL;

    char lowercase_parity = tolower(*end);
    switch (lowercase_parity) {
    case 'n':
        *parity = SP_PARITY_NONE;
        break;
    case 'o':
        *parity = SP_PARITY_ODD;
        break;
    case 'e':
        *parity = SP_PARITY_EVEN;
        break;
    case 'm':
        *parity = SP_PARITY_MARK;
        break;
    case 's':
        *parity = SP_PARITY_SPACE;
        break;
    default:
        return -EINVAL;
    }
    end++;
    uart_format = end;
    *stop_bits = strtoul(uart_format, &end, 10);
    if (params == end)
        return -EINVAL;

}

uint16_t PatternGenerator::encapsulateUartFrame(char chr, uint16_t *bits_per_frame, uint16_t data_bits_per_frame, sp_parity parity, uint16_t stop_bits, bool msb_first)
{

    uint16_t ret = 0xffff;
    bool parity_bit_available;
    uint16_t parity_bit_value = 1;
    auto chr_to_test = chr;

    switch(parity)
    {
    case SP_PARITY_NONE:
        parity_bit_available = false;
        break;
    case SP_PARITY_ODD:
        parity_bit_value = 1;
        parity_bit_available = true;
        for(auto i=0;i<data_bits_per_frame;i++)
        {
            parity_bit_value = parity_bit_value ^ (chr_to_test & 0x01);
            chr_to_test = chr_to_test>>1;
        }
        break;
    case SP_PARITY_EVEN:
        parity_bit_value = 0;
        parity_bit_available = true;
        for(auto i=0;i<data_bits_per_frame;i++)
        {
            parity_bit_value = parity_bit_value ^ (chr_to_test & 0x01);
            chr_to_test = chr_to_test>>1;
        }
        break;
    case SP_PARITY_MARK:
        parity_bit_available = true;
        parity_bit_value = 1;
        break;
    case SP_PARITY_SPACE:
        parity_bit_available = true;
        parity_bit_value = 0;
        break;
    }



    if(!msb_first)
    {
        ret = chr;
        ret = ret << 1; // start bit
        uint16_t stop_bit_values;

        /*        if(parity_bit_available)
        {
            stop_bit_values = ((1<<stop_bits+1)-1) & parity_bit_value; // parity bit value is cleared
        }
        else
            stop_bit_values = ((1<<stop_bits)-1); // if parity bit not availabe, stop bits will not be incremented
*/
        stop_bit_values = ((1<<(stop_bits+parity_bit_available))-1) & ((parity_bit_value) ? (0xffff) : (0xfffe)); // todo: Simplify this
        ret = ret | stop_bit_values << (data_bits_per_frame+1);


    }
    else
    {
        ret = (~(1<<data_bits_per_frame)); // start bit
        ret = ret & chr;
        if(parity_bit_available)
        {
            ret = (ret << 1) | parity_bit_value;
        }
        for(auto i=0;i<stop_bits;i++)
            ret = (ret << 1) | 0x01;
    }

    (*bits_per_frame) = data_bits_per_frame + 1 + stop_bits + parity_bit_available;

    return ret;

}

void PatternGenerator::createUart(const char *str, uint16_t str_length, const char *params, uint64_t sample_rate_, uint32_t holdoff_time, uint16_t channel, bool msb_first)
{
    if(buffer)
        delete buffer;

    sp_parity parity = SP_PARITY_NONE;
    unsigned int stop_bits = 1;
    unsigned int baud_rate = 9600;
    unsigned data_bits_per_frame = 8;
    parseParamsUart(params,&baud_rate,&data_bits_per_frame,&parity,&stop_bits);


    uint16_t number_of_frames = str_length;
    uint32_t samples_per_bit = sample_rate/baud_rate;

    uint16_t bits_per_frame;
    encapsulateUartFrame(*str, &bits_per_frame,data_bits_per_frame,parity,stop_bits, msb_first);
    uint32_t samples_per_frame = samples_per_bit * bits_per_frame;



    uint32_t BEFORE_PADDING = holdoff_time/2;
    uint32_t AFTER_PADDING = holdoff_time/2;

    number_of_samples = samples_per_frame*number_of_frames  + BEFORE_PADDING + AFTER_PADDING;

    buffer = new short[number_of_samples]; // no need to recreate buffer
    buffersize = (number_of_samples)*sizeof(short);
    memset(buffer, 0xffff, (number_of_samples)*sizeof(short));

    short *ptr = buffer;
    int i;
    for(i=0;i<BEFORE_PADDING;i++,ptr++)
    {
        *ptr = 0x01;
    }

    for(i=0;i<str_length;i++,str++)
    {
        auto frame_to_send = encapsulateUartFrame(*str, &bits_per_frame, data_bits_per_frame,parity, stop_bits, msb_first);
        for(auto j=0;j<bits_per_frame;j++)
        {
            short bit_to_send;
            if(!msb_first)
            {
                bit_to_send = (frame_to_send & 0x01 ) << channel;
                frame_to_send = frame_to_send >> 1;
            }
            else
            {

                bit_to_send = ((frame_to_send & (1<<(bits_per_frame-1))) ? 1 : 0 )<< channel; // set bit here
                frame_to_send = frame_to_send << 1;
            }
            for(auto k=0;k<samples_per_bit;k++,ptr++) {
                *ptr =  bit_to_send;// set bit here
            }

        }
    }

    for(i=0;i<AFTER_PADDING;i++,ptr++)
    {
        *ptr = 0x01;
    }

    createBinaryBuffer();
    dataChanged();
    main_win->action_view_zoom_fit()->trigger();

}


void PatternGenerator::createBinaryBuffer()
{
    options["numchannels"] = Glib::Variant<gint32>(g_variant_new_int32(no_channels),true);//(Glib::VariantBase)(gint32(16));
    options["samplerate"] = Glib::Variant<guint64>(g_variant_new_uint64(sample_rate),true);//(Glib::VariantBase)(gint64(1000000));
    std::shared_ptr<pv::devices::BinaryBuffer> patern_generator_ptr( new pv::devices::BinaryBuffer(context,buffer,&buffersize,binary_format,options));

    main_win->select_device(patern_generator_ptr);
}


void PatternGenerator::dataChanged()
{
    main_win->run_stop();
}


bool PatternGenerator::startPatternGeneration(bool cyclic)
{

    /* Enable Tx channels*/
    char temp_buffer[12];

    if(!channel_manager_dev || !dev)
    {
        qDebug("Devices not found");
        return false;

    }
    qDebug("Setting channel direction");
    for (int j = 0; j < no_channels; j++) {
        if(channel_enable_mask & (1<<j)) {
            auto ch = iio_device_get_channel(channel_manager_dev, j);
            iio_channel_attr_write(ch, "direction", "out");
        }
    }

    qDebug("Setting sample rate");
    /* Set sample rate   */

    iio_device_attr_write(dev, "sampling_frequency", std::to_string(sample_rate).c_str());
    qDebug("Enabling channels");
    for (int j = 0; j < no_channels; j++) {
        auto ch = iio_device_get_channel(dev, j);
        iio_channel_enable(ch);
    }


    /* Create buffer     */
    qDebug("Creating buffer");
    txbuf = iio_device_create_buffer(dev, number_of_samples, cyclic);
    if(!txbuf)
    {
        qDebug("Could not create buffer - errno: %d - %s", errno, strerror(errno));
        return false;
    }
    buffer_created = true;
    short *p_dat;
    ptrdiff_t p_inc;

    int i = 0;
    for (p_dat = (short*)iio_buffer_start(txbuf); (p_dat < iio_buffer_end(txbuf)); (uint16_t*)p_dat++,i++)
    {
        *p_dat = buffer[i];
    }

    /* Push buffer       */
    auto number_of_bytes = iio_buffer_push(txbuf);
    qDebug("\nPushed %ld bytes to devices\r\n",number_of_bytes);
    return true;
}

void PatternGenerator::stopPatternGeneration()
{

    /* Destroy buffer */
    if(buffer_created == true) {
        iio_buffer_destroy(txbuf);
        buffer_created = false;
    }
    /* Reset Tx Channls*/
    auto nb_channels = iio_device_get_channels_count(channel_manager_dev);
    for (int j = 0; j < nb_channels; j++) {
        auto ch = iio_device_get_channel(channel_manager_dev, j);
        iio_channel_attr_write(ch, "direction", "in");
    }
}

void PatternGenerator::startStop(bool start)
{

    main_win->action_view_zoom_fit()->trigger();
    if (start)
    {
        if(startPatternGeneration(true))
            ui->btnRunStop->setText("Stop");
        else
            qDebug("Pattern generation failed");

    }
    else
    {
        stopPatternGeneration();
        ui->btnRunStop->setText("Run");
    }
}

void PatternGenerator::singleRun()
{
    main_win->action_view_zoom_fit()->trigger();
    stopPatternGeneration();
    if(startPatternGeneration(false))
    {
        uint32_t time_until_buffer_destroy = 1000 + (uint32_t)((number_of_samples/((float)sample_rate))*1000.0);
        qDebug("Time until buffer destroy %d", time_until_buffer_destroy);
        QTimer::singleShot(time_until_buffer_destroy, this, SLOT(singleRunStop()));
        qDebug("Pattern generation single started");
        ui->btnSingleRun->setChecked(false);
    }
    else
    {
        qDebug("Pattern generation failed");
        ui->btnSingleRun->setChecked(true);
    }

}

void PatternGenerator::singleRunStop()
{
    qDebug("Pattern Generation stopped ");
    stopPatternGeneration();
    ui->btnSingleRun->setChecked(false);
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


void adiscope::PatternGenerator::on_BinaryCounter_clicked()
{
    QString maxCount =  ui->generation_data->text();
    // filter input
    sample_rate = ui->sampleRate->text().toULongLong();
    createBinaryCounter(maxCount.toInt(),1000000);
}

void adiscope::PatternGenerator::on_UART_clicked()
{
    QString val = ui->generation_data->text();
    QString format = ui->UARTFormat->text();
    sample_rate = ui->sampleRate->text().toULongLong();
    QString holdoff = ui->holdOff->text();
    createUart(val.toStdString().c_str(),val.length(),format.toStdString().c_str(),sample_rate, holdoff.toULong(),0 , false);
}
