# Scopy Plugin Patterns — Full Reference

## Plugin Lifecycle Methods

### initMetadata()
```cpp
void MyPlugin::initMetadata()
{
    loadMetadata(R"plugin(
    {
       "priority":100,
       "category":["iio"],
       "exclude":["m2kplugin"]
    }
)plugin");
}
```

### onConnect()
```cpp
bool MyPlugin::onConnect()
{
    Connection *conn = ConnectionProvider::open(m_param);
    if(!conn) return false;

    m_widgetGroup = new IIOWidgetGroup(this);

    MyTool *tool = new MyTool(conn->context(), m_widgetGroup);
    m_toolList[0]->setTool(tool);
    m_toolList[0]->setEnabled(true);
    m_toolList[0]->setRunBtnVisible(false);

    return true;
}
```

### onDisconnect()
```cpp
bool MyPlugin::onDisconnect()
{
    for(auto &tool : m_toolList) {
        tool->setEnabled(false);
        tool->setRunning(false);
        tool->setRunBtnVisible(false);
        QWidget *w = tool->tool();
        if(w) { tool->setTool(nullptr); delete(w); }
    }
    if(m_widgetGroup) { delete m_widgetGroup; m_widgetGroup = nullptr; }
    ConnectionProvider::close(m_param);
    return true;
}
```

## ToolTemplate Setup

```cpp
void MyTool::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_tool = new ToolTemplate(this);
    m_tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_tool->topContainer()->setVisible(true);
    m_tool->topContainerMenuControl()->setVisible(false);
    mainLayout->addWidget(m_tool);

    // Refresh button
    m_refreshButton = new AnimatedRefreshBtn(false, this);
    m_tool->addWidgetToTopContainerHelper(m_refreshButton, TTA_RIGHT);

    // Central scroll area
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *centralLayout = new QVBoxLayout(centralWidget);
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(centralWidget);
    m_tool->addWidgetToCentralContainerHelper(scrollArea);
}
```

## Refresh Button Pattern

```cpp
connect(m_refreshButton, &QPushButton::clicked, this, [this]() {
    m_refreshButton->startAnimation();
    QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
    connect(watcher, &QFutureWatcher<void>::finished, this,
        [this, watcher]() {
            m_refreshButton->stopAnimation();
            watcher->deleteLater();
        }, Qt::QueuedConnection);
    QFuture<void> future = QtConcurrent::run([this]() {
        Q_EMIT readRequested();
    });
    watcher->setFuture(future);
});
```

## Section Widget Pattern

```cpp
QWidget* createSection(const QString &title, QWidget *parent)
{
    QWidget *section = new QWidget(parent);
    Style::setBackgroundColor(section, json::theme::background_primary);
    Style::setStyle(section, style::properties::widget::border_interactive);

    QVBoxLayout *layout = new QVBoxLayout(section);
    QLabel *titleLabel = new QLabel(title, section);
    Style::setStyle(titleLabel, style::properties::label::menuBig);
    layout->addWidget(titleLabel);

    return section;
}
```

## Advanced Tool Pattern

For plugins with `adi,*` debug attributes:

```cpp
class MyPluginAdvanced : public QWidget {
    Q_OBJECT
public:
    explicit MyPluginAdvanced(iio_context *ctx, IIOWidgetGroup *group, QWidget *parent = nullptr);
Q_SIGNALS:
    void readRequested();
private:
    QStackedWidget *m_stack;
    QButtonGroup *m_btnGroup;
};
```

Each sub-tab is its own widget class:
```cpp
class TxSettingsWidget : public QWidget {
    // Uses IIOWidgetBuilder for all widget creation
    // Uses MenuSectionCollapseWidget for collapsible sections
};
```

## Logging Categories

```cpp
// Header
Q_DECLARE_LOGGING_CATEGORY(CAT_MYPLUGIN)

// Source
Q_LOGGING_CATEGORY(CAT_MYPLUGIN, "MyPlugin")

// Usage
qDebug(CAT_MYPLUGIN) << "Message";
qWarning(CAT_MYPLUGIN) << "Warning";
```
