# PRAKTIKUM 1 - REAKTIONSSPIEL


## AUFBAU

| DISPLAY | STM-DISCOVERY |
|---------|---------------|
| VCC     | 5V            |
| GND     | GND           |
| SCL     | PB6 (I2C1)    |
| SDA     | PB9 (I2C1)    |




## TIMER 7 - MEASUREMENT TIMER

Der Timer7 wird in diesem Projekt, für das Messen der Zeit zwischen Spielstart `state=STATUS_RANDOM_TIMER_EXPIRED` und der Wechsel in den `state=STATUS_MEASUREMENT_DONE` Status.
Dieser wird mit den Betätigen des User-Buttons `PA0` gewechselt.
Der Timer soll eine Periodendauer von `1ms` haben, somit soll auch die Zeit in `ms` Schritten angezeigt werden.
Hierzu muss der Prescaler angepasst werden. Das Ergebnis (Zeit, zwischen Start/Stop des Timers) kann dann aus dem Counter Register `htim7.Instance->CNT` entnommen werden.

[tim7_1ms](./images/tim7_1ms.png)

In der Clock-Configuration, wird der Timer7 über die `APB1 Timer-Clock` Einstellung eingestellt.
Diese ist in der Projektmapp auf `10.5 MHz` eingestellt. Mittels des Prescalers des Timers kann der Takt reduziert werden.
Hier `10.000.000 / 1000 ms/s * 10 counter_period = 1.000 PSC`, somit wird der Interrupt jede Milli-Sekunde ausgelößt.

[tim7_1ms_cap](./images/tim7_1ms_cap.png)

Die Messung zeigt, dass mit diesen Einstellungen eine Periodendauer von `1.0489ms` erreicht wird, welches für diesen Anwendungsfall ausreichend ist.


## TIMER 7 - RANDOM TIMER

