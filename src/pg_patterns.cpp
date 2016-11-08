
#include <QDebug>
#include <QFile>
#include <QtQml/QJSEngine>
#include <QtQml/QQmlEngine>
#include <QPushButton>


#include <QJsonDocument>

#include "pg_patterns.hpp"

using namespace std;
using namespace adiscope;

namespace adiscope {

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

std::string Pattern::toString()
{
    return "";
}

bool Pattern::fromString(std::string from)
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
static PatternUI *create_ui(int index, QWidget *parent = 0)
{

}

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

uint16_t BinaryCounterPattern::get_init_value() const
{
    return init_value;
}

void BinaryCounterPattern::set_init_value(const uint16_t &value)
{
    init_value = value;
}

std::string BinaryCounterPattern::toString()
{/*
    return "\""+get_name()+"\":" + "{" + "\"frequency\":" + std::to_string(frequency)
            + ",\"start_value\":" + std::to_string(start_value)
            + ",\"end_value\":" + std::to_string(end_value)
            + ",\"increment\":" + std::to_string(increment)
            + ",\"init_value\":" + std::to_string(init_value) + "}";*/
 /*   QJsonObject obj;

    obj.insert("frequency",QJsonValue((qint32)frequency));
    obj.insert("start_value",QJsonValue(start_value));
    obj.insert("end_value",QJsonValue(end_value));
    obj.insert("increment",QJsonValue(increment));
    obj.insert("init_value",QJsonValue(init_value));
    QJsonObject container;
    container.insert(QString::fromStdString(get_name()),obj);
    QJsonDocument doc(container);
    QString strJson(doc.toJson(QJsonDocument::Compact));
    return strJson.toStdString();*/

    //return frequency << start_value << end_value << increment << init_value;
}

bool BinaryCounterPattern::fromString(std::string from)
{

   //from >>
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
    auto i=init_value;
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

void BinaryCounterPatternUI::parse_ui()
{
    bool ok = false;
    set_frequency(ui->frequencyEdit->text().toULong(&ok));
    if(!ok) qDebug()<<"Cannot set frequency, not a uint32";
    set_init_value(ui->inittval_LE->text().toULong(&ok));
    if(!ok) qDebug()<<"Cannot set_init_value, not a uint32";

    set_start_value(ui->startEdit->text().toUInt(&ok));
    if(!ok) qDebug()<<"Cannot set frequency, not a uint16";
    set_end_value(ui->endEdit->text().toUInt(&ok));
    if(!ok) qDebug()<<"Cannot set frequency, not a uint16";
    set_increment(ui->incrementEdit->text().toUInt(&ok));
    if(!ok) qDebug()<<"Cannot set frequency, not a uint16";
}

GrayCounterPattern::GrayCounterPattern()
{
    set_name("Gray Counter");
    set_description("Gray Counter");
    set_periodic(true);
}

uint8_t GrayCounterPattern::generate_pattern()
{
    delete_buffer();
    buffer = new short[number_of_samples];
    auto samples_per_count = ((float)sample_rate/(float)frequency);
    auto i=init_value;
    auto j=0;

    while(j<number_of_samples)
    {
        for(auto k=0;k<samples_per_count;k++,j++)
        {
            if(j>=number_of_samples)break;
            buffer[j] = i ^ (i >> 1);
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

GrayCounterPatternUI::GrayCounterPatternUI(QWidget *parent) : PatternUI(parent){
    qDebug()<<"GrayCounterPatternUI Created";
    ui = new Ui::BinaryCounterPatternUI();
    ui->setupUi(this);
    setVisible(false);
}
GrayCounterPatternUI::~GrayCounterPatternUI(){
    qDebug()<<"BinaryCounterPatternUI Destroyed";
}
void GrayCounterPatternUI::build_ui(QWidget *parent){
    parent_ = parent;
    parent->layout()->addWidget(this);

}
void GrayCounterPatternUI::destroy_ui(){
    parent_->layout()->removeWidget(this);
    //   delete ui;
}

void GrayCounterPatternUI::parse_ui()
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
    set_periodic(false);
}

uint8_t LFSRPattern::generate_pattern()
{
    // https://en.wikipedia.org/wiki/Linear-feedback_shift_register
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

int ClockPattern::get_phase() const
{
    return phase;
}

void ClockPattern::set_phase(int value)
{
    phase = value;
    if(phase>360) phase = phase%360;
    if(phase<0) phase = 360-(abs(phase)%360);

}

ClockPattern::ClockPattern()
{
    set_name("Clock");
    set_description("Clock pattern");
    set_periodic(true);
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

    // phased samples
    int phased = (period_number_of_samples * phase/360);

    while(i<number_of_samples)
    {
        if((i+phased) % ((int)period_number_of_samples) < low_number_of_samples)
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
}

void ClockPatternUI::parse_ui()
{
    bool ok =0;
    set_frequency(ui->frequencyEdit->text().toFloat(&ok));
    if(!ok) qDebug()<<"Cannot set frequency, not a float";
    set_duty_cycle(ui->dutyEdit->text().toFloat(&ok));
    if(!ok) qDebug()<<"Cannot set duty, not a float";
    set_phase(ui->phaseLineEdit->text().toFloat(&ok));
    if(!ok) qDebug()<<"Cannot set phase, not a float";
}

bool ConstantPattern::get_constant() const
{
    return constant;
}

void ConstantPattern::set_constant(bool value)
{
    constant = value;
}

ConstantPattern::ConstantPattern()
{
    set_name(ConstantPatternName);
    set_description(ConstantPatternDescription);
    set_periodic(false);
}
uint8_t ConstantPattern::generate_pattern()
{
    delete_buffer();
    buffer = new short[number_of_samples];
    for(auto i=0;i<number_of_samples;i++)
    {
        if(constant) buffer[i] = 0xffff;
        else         buffer[i] = 0x0000;
    }
    return 0;
}

ConstantPatternUI::ConstantPatternUI(QWidget *parent) : PatternUI(parent)
{
    qDebug()<<"ConstantPatternUI created";
    ui = new Ui::ConstantPatternUI();
    ui->setupUi(this);
    setVisible(false);
}

ConstantPatternUI::~ConstantPatternUI(){
    qDebug()<<"ConstantPatternUI destroyed";
}
void ConstantPatternUI::build_ui(QWidget *parent){
    parent_ = parent;
    parent->layout()->addWidget(this);
}
void ConstantPatternUI::destroy_ui(){
    parent_->layout()->removeWidget(this);
}

void ConstantPatternUI::parse_ui()
{
    /* bool ok =0;
    set_frequency(ui->frequencyEdit->text().toFloat(&ok));
    if(!ok) qDebug()<<"Cannot set frequency, not a float";*/
    bool temp = ui->constantComboBox->currentText().toInt();
    set_constant(temp);

}

uint16_t NumberPattern::get_nr() const
{
    return nr;
}

void NumberPattern::set_nr(const uint16_t &value)
{
    nr = value;
}

NumberPattern::NumberPattern()
{
    set_name(NumberPatternName);
    set_description(NumberPatternDescription);
    set_periodic(false);
}
uint8_t NumberPattern::generate_pattern()
{
    delete_buffer();
    buffer = new short[number_of_samples];
    for(auto i=0;i<number_of_samples;i++)
    {
        buffer[i] = nr;
    }
    return 0;
}


NumberPatternUI::NumberPatternUI(QWidget *parent) : PatternUI(parent)
{
    qDebug()<<"NumberPatternUI created";
    ui = new Ui::NumberPatternUI();
    ui->setupUi(this);
    setVisible(false);
}

NumberPatternUI::~NumberPatternUI(){
    qDebug()<<"NumberPatternUI destroyed";
}
void NumberPatternUI::build_ui(QWidget *parent){
    parent_ = parent;
    parent->layout()->addWidget(this);
}
void NumberPatternUI::destroy_ui(){
    parent_->layout()->removeWidget(this);
}

void NumberPatternUI::parse_ui()
{
    bool ok =0;
    if(ui->numberLineEdit->text().toInt() > 65535) ui->numberLineEdit->setText("65535");
    set_nr(ui->numberLineEdit->text().toInt(&ok,10));
    if(!ok) qDebug()<<"Cannot set frequency, not an int";
}

RandomPattern::RandomPattern()
{
    set_name(RandomPatternName);
    set_description(RandomPatternDescription);
    set_periodic(false);
}

uint32_t RandomPattern::get_min_sampling_freq()
{
    return frequency;
}

uint32_t RandomPattern::get_required_nr_of_samples()
{
    return 1;
}

uint32_t RandomPattern::get_frequency() const
{
    return frequency;
}

void RandomPattern::set_frequency(const uint32_t &value)
{
    frequency = value;
}

uint8_t RandomPattern::generate_pattern()
{
    delete_buffer();
    buffer = new short[number_of_samples];
    auto samples_per_count = ((float)sample_rate/(float)frequency);
    int j=0;
    while(j<number_of_samples)
    {
        uint16_t random_value = rand() % (1<<number_of_channels);
        for(auto k=0;k<samples_per_count;k++,j++)
        {
            if(j>=number_of_samples)break;
            buffer[j] = random_value;
        }

    }
    return 0;
}

RandomPatternUI::RandomPatternUI(QWidget *parent)
{
    qDebug()<<"RandomPatternUI created";
    ui = new Ui::FrequencyPatternUI();
    ui->setupUi(this);
    setVisible(false);
}
RandomPatternUI::~RandomPatternUI()
{
    qDebug()<<"NumberPatternUI destroyed";
}

void RandomPatternUI::parse_ui()
{
    bool ok =0;
    set_frequency(ui->frequencyLineEdit->text().toInt(&ok,10));
    if(!ok) qDebug()<<"Cannot set frequency, not an int";
}

void RandomPatternUI::build_ui(QWidget *parent)
{
    parent_ = parent;
    parent->layout()->addWidget(this);
}

void RandomPatternUI::destroy_ui()
{
    parent_->layout()->removeWidget(this);
}


PulsePattern::PulsePattern()
{
    set_name(PulsePatternName);
    set_description(PulsePatternDescription);
    set_periodic(true);
}

uint8_t PulsePattern::generate_pattern()
{
    delete_buffer();
    buffer = new short[number_of_samples];

    float period_number_of_samples = high_number_of_samples+low_number_of_samples;
    qDebug()<<"period_number_of_samples - "<<period_number_of_samples;
    float number_of_periods = number_of_samples / period_number_of_samples;
    qDebug()<<"number_of_periods - " << number_of_periods;

    delete_buffer();
    buffer = new short[number_of_samples];
    int i=0;

    auto cnt = counter_init;
    auto div_cnt = divider_init;

    // TODO: Compute divider and divider_cnt based on actual sample rate - from 80MHz
    while(i<number_of_samples)
    {
        if(div_cnt<divider)
        {
            div_cnt++;
        }
        else
        {
            div_cnt = 0;
            cnt++;
        }
        if(cnt<low_number_of_samples)
        {
            buffer[i] = 0x0000;
        }
        else if(cnt>=low_number_of_samples && cnt<(low_number_of_samples+high_number_of_samples))
        {
            buffer[i] = 0xffff;
        }
        else
        {
            cnt = 0;
        }
        i++;

    }
}

uint32_t PulsePattern::get_min_sampling_freq()
{
    return 1;
}

uint32_t PulsePattern::get_required_nr_of_samples()
{
    return 1;
}

bool PulsePattern::get_start() { return start;}
uint32_t PulsePattern::get_low_number_of_samples()  { return low_number_of_samples; }
uint32_t PulsePattern::get_high_number_of_samples() { return high_number_of_samples; }
uint32_t PulsePattern::get_counter_init()           { return counter_init; }
uint16_t PulsePattern::get_divider()                { return divider; }
uint16_t PulsePattern::get_divider_init()           { return divider_init; }

void PulsePattern::set_start(bool val)  { start=val;}
void PulsePattern::set_low_number_of_samples(uint32_t val)  { low_number_of_samples=val;}
void PulsePattern::set_high_number_of_samples(uint32_t val) { high_number_of_samples=val;}
void PulsePattern::set_counter_init(uint32_t val)           { counter_init=val;}
void PulsePattern::set_divider(uint16_t val)                { divider=val;}
void PulsePattern::set_divider_init(uint16_t val)           { divider_init=val;}

PulsePatternUI::PulsePatternUI(QWidget *parent)
{
    qDebug()<<"PulsePatternUI created";
    ui = new Ui::PulsePatternUI();
    ui->setupUi(this);
    setVisible(false);
}
PulsePatternUI::~PulsePatternUI()
{
    qDebug()<<"PulsePatternUI destroyed";
}

void PulsePatternUI::parse_ui()
{
    bool ok =0;
    set_start(ui->start_CB->currentText().toInt(&ok,10));
    if(!ok) qDebug()<<"Cannot set start, not an int";

    set_counter_init(ui->counterInit_LE->text().toInt(&ok,10));
    if(!ok) qDebug()<<"Cannot set counter_init, not an int";

    set_low_number_of_samples(ui->low_LE->text().toInt(&ok,10));
    if(!ok) qDebug()<<"Cannot set_low_number_of_samples, not an int";

    set_high_number_of_samples(ui->high_LE->text().toInt(&ok,10));
    if(!ok) qDebug()<<"Cannot set_high_number_of_samples, not an int";

    set_divider(ui->divider_LE->text().toInt(&ok,10));
    if(!ok) qDebug()<<"Cannot set_divider, not an int";
    if(get_divider()<0)  set_divider(1);

    set_divider_init(ui->dividerInit_LE->text().toInt(&ok,10));
    if(!ok) qDebug()<<"Cannot set_divider_init, not an int";

}

void PulsePatternUI::build_ui(QWidget *parent)
{
    parent_ = parent;
    parent->layout()->addWidget(this);
}

void PulsePatternUI::destroy_ui()
{
    parent_->layout()->removeWidget(this);
}

JohnsonCounterPattern::JohnsonCounterPattern()
{
    set_name(JohnsonCounterPatternName);
    set_description(JohnsonCounterPatternDescription);
    set_periodic(true);
}

uint32_t JohnsonCounterPattern::get_min_sampling_freq()
{
    return frequency;
}

uint32_t JohnsonCounterPattern::get_required_nr_of_samples()
{
    // greatest common divider duty cycle and 1000;0;
    return ((float)sample_rate/(float)frequency) * (2*number_of_channels);
}

uint32_t JohnsonCounterPattern::get_frequency() const
{
    return frequency;
}

void JohnsonCounterPattern::set_frequency(const uint32_t &value)
{
    frequency = value;
}

uint8_t JohnsonCounterPattern::generate_pattern()
{
    delete_buffer();
    buffer = new short[number_of_samples];
    auto samples_per_count = ((float)sample_rate/(float)frequency);
    auto i=0;
    auto j=0;

    while(j<number_of_samples)
    {
        for(auto k=0;k<samples_per_count;k++,j++)
        {
            if(j>=number_of_samples)break;
            buffer[j] = i;
        }
        i=(i>>1) | (((~i)&0x01) << (number_of_channels-1));

    }
    return 0;
}

JohnsonCounterPatternUI::JohnsonCounterPatternUI(QWidget *parent)
{
    qDebug()<<"JohnsonCounterPatternUI created";
    ui = new Ui::FrequencyPatternUI();
    ui->setupUi(this);
    setVisible(false);
}
JohnsonCounterPatternUI::~JohnsonCounterPatternUI()
{
    qDebug()<<"JohnsonCounterPatternUI destroyed";
}

void JohnsonCounterPatternUI::parse_ui()
{
    bool ok =0;
    set_frequency(ui->frequencyLineEdit->text().toInt(&ok,10));
    if(!ok) qDebug()<<"Cannot set frequency, not an int";
}

void JohnsonCounterPatternUI::build_ui(QWidget *parent)
{
    parent_ = parent;
    parent->layout()->addWidget(this);
}

void JohnsonCounterPatternUI::destroy_ui()
{
    parent_->layout()->removeWidget(this);
}

uint32_t WalkingPattern::get_frequency() const
{
    return frequency;
}

void WalkingPattern::set_frequency(const uint32_t &value)
{
    frequency = value;
}

uint16_t WalkingPattern::get_length() const
{
    return length;
}

void WalkingPattern::set_length(const uint16_t &value)
{
    length = value;
}

bool WalkingPattern::get_right() const
{
    return right;
}

void WalkingPattern::set_right(bool value)
{
    right = value;
}

bool WalkingPattern::get_level() const
{
    return level;
}

void WalkingPattern::set_level(bool value)
{
    level = value;
}

WalkingPattern::WalkingPattern()
{
    set_name(WalkingCounterPatternName);
    set_description(WalkingCounterPatternDescription);
    set_periodic(1);
}

uint32_t WalkingPattern::get_min_sampling_freq()
{
    return frequency;
}

uint32_t WalkingPattern::get_required_nr_of_samples()
{
    // greatest common divider duty cycle and 1000;0;
    return ((float)sample_rate/(float)frequency) * (number_of_channels);
}

uint8_t WalkingPattern::generate_pattern()
{
    delete_buffer();
    buffer = new short[number_of_samples];
    auto samples_per_count = ((float)sample_rate/(float)frequency);
    uint16_t i;
    i = (1<<length) - 1;
    if(!level) i = ~i;

    auto j=0;

    while(j<number_of_samples)
    {

        for(auto k=0;k<samples_per_count;k++,j++)
        {
            if(j>=number_of_samples)break;
            buffer[j] = i;
        }
        if(right)
        {
            auto dummy = i&0x01;
            i=(i>>1);
            if(dummy) i = i | (1 << (number_of_channels-1));
            else      i = i & ~(1<<(number_of_channels-1));
        }
        else      i=(i<<1) | ((i&(1<<number_of_channels)) >> number_of_channels);

    }
    return 0;
}

WalkingPatternUI::WalkingPatternUI(QWidget *parent)
{
    qDebug()<<"WalkingCounterPatternUI created";
    ui = new Ui::WalkingPatternUI();
    ui->setupUi(this);
    setVisible(false);
}
WalkingPatternUI::~WalkingPatternUI()
{
    qDebug()<<"WalkingCounterPatternUI destroyed";
}

void WalkingPatternUI::parse_ui()
{
    bool ok =0;
    set_frequency(ui->frequency_LE->text().toInt(&ok,10));
    if(!ok) qDebug()<<"Cannot set frequency, not an int";
    set_length(ui->length_LE->text().toInt(&ok,10));
    if(!ok) qDebug()<<"Cannot set length_LE, not an int";
    set_level(ui->level_CB->currentText().toInt(&ok,10));
    if(!ok) qDebug()<<"Cannot set frequency, not an int";
    set_right("Right"==ui->direction_CB->currentText());
    if(!ok) qDebug()<<"Cannot set frequency, not an int";
}

void WalkingPatternUI::build_ui(QWidget *parent)
{
    parent_ = parent;
    parent->layout()->addWidget(this);
}

void WalkingPatternUI::destroy_ui()
{
    parent_->layout()->removeWidget(this);
}


}
