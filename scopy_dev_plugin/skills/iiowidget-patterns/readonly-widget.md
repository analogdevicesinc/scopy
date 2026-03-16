# Read-Only Widget Patterns

Disabled widgets that display attribute values without user interaction.

## Pattern 1: Basic read-only (readRequested-driven)

The simplest read-only widget. Uses `compactMode` and `setEnabled(false)`. Reads only when `readRequested` is emitted (e.g., on refresh button click).

```cpp
// ad6676.cpp:93 — Basic read-only with compact mode
IIOWidget *Ad6676::createReadOnlyWidget(iio_channel *ch, const QString &attr, const QString &title)
{
    IIOWidget *widget = IIOWidgetBuilder(m_centralWidget)
                .device(m_dev)
                .channel(ch)
                .attribute(attr)
                .title(title)
                .compactMode(true)
                .group(m_group)
                .buildSingle();

    if(widget) {
        widget->setEnabled(false);
        widget->showProgressBar(false);
        connect(this, &Ad6676::readRequested, widget, &IIOWidget::readAsync);
    }
    return widget;
}
```

## Pattern 2: Timer-based continuous polling

Reads the attribute periodically using a QTimer. Good for values that change over time (e.g., RSSI, temperature, status).

```cpp
// adrv9002.cpp:758 — Continuous polling every 10 seconds
IIOWidget *Adrv9002::createContinuousReadOnlyWidget(iio_channel *ch, const QString &attr,
                                                     const QString &title)
{
    IIOWidget *widget = IIOWidgetBuilder(m_centralWidget)
                .device(m_iio_dev)
                .channel(ch)
                .attribute(attr)
                .title(title)
                .compactMode(true)
                .group(m_group)
                .buildSingle();

    if(widget) {
        widget->setEnabled(false);
        widget->showProgressBar(false);

        connect(this, &Adrv9002::readRequested, widget, &IIOWidget::readAsync);
        QTimer *timer = new QTimer(widget);
        QObject::connect(timer, &QTimer::timeout, [widget]() { widget->readAsync(); });
        timer->start(10000);  // 10 second interval
    }
    return widget;
}
```

## Pattern 3: 2-failure stop timer

A timer that automatically stops polling after 2 consecutive read failures. Prevents flooding the device with failed reads when it becomes unavailable.

```cpp
// Standard pattern from codebase
IIOWidget *widget = IIOWidgetBuilder(parent)
            .channel(ch)
            .attribute("status_attr")
            .title("Status Display Name")
            .group(m_group)
            .buildSingle();
widget->setEnabled(false);

QTimer *timer = new QTimer(parent);
int *failCount = new int(0);
connect(timer, &QTimer::timeout, widget, &IIOWidget::readAsync);
connect(widget, &IIOWidget::currentStateChanged, timer,
    [timer, failCount](IIOWidget::State state, QString) {
        if(state == IIOWidget::Error) {
            (*failCount)++;
            if(*failCount >= 2) timer->stop();
        } else if(state == IIOWidget::Correct) {
            *failCount = 0;
        }
    });
timer->start(1000);
```

## When to Use Which

| Pattern | Use When |
|---------|----------|
| Basic read-only | Value only needs reading on manual refresh |
| Timer-based | Value changes over time, continuous monitoring needed |
| 2-failure stop | Like timer-based, but device may disconnect or fail |

## Common Errors

- **Using `readFailed`/`readSuccess` signals**: These don't exist. Use `currentStateChanged` with `IIOWidget::State` enum (`Error`, `Correct`).
- **Forgetting `setEnabled(false)`**: Widget will appear interactive but writes will fail or cause confusion.
- **Forgetting `showProgressBar(false)`**: The progress bar will flash on every read cycle, which is distracting for a status display.
- **Timer without cleanup**: Use `new QTimer(widget)` (parent = widget) so the timer is destroyed when the widget is destroyed.
- **Missing `readRequested` connection on timer widgets**: Even timer-based widgets should also connect to `readRequested` so they respond to manual refresh.
