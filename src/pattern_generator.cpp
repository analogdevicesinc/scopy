#include <iio.h>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QFile>
#include <QTimer>
#include <QtQml/QJSEngine>
#include <QtQml/QQmlEngine>
#include <QDirIterator>
#include <QPushButton>
#include <QHBoxLayout>

///* pulseview and sigrok */
#include <boost/math/common_factor.hpp>
#include "pulseview/pv/mainwindow.hpp"
#include "pulseview/pv/devices/binarybuffer.hpp"
#include "pulseview/pv/devicemanager.hpp"
#include "pulseview/pv/toolbars/mainbar.hpp"
#include "libsigrokcxx/libsigrokcxx.hpp"
#include "libsigrokdecode/libsigrokdecode.h"

#include "boost/thread.hpp"
#include <libserialport.h>

#include "pattern_generator.hpp"

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


/*QStringList PatternGenerator::digital_trigger_conditions = QStringList()
        << "edge-rising"
        << "edge-falling"
        << "edge-any"
        << "level-low"
        << "level-high";

*/

QStringList PatternGenerator::possibleSampleRates = QStringList()
        << "80000000"
        << "40000000"   << "20000000"  << "10000000"
        << "5000000"    << "2000000"   << "1000000"
        << "500000"     << "200000"    << "100000"
        << "50000"      << "20000"     << "10000"
        << "5000"       << "2000"      << "1000"
        << "500"        << "200"       << "100"
        << "50"         << "20"        << "10"
        << "5"          << "2"         << "1";

PatternGenerator::PatternGenerator(struct iio_context *ctx, Filter *filt, QPushButton *runBtn, QWidget *parent) :
    QWidget(parent),
    ctx(ctx),
    dev(iio_context_find_device(ctx, "m2k-logic-analyzer-tx")),
    channel_manager_dev(iio_context_find_device(ctx, "m2k-logic-analyzer")),
    menuOpened(true),
    settings_group(new QButtonGroup(this)), menuRunButton(runBtn),
    ui(new Ui::PatternGenerator),
    txbuf(0), sample_rate(100000), channel_enable_mask(0xffff),buffer(nullptr),
    buffer_created(0), currentUI(nullptr)
{
    // UI
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose, true);
    this->settings_group->setExclusive(true);

    // IIO
    this->no_channels = iio_device_get_channels_count(channel_manager_dev);

    //sigrok and sigrokdecode initialisation
    context = sigrok::Context::create();
    pv::DeviceManager device_manager(context);
    pv::MainWindow* w = new pv::MainWindow(device_manager, filt, "pattern_generator", "",parent);
    binary_format = w->get_format_from_string("binary");


    /* setup PV plot view */
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

    int i = 0;

    BinaryCounterPatternUI *bcpu = new BinaryCounterPatternUI(this);
    ClockPatternUI *cpu = new ClockPatternUI(this);
    UARTPatternUI *upu = new UARTPatternUI(this);
    LFSRPatternUI *ppu = new LFSRPatternUI(this);
    patterns.push_back(dynamic_cast<PatternUI*>(bcpu));
    patterns.push_back(dynamic_cast<PatternUI*>(cpu));
    patterns.push_back(dynamic_cast<PatternUI*>(upu));
    patterns.push_back(dynamic_cast<PatternUI*>(ppu));

    for(auto &var : patterns)
    {
        ui->scriptCombo->addItem(QString::fromStdString(var->get_name()));
        ui->scriptCombo->setItemData(i,QString::fromStdString(var->get_description()),Qt::ToolTipRole);
        i++;
    }

    QString searchPattern = "generator.json";
    QDirIterator it("patterngenerator", QStringList() << searchPattern, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        QFile file;
        QString filename = it.next();

        file.setFileName(filename);
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        QJsonDocument d = QJsonDocument::fromJson(file.readAll());
        file.close();        
        QJsonObject obj(d.object());

        filename.chop(searchPattern.length());
        obj.insert("filepath",filename);

        if(obj["enabled"] == true)
        {
            ui->scriptCombo->addItem(obj["name"].toString());
            if(obj.contains("description"))
                ui->scriptCombo->setItemData(i,obj["description"].toString(),Qt::ToolTipRole);
            i++;
            JSPatternUI *jspu = new JSPatternUI(obj, this);
            patterns.push_back(dynamic_cast<PatternUI*>(jspu));
        }
        qDebug()<<obj;
    }

    ui->sampleRateCombo->addItems(possibleSampleRates);
    for(i=0;i<possibleSampleRates.length()-1;i++)
        if(sample_rate <= possibleSampleRates[i].toInt() && sample_rate > possibleSampleRates[i+1].toInt())
            break;
    ui->sampleRateCombo->setCurrentIndex(i);
    sampleRateValidator = new QIntValidator(1,80000000,this);
    ui->sampleRateCombo->setValidator(sampleRateValidator);

    ui->ChannelEnableMask->setText("0xffff");
    ui->ChannelsToGenerate->setText("0x0001");
    ui->numberOfSamples->setText("2048");


    connect(ui->btnRunStop, SIGNAL(toggled(bool)), this, SLOT(startStop(bool)));
    connect(runBtn, SIGNAL(toggled(bool)), ui->btnRunStop, SLOT(setChecked(bool)));
    connect(ui->btnRunStop, SIGNAL(toggled(bool)), runBtn, SLOT(setChecked(bool)));

    connect(ui->btnSingleRun, SIGNAL(pressed()), this, SLOT(singleRun()));
    connect(ui->btnSettings , SIGNAL(pressed()), this, SLOT(toggleRightMenu()));
}

PatternGenerator::~PatternGenerator()
{
    stopPatternGeneration();

    for(auto var : patterns)
    {
        delete var;
    }
    delete ui;
    delete sampleRateValidator;
    // Destroy libsigrokdecode
    srd_exit();
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
 //   ui->rightWidget->toggleMenu(open); TEMP
    this->menuOpened = open;
}

void PatternGenerator::toggleRightMenu()
{
    toggleRightMenu(static_cast<QPushButton *>(QObject::sender()));
}

void adiscope::PatternGenerator::on_sampleRateCombo_activated(const QString &arg1)
{
    sample_rate = arg1.toInt();
}

void adiscope::PatternGenerator::on_generatePattern_clicked()
{

    bool ok;
    channel_group = ui->ChannelsToGenerate->text().toUShort(&ok,16);
    if(!ok) {qDebug()<< "could not convert to hex";return;}
    number_of_samples = ui->numberOfSamples->text().toLong();
    last_sample = ui->lastSample->text().toLong();// number_of_samples;//-100;
    start_sample = ui->startingSample->text().toLong();
    if(buffersize != number_of_samples * 2 && buffer != nullptr){
        delete buffer;
        buffer=nullptr;
    }
    if(!buffer)
    {
        buffer = new short[number_of_samples];
        buffersize = number_of_samples * sizeof(short);
        memset(buffer, 0x0000, (number_of_samples)*sizeof(short));
    }

    PatternUI *current;
    current = patterns[ui->scriptCombo->currentIndex()];
    current->parse_ui();
    current->set_number_of_channels(get_nr_of_channels());
    current->set_number_of_samples(get_nr_of_samples());
    current->set_sample_rate(sample_rate);

    qDebug()<<"pregenerate status: "<<current->pre_generate();
    qDebug()<<"minimum sampling frequency"<<current->get_min_sampling_freq(); // least common multiplier
    current->set_sample_rate(current->get_min_sampling_freq()); // TEMP
    qDebug()<<"minimum number of samples"<<current->get_required_nr_of_samples(); // if not periodic, verify minimum, else least common multiplier with least common freq
    current->set_sample_rate(sample_rate);


    if(current->generate_pattern() != 0) {qDebug()<<"Pattern Generation failed";return;} //ERROR TEMPORARY
    /*if(current->number_of_samples>(last_sample-start_sample)) {qDebug()<<"Warning! not enough buffer space to generate whole pattern";}
    else {last_sample = current->number_of_samples+start_sample;}*/
    commitBuffer(current->get_buffer());
    createBinaryBuffer();
    current->delete_buffer();
    dataChanged();
    main_win->action_view_zoom_fit()->trigger();

}

uint32_t adiscope::PatternGenerator::get_nr_of_samples()
{
    return last_sample - start_sample;
}

uint32_t adiscope::PatternGenerator::get_nr_of_channels()
{
    int i=0;
    int channel_mask_temp = channel_group;
    while(channel_mask_temp)
    {
        i=i+(channel_mask_temp & 0x01);
        channel_mask_temp>>=1;
    }
    return i;
}

uint32_t adiscope::PatternGenerator::get_sample_rate()
{
    return sample_rate;
}

short PatternGenerator::convert_jsbuffer(uint8_t *mapping, uint32_t val)
{
    short ret=0;
    int i=0;
    while(val)
    {
        if(val&0x01)
        {
            ret = ret | (1<<mapping[i]);
        }
        i++;
        val>>=1;
    }
    return ret;
}

void adiscope::PatternGenerator::commitBuffer(short *bufferPtr)
{
    uint8_t channel_mapping[no_channels];

    int i=0,j=0;
    auto channel_enable_mask_temp = channel_group;
    auto buffer_channel_mask = (1<<get_nr_of_channels())-1;
    while(channel_enable_mask_temp)
    {
        if(channel_enable_mask_temp & 0x01) {
            channel_mapping[j] = i;
            j++;
        }
        channel_enable_mask_temp>>=1;
        i++;
    }

    for(auto i=start_sample;i<last_sample;i++)
    {
        auto val = (bufferPtr[i-start_sample] & buffer_channel_mask);
        buffer[i] = (buffer[i] & ~channel_group) | convert_jsbuffer(channel_mapping, val);
    }
}

void adiscope::PatternGenerator::on_clearButton_clicked()
{
    memset(buffer, 0x0000, (number_of_samples)*sizeof(short));
    createBinaryBuffer();
    dataChanged();
    main_win->action_view_zoom_fit()->trigger();
}

void adiscope::PatternGenerator::on_generateUI_clicked()
{
    if(currentUI!=nullptr)
    {
        currentUI->deinit();
        currentUI->setVisible(false);
        currentUI->destroy_ui(); 
    }
    currentUI = patterns[ui->scriptCombo->currentIndex()];
    currentUI->build_ui(ui->rightWidgetPage2);
    currentUI->init();
    currentUI->post_load_ui();
    currentUI->setVisible(true);
}

/////////////////////////////////////////////////
//////// PATTERNS //////////////////////////////
////////////////////////////////////////////////

JSConsole::JSConsole(QObject *parent) :
    QObject(parent)
{
}

void JSConsole::log(QString msg)
{
    qDebug() << "jsConsole: "<< msg;
}

Pattern::Pattern()
{
    qDebug()<<"PatternCreated";
    buffer = nullptr;
}

Pattern::~Pattern()
{
    qDebug()<<"PatternDestroyed";
    delete_buffer();
}

string Pattern::get_name()
{
    return name;
}

void Pattern::set_name(string name_)
{
    name = name_;
}

string Pattern::get_description()
{
    return description;
}

void Pattern::set_description(string description_)
{
    description = description_;
}

void Pattern::set_sample_rate(uint32_t sample_rate_)
{
    sample_rate = sample_rate_;
}

void Pattern::set_number_of_samples(uint32_t number_of_samples_)
{
    number_of_samples = number_of_samples_;
}
void Pattern::set_number_of_channels(uint16_t number_of_channels_)
{
    number_of_channels = number_of_channels_;
}

void Pattern::init()
{

}

void Pattern::deinit()
{

}

bool Pattern::is_periodic()
{
    return periodic;
}

void Pattern::set_periodic(bool periodic_)
{
    periodic=periodic_;
}

short* Pattern::get_buffer()
{
    return buffer;
}

void Pattern::delete_buffer()
{
    if(buffer)
        delete buffer;
    buffer=nullptr;
}

uint8_t Pattern::pre_generate()
{
    return 0;
}

uint32_t Pattern::get_min_sampling_freq()
{
    return 1; // minimum 1 hertz if not specified otherwise
}

uint32_t Pattern::get_required_nr_of_samples()
{
    return 0; // 0 samples required
}

PatternUI::PatternUI(QWidget *parent) : QWidget(parent){
    qDebug()<<"PatternUICreated";
}
PatternUI::~PatternUI(){
    qDebug()<<"PatternUIDestroyed";
}
void PatternUI::build_ui(QWidget *parent){}
void PatternUI::post_load_ui(){}
void PatternUI::parse_ui(){}
void PatternUI::destroy_ui(){}

uint32_t BinaryCounterPattern::get_min_sampling_freq()
{
    return frequency;
}

uint32_t BinaryCounterPattern::get_required_nr_of_samples()
{
    // greatest common divider duty cycle and 1000;0;
    return ((float)sample_rate/(float)frequency) * (1<<number_of_channels);
}


uint32_t BinaryCounterPattern::get_frequency() const
{
    return frequency;
}

void BinaryCounterPattern::set_frequency(const uint32_t &value)
{
    frequency = value;
}

uint16_t BinaryCounterPattern::get_start_value() const
{
    return start_value;
}

void BinaryCounterPattern::set_start_value(const uint16_t &value)
{
    start_value = value;
}

uint16_t BinaryCounterPattern::get_end_value() const
{
    return end_value;
}

void BinaryCounterPattern::set_end_value(const uint16_t &value)
{
    end_value = value;
}

uint16_t BinaryCounterPattern::get_increment() const
{
    return increment;
}

void BinaryCounterPattern::set_increment(const uint16_t &value)
{
    increment = value;
}

BinaryCounterPattern::BinaryCounterPattern()
{
    qDebug()<<"BinaryCounterPatternCreated";
    set_name("BinaryCounter");
    set_description("BinaryCounterDescription");
    set_periodic(true);
}

uint8_t BinaryCounterPattern::generate_pattern()
{
    delete_buffer();
    buffer = new short[number_of_samples];
    auto samples_per_count = ((float)sample_rate/(float)frequency);
    auto i=start_value;
    auto j=0;
    while(j<number_of_samples)
    {
        for(auto k=0;k<samples_per_count;k++,j++)
        {
            if(j>=number_of_samples)break;
            buffer[j] = i;
        }
        if(i<end_value)
        {
            i=i+increment;
        }
        else
        {
            i=start_value;
        }
    }
    return 0;


}

BinaryCounterPatternUI::BinaryCounterPatternUI(QWidget *parent) : PatternUI(parent){
    qDebug()<<"BinaryCounterPatternUI Created";
    ui = new Ui::BinaryCounterPatternUI();
    ui->setupUi(this);
    setVisible(false);
}
BinaryCounterPatternUI::~BinaryCounterPatternUI(){
              qDebug()<<"BinaryCounterPatternUI Destroyed";
}
void BinaryCounterPatternUI::build_ui(QWidget *parent){
    parent_ = parent;
    parent->layout()->addWidget(this);

}
void BinaryCounterPatternUI::destroy_ui(){
    parent_->layout()->removeWidget(this);
 //   delete ui;
}

UARTPattern::UARTPattern()
{
    qDebug()<<"UARTPattern Created";
    parity = SP_PARITY_NONE;
    stop_bits = 1;
    baud_rate = 9600;
    data_bits = 8;
    msb_first=false;
    set_periodic(false);
    set_name("UART");
    set_description("UARTDescription");

}

void UARTPattern::set_string(std::string str_)
{
    str = str_;
}

int UARTPattern::set_params(std::string params_)
{
    // https://github.com/analogdevicesinc/libiio/blob/master/serial.c#L426
    const char *params = params_.c_str();
    char *end;

    baud_rate = strtoul(params, &end, 10);
    if (params == end)
        return -EINVAL;

    auto uart_format = strchr(params,'/');
    if(uart_format == NULL) /* / not found, use default settings*/
    {
        data_bits = 8;
        parity = SP_PARITY_NONE;
        stop_bits = 1;
        return 0;
    }

    uart_format++;

    data_bits = strtoul(uart_format, &end, 10);
    if (params == end)
        return -EINVAL;

    char lowercase_parity = tolower(*end);
    switch (lowercase_parity) {
    case 'n':
        parity = SP_PARITY_NONE;
        break;
    case 'o':
        parity = SP_PARITY_ODD;
        break;
    case 'e':
        parity = SP_PARITY_EVEN;
        break;
    case 'm':
        parity = SP_PARITY_MARK;
        break;
    case 's':
        parity = SP_PARITY_SPACE;
        break;
    default:
        return -EINVAL;
    }
    end++;
    uart_format = end;
    stop_bits = strtoul(uart_format, &end, 10);
    if (params == end)
        return -EINVAL;

    return 0;
}


void UARTPattern::set_msb_first(bool msb_first_)
{
    msb_first = msb_first_;
}

uint16_t UARTPattern::encapsulateUartFrame(char chr, uint16_t *bits_per_frame)
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
        for(auto i=0;i<data_bits;i++)
        {
            parity_bit_value = parity_bit_value ^ (chr_to_test & 0x01);
            chr_to_test = chr_to_test>>1;
        }
        break;
    case SP_PARITY_EVEN:
        parity_bit_value = 0;
        parity_bit_available = true;
        for(auto i=0;i<data_bits;i++)
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

/*      if(parity_bit_available)
        {
            stop_bit_values = ((1<<stop_bits+1)-1) & parity_bit_value; // parity bit value is cleared
        }
        else
            stop_bit_values = ((1<<stop_bits)-1); // if parity bit not availabe, stop bits will not be incremented
*/
        stop_bit_values = ((1<<(stop_bits+parity_bit_available))-1) & ((parity_bit_value) ? (0xffff) : (0xfffe)); // todo: Simplify this
        ret = ret | stop_bit_values << (data_bits+1);


    }
    else
    {
        ret = (~(1<<data_bits)); // start bit
        ret = ret & chr;
        if(parity_bit_available)
        {
            ret = (ret << 1) | parity_bit_value;
        }
        for(auto i=0;i<stop_bits;i++)
            ret = (ret << 1) | 0x01;
    }

    (*bits_per_frame) = data_bits + 1 + stop_bits + parity_bit_available;

    return ret;

}

uint32_t UARTPattern::get_min_sampling_freq()
{
    return baud_rate;
}

uint32_t UARTPattern::get_required_nr_of_samples()
{
    uint16_t number_of_frames = str.length();
    uint32_t samples_per_bit = sample_rate/baud_rate;
    uint16_t bits_per_frame;
    encapsulateUartFrame(*(str.c_str()), &bits_per_frame);
    uint32_t samples_per_frame = samples_per_bit * bits_per_frame;
    number_of_samples = samples_per_frame*(number_of_frames + 1/* padding */);
    return number_of_samples;
}

uint8_t UARTPattern::generate_pattern()
{
    delete_buffer();


    uint16_t number_of_frames = str.length();
    uint32_t samples_per_bit = sample_rate/baud_rate;
    qDebug()<< "samples_per_bit - "<<(float)sample_rate/(float)baud_rate;
    uint16_t bits_per_frame;
    encapsulateUartFrame(*(str.c_str()), &bits_per_frame);
    uint32_t samples_per_frame = samples_per_bit * bits_per_frame;
    number_of_samples = samples_per_frame*(number_of_frames + 1/* padding */);

    buffer = new short[number_of_samples]; // no need to recreate buffer
    auto buffersize = (number_of_samples)*sizeof(short);
    memset(buffer, 0xffff, (number_of_samples)*sizeof(short));

    short *buf_ptr = buffer;
    const char *str_ptr = str.c_str();
    int i;

    for(i=0;i<samples_per_frame/2;i++) // pad with half a frame
    {
        *buf_ptr = 1;
        buf_ptr++;
    }
    for(i=0;i<str.length();i++,str_ptr++)
    {
        auto frame_to_send = encapsulateUartFrame(*str_ptr, &bits_per_frame);
        for(auto j=0;j<bits_per_frame;j++)
        {
            short bit_to_send;
            if(!msb_first)
            {
                bit_to_send = (frame_to_send & 0x01 );
                frame_to_send = frame_to_send >> 1;
            }
            else
            {
                bit_to_send = ((frame_to_send & (1<<(bits_per_frame-1))) ? 1 : 0 ); // set bit here
                frame_to_send = frame_to_send << 1;
            }
            for(auto k=0;k<samples_per_bit;k++,buf_ptr++) {
                *buf_ptr =  bit_to_send;// set bit here
            }
        }
    }
    for(i=0;i<samples_per_frame/2;i++) // pad with half a frame
    {
        *buf_ptr = 1;
        buf_ptr++;
    }
    return 0;
}

UARTPatternUI::UARTPatternUI(QWidget *parent) : PatternUI(parent)
{
    qDebug()<<"UARTPatternUI created";
    set_params("9600/8N1");
    set_string("hello World");

    ui = new Ui::UARTPatternUI();
    ui->setupUi(this);
    setVisible(false);
}

UARTPatternUI::~UARTPatternUI(){
    qDebug()<<"UARTPatternUI destroyed";
}
void UARTPatternUI::build_ui(QWidget *parent){
    parent_ = parent;
    parent->layout()->addWidget(this);
}
void UARTPatternUI::destroy_ui(){
    parent_->layout()->removeWidget(this);
//    delete ui;
}


void UARTPatternUI::parse_ui()
{
    ui->paramsOut->setText(ui->baudCombo->currentText() + "/8" +ui->parityCombo->currentText()[0] + ui->stopCombo->currentText());
    qDebug()<<ui->paramsOut->text();
    set_params(ui->paramsOut->text().toStdString());
    qDebug()<<ui->dataEdit->text();
    set_string(ui->dataEdit->text().toStdString());
}
/*
void adiscope::UARTPatternUI::on_setUARTParameters_clicked()
{

}*/

JSPattern::JSPattern(QJsonObject obj_) : obj(obj_){
    qDebug()<<"JSPattern created";
    console = new JSConsole();
    qEngine = nullptr;
}

void JSPattern::init()
{
    if(qEngine!=nullptr)
    {
        qEngine->collectGarbage();
        delete qEngine;
    }
    qEngine = new QJSEngine();
}

void JSPattern::deinit()
{
    if(qEngine!=nullptr)
    {
        qEngine->collectGarbage();
        delete qEngine;
        qEngine = nullptr;
    }
}

uint32_t JSPattern::get_min_sampling_freq()
{
    QJSValue result = qEngine->evaluate("get_min_sampling_freq()");
    if(result.isNumber()) return result.toUInt();
    else if(result.isString()) qDebug() << "Error - return value of get_min_sampling_freq() is a string - " << result.toString();
    else qDebug() << "Error - get_min_sampling_freq - "<< result.toString();
    return 0;
}

uint32_t JSPattern::get_required_nr_of_samples()
{
    QJSValue result = qEngine->evaluate("get_required_nr_of_samples()");
    if(result.isNumber()) return result.toUInt();
    else if(result.isString()) qDebug() << "Error - return value of get_required_nr_of_samples() is a string - " << result.toString();
    else qDebug() << "Error - get_required_nr_of_samples - "<< result.toString();
    return 0;
}

bool JSPattern::is_periodic()
{
    QJSValue result = qEngine->evaluate("is_periodic()");
    if(result.isBool()) return result.toBool();
    else if(result.isString()) qDebug() << "Error - return value of is_periodic() is a string - " << result.toString();
    else qDebug() << "Error - is_periodic - "<< result.toString();
    return 0;
}

uint8_t JSPattern::pre_generate()
{
    bool ok;
    QString fileName(obj["filepath"].toString() + obj["generate_script"].toString());
    qDebug()<<fileName;
    QFile scriptFile(fileName);
    scriptFile.open(QIODevice::ReadOnly);
    QTextStream stream(&scriptFile);
    QString contents = stream.readAll();
    scriptFile.close();

    qEngine->evaluate("function is_periodic(){ status_window.print(\"is_periodic() not found\")}");
    qEngine->evaluate("function get_required_nr_of_samples(){ status_window.print(\"get_required_nr_of_samples() not found\")}");
    qEngine->evaluate("function get_min_sampling_freq(){ status_window.print(\"get_min_sampling_freq() not found\")}");
    qEngine->evaluate("function generate(){ status_window.print(\"generate() not found\")}");

    // if file does not exist, stream will be empty
    handle_result(qEngine->evaluate(contents, fileName),"Eval generatescript");


    return 0;
}

bool JSPattern::handle_result(QJSValue result,QString str)
{
    if(result.isError())
    {
       qDebug()
                    << "Uncaught exception at line"
                    << result.property("lineNumber").toInt()
                    << ":" << result.toString();
        return -2;
    }
    else
        qDebug()<<str<<" - Success";

}

uint8_t JSPattern::generate_pattern()
{

    handle_result(qEngine->evaluate("generate()"),"Eval generate");
    commitBuffer(qEngine->evaluate("pg.buffer"),qEngine->evaluate("pg.buffersize"));
    return 0;
}

quint32 JSPattern::get_nr_of_samples()
{
    return number_of_samples;
}

quint32 JSPattern::get_nr_of_channels()
{
    return number_of_channels;
}

quint32 JSPattern::get_sample_rate()
{
    return sample_rate;
}

void JSPattern::JSErrorDialog(QString errorMessage)
{
    qDebug()<<"JSErrorDialog: "<<errorMessage;
}

void JSPattern::commitBuffer(QJSValue jsBufferValue, QJSValue jsBufferSize)
{
    if(!jsBufferValue.isArray()) {qDebug()<<"Not an array";return;}
    if(!jsBufferSize.isNumber()) {qDebug()<<"Not a valid size";return;}

    delete_buffer();
    buffer = new short[jsBufferSize.toInt()];

    for(auto i=0;i<jsBufferSize.toInt();i++)
    {
        if(!jsBufferValue.property(i).isError())
        {
            auto val = jsBufferValue.property(i).toInt();
            buffer[i] = val;
        }
        else
        {
            buffer[i] = 0;
        }
    }
}

JSPatternUI::JSPatternUI(QJsonObject obj_, QWidget *parent) : JSPattern(obj_), PatternUI(parent){
    qDebug()<<"JSPatternUI created";
    loader = nullptr;
    ui_form = nullptr;
    ui = new Ui::GenericJSPatternUI();
    ui->setupUi(this);
    setVisible(false);
    textedit = new JSPatternUIStatusWindow(ui->jsstatus);

}
JSPatternUI::~JSPatternUI(){
    qDebug()<<"JSPatternUI destroyed";

}
void JSPatternUI::build_ui(QWidget *parent)
{
    qDebug()<<"JSPatternUI built";
    parent_ = parent;
    parent->layout()->addWidget(this);
    QFile file(obj["filepath"].toString() + obj["ui_form"].toString());
    file.open(QFile::ReadOnly);
    if(file.exists() && file.isReadable()){
        loader = new QUiLoader;
        ui_form = loader->load(&file, ui->ui_form);
        file.close();
        form_name = ui_form->objectName();

    }
    else
    {
        qDebug() << "file does not exist";
    }


}
void JSPatternUI::destroy_ui()
{
    if(ui_form) delete ui_form;
    if(loader) delete loader;
    parent_->layout()->removeWidget(this);
    //delete ui_form;
 //   delete ui;
}

void JSPatternUI::find_all_children(QObject* parent, QJSValue property)
{

    if(parent->children().count() == 0){ return;}
    for(auto child: parent->children())
    {
        if(child->objectName()!="")
        {
            QJSValue jschild = qEngine->newQObject(child);
            property.setProperty(child->objectName(),jschild);
            QQmlEngine::setObjectOwnership(child, QQmlEngine::CppOwnership);

            find_all_children(child, property.property(child->objectName()));
        }
    }
}

void JSPatternUI::post_load_ui()
{
    QString fileName(obj["filepath"].toString() + obj["ui_script"].toString());
    qDebug()<<fileName;
    QFile scriptFile(fileName);
    scriptFile.open(QIODevice::ReadOnly);
    QTextStream stream(&scriptFile);
    QString contents = stream.readAll();
    scriptFile.close();

    JSPattern *pg = this;
    JSPatternUIStatusWindow *status_window = textedit;
    QJSValue pgObj =  qEngine->newQObject(pg);
    QJSValue consoleObj =  qEngine->newQObject(console);
    QJSValue statusTextObj =  qEngine->newQObject(status_window);
    QJSValue uiObj =  qEngine->newQObject(/*ui_form*/ui->ui_form->findChild<QWidget*>(form_name));

    qEngine->globalObject().setProperty("pg",pgObj);
    QQmlEngine::setObjectOwnership(/*(QObject*)*/pg, QQmlEngine::CppOwnership);
    qEngine->globalObject().setProperty("console", consoleObj);
    QQmlEngine::setObjectOwnership(/*(QObject*)*/console, QQmlEngine::CppOwnership);
    qEngine->globalObject().setProperty("ui", uiObj);    
    QQmlEngine::setObjectOwnership(/*(QObject*)*/ui_form, QQmlEngine::CppOwnership);

    qEngine->globalObject().setProperty("status_window", statusTextObj);
    QQmlEngine::setObjectOwnership(/*(QObject*)*/status_window, QQmlEngine::CppOwnership);

    qEngine->evaluate("ui_elements = [];");

    find_all_children(ui->ui_form->findChild<QObject*>(form_name), qEngine->globalObject().property("ui_elements"));

    qEngine->evaluate("pg.buffer = [];").toString();
    qEngine->evaluate("pg.buffersize = 0;").toString();


    qEngine->evaluate("function post_load_ui(){ status_window.print(\"post_load_ui() not found\")}");
    qEngine->evaluate("function parse_ui(){ status_window.print(\"parse_ui() not found\")}");
    handle_result(qEngine->evaluate(contents, fileName),"eval ui_script");
    handle_result(qEngine->evaluate("post_load_ui()"),"post_load_ui");
}

void JSPatternUI::parse_ui()
{
    handle_result(qEngine->evaluate("parse_ui()"),"parse_ui");
}

bool JSPatternUI::handle_result(QJSValue result,QString str)
{
 JSPattern::handle_result(result,str);
# if 0
    if(result.isError())
    {
   /*    qDebug()
                    << "Uncaught exception at line"
                    << result.property("lineNumber").toInt()
                    << ":" << result.toString();
        return -2;*/
        textedit->print((QString)"Uncaught exception at line" + result.property("lineNumber").toString() + ":" + result.toString());

    }
    else
        textedit->print(str + " - Success");
#endif
}

JSPatternUIStatusWindow::JSPatternUIStatusWindow(QTextEdit *textedit)
{
    con = textedit;
}
void JSPatternUIStatusWindow::clear()
{
    con->clear();
}

void JSPatternUIStatusWindow::print(QString str)
{
    con->append(str);
}

uint32_t LFSRPattern::get_lfsr_period() const
{
    return lfsr_period;
}

uint32_t LFSRPattern::get_lfsr_poly() const
{
    return lfsr_poly;
}

void LFSRPattern::set_lfsr_poly(const uint32_t &value)
{
    lfsr_poly = value;
}

uint16_t LFSRPattern::get_start_state() const
{
    return start_state;
}

void LFSRPattern::set_start_state(const uint16_t &value)
{
    start_state = value;
}

LFSRPattern::LFSRPattern()
{
    lfsr_poly = 0x01;
    start_state = 0x01;
    lfsr_period = 0;
    set_name("LFSR");
    set_description("Linear Feedback Shift Register sequence generated using Galois method");
}

uint8_t LFSRPattern::generate_pattern()
{
    uint16_t lfsr = start_state;
    int i=0;
    delete_buffer();
    buffer = new short[number_of_samples];
    do
     {
         unsigned lsb = lfsr & 1;   /* Get LSB (i.e., the output bit). */
         lfsr >>= 1;                /* Shift register */
         if (lsb) {                 /* If the output bit is 1, apply toggle mask. */
             lfsr ^= lfsr_poly;
         }
         buffer[i] = lfsr;
         i++;       
     } while (i < number_of_samples);
    return 0;
}

uint32_t LFSRPattern::compute_period()
{
    uint16_t lfsr = start_state;
    unsigned period = 0;

    do
    {
        unsigned lsb = lfsr & 1;   /* Get LSB (i.e., the output bit). */
        lfsr >>= 1;                /* Shift register */
        if (lsb) {                 /* If the output bit is 1, apply toggle mask. */
            lfsr ^= lfsr_poly;
        }
        ++period;
    } while (lfsr != start_state);
    lfsr_period = period;
    return period;
}

LFSRPatternUI::LFSRPatternUI(QWidget *parent) : PatternUI(parent)
{
    qDebug()<<"LFSRPatternUI created";
    ui = new Ui::LFSRPatternUI();
    ui->setupUi(this);
    setVisible(false);
}

LFSRPatternUI::~LFSRPatternUI(){
    qDebug()<<"LFSRPatternUI destroyed";
}
void LFSRPatternUI::build_ui(QWidget *parent){
    parent_ = parent;
    parent->layout()->addWidget(this);
}
void LFSRPatternUI::destroy_ui(){
    parent_->layout()->removeWidget(this);
//    delete ui;
}

void LFSRPatternUI::parse_ui()
{
    bool ok=0;
    set_lfsr_poly(ui->genPoly->text().toULong(&ok,16));
    if(!ok) qDebug()<<"LFSR Poly cannot be converted to int";
    set_start_state(ui->startState->text().toULong(&ok,16));

    ui->polyPeriod->setText(QString::number(compute_period()));
}

/*void LFSRPatternUI::on_setLFSRParameters_clicked()
{
}*/


uint32_t ClockPattern::get_min_sampling_freq()
{
    return frequency * (duty_cycle_granularity);
}

uint32_t ClockPattern::get_required_nr_of_samples()
{
    // greatest common divider duty cycle and 1000;0;
    return duty_cycle_granularity;
}


float ClockPattern::get_duty_cycle() const
{
    return duty_cycle;
}

void ClockPattern::set_duty_cycle(float value)
{
    if(value>100) value = 100;    

//    value = (value / (100/duty_cycle_granularity)) * (100/duty_cycle_granularity);
    duty_cycle = value;
}

float ClockPattern::get_frequency() const
{
    return frequency;
}

void ClockPattern::set_frequency(float value)
{
    frequency = value;
    if(frequency>40000000) frequency = 40000000;
    if(frequency>20000000) duty_cycle_granularity = 2;
    if(frequency>10000000 && frequency <= 20000000) duty_cycle_granularity = 4;
    if(frequency>5000000 && frequency <= 10000000) duty_cycle_granularity = 8;
    if(frequency>2000000 && frequency <= 5000000) duty_cycle_granularity = 16;
    if(frequency<2000000) duty_cycle_granularity = 20;
}

ClockPattern::ClockPattern()
{
    set_name("Clock");
    set_description("Clock pattern");
}

uint8_t ClockPattern::generate_pattern()
{
    float period_number_of_samples = (float)sample_rate/frequency;
    qDebug()<<"period_number_of_samples - "<<period_number_of_samples;
    float number_of_periods = number_of_samples / period_number_of_samples;
    qDebug()<<"number_of_periods - " << number_of_periods;
    float low_number_of_samples = (period_number_of_samples * (100-duty_cycle)) / 100;
    qDebug()<<"low_number_of_samples - " << low_number_of_samples;
    float high_number_of_samples = period_number_of_samples - low_number_of_samples;
    qDebug()<<"high_number_of_samples - " << high_number_of_samples;

    delete_buffer();
    buffer = new short[number_of_samples];
    int i=0;
    while(i<number_of_samples)
    {
        if(i % ((int)period_number_of_samples) < low_number_of_samples)
            buffer[i] = 0;
        else
            buffer[i] = 0xffff;
        //buffer[i] = (number_of_samples % period_number_of_samples) ;
        i++;
    }
    return 0;
}

ClockPatternUI::ClockPatternUI(QWidget *parent) : PatternUI(parent)
{
    qDebug()<<"ClockPatternUI created";
    ui = new Ui::ClockPatternUI();
    ui->setupUi(this);
    setVisible(false);
}

ClockPatternUI::~ClockPatternUI(){
    qDebug()<<"ClockPatternUI destroyed";
}
void ClockPatternUI::build_ui(QWidget *parent){
    parent_ = parent;
    parent->layout()->addWidget(this);
}
void ClockPatternUI::destroy_ui(){
    parent_->layout()->removeWidget(this);
//    delete ui;
}

void ClockPatternUI::parse_ui()
{
    bool ok =0;
    set_frequency(ui->frequencyEdit->text().toFloat(&ok));
    if(!ok) qDebug()<<"Cannot set frequency, not a float";
    set_duty_cycle(ui->dutyEdit->text().toFloat(&ok));
    if(!ok) qDebug()<<"Cannot set duty, not a float";
}

/*void ClockPatternUI::on_setClockParams_clicked()
{

}*/

void BinaryCounterPatternUI::parse_ui()
{
    bool ok = false;
    set_frequency(ui->frequencyEdit->text().toULong(&ok));
    if(!ok) qDebug()<<"Cannot set frequency, not a uint32";
    set_start_value(ui->startEdit->text().toUInt(&ok));
    if(!ok) qDebug()<<"Cannot set frequency, not a uint16";
    set_end_value(ui->endEdit->text().toUInt(&ok));
    if(!ok) qDebug()<<"Cannot set frequency, not a uint16";
    set_increment(ui->incrementEdit->text().toUInt(&ok));
    if(!ok) qDebug()<<"Cannot set frequency, not a uint16";
}

/*void adiscope::BinaryCounterPatternUI::on_setBinaryCounterParams_clicked()
{
}*/

} /* namespace adiscope */
