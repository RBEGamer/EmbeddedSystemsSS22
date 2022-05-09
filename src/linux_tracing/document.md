# Einleitung

Tracing ist die spezielle Verwendung der Protokollierung zur Aufzeichnung von Informationen über den Ausführungsablauf eines Programms.
Oft werden mit eigenständig hinzugefügte Print-Messages der Code debuggt. Somit verfolgt man die Anweisungen mit einem eigenem tracing-System.
Linux bringt einige eigenständige Tools mit, mit denen es möglich ist Vorgänge innerhalb von einem Embedded-System nachvollziehen und analysieren zu können.
Die Linux-Tracing Funktionalität und die bestehenden Tools, welche im Linux-Kernel integriert sind, helfen so dabei bei der Identifikation von Laufzeiten, Nebenläufigkeiten  und der Untersuchung von Latenzproblemen,

## Relevanz

* multicore systeme

## Tracing


## Ursprung


# Grundlagen

## Ringbuffer

## Debug-Filesystem

* möglichkeiten

```bash
#ENABLE DEBUG FS
$ sudo mount -t debugfs debugfs /sys/kernel/debug
```

## Trace Events


Durch das Debug-Filesystem ist jetzt der Zugriff auf die Debug und insbesondere auf die Tracing-Daten möglich.
Im Debug-Filesystem ist nach aktivierung der `tracing`-Ordner vorhanden.
In diesem werden die verfügbaren Events in Gruppen (Ordnern) dargestellt, auf welche im späteren Verlauf reagiert werden können.


```bash
# GET TRACERS
$ cat /sys/kernel/debug/tracing/available_tracers

# GET AVAILABLE EVENT LIST
$ cd /sys/kernel/debug/tracing
$ ls -1 events/

# sched
# irq
```

### Scheduler
### IRQ

## Abfangen von Events

```bash
 $ cd /sys/kernel/debug/tracing
 $ echo 1 > events/sched/enable
```
### Kprobes

Kprobes können dazu verwendet werden, Laufzeit und Performance-Daten des Kernels zu sammeln.
Der Vorteil and diesen ist, dass diese Daten ohne Unterbrechnung der Ausführung auf CPU-Instruktions-Ebene aggregiert werden können, anders wie bei dem Debuggen eines Programms mittels Breakpoints.
Ein weiterer Vorteil ist, dass das Registrieren der Kprobes dynamisch zur Laufzeit und ohne Änderungen des Programmcodes geschieht.

### CPU-Traps


# Tracing auf Mikrokontrollern

# Tools

Allgemein sind keine speziellen Programme notwending um die Laufzeiteigenschaften eines Programms aufzuzeichnen.
Der Linux-Kernel bringt bereits alle nötigen Funktionalitäten mit. Jedoch gibt es Tools die eine visuelle Darstellung der aufgezeichneten Events ermöglichen.


## Trace-Log Aufzeichnung

### trace-cmd

```bash
# CHECK IF TRACING IS ENABLED
# RUN AS SUDO
$ mount | grep tracefs
## => none on /sys/kernel/tracing type tracefs (rw,relatime,seclabel)

# IF TRACING IS NOT ACTIVE, ENABLE IT FIRST

## ONLY SCHEDULER EVENTS
$ echo sched_wakeup >> /sys/kernel/debug/tracing/set_event
## ALL EVENTS
$ echo *:* > /sys/kernel/debug/tracing/set_event


# RECORD
$ trace-cmd record -e sched ./program_executable
```

## Visualisierung

### trace-cmd

Die trace-cmd Konsolenanwendung dient nicht nur zur Aufzeichnung der Trace-Events, sondern bietet auch die Möglichkeit augezeichnetet Reports visuell darzustellen.
Die Ausgabe erfolgt als Tabelle in der Konsole und ist somit rein Textbasiert.



### Kernelshark

![Kernelshark \label{kernelshark}](images/kernelshark.png)

# Interpretation des Kernel-Trace Ergebnisses

# Beispiel der Identifikation von Laufzeitproblemen

In diesem Abschnitt soll an einem einfache Beispiel gezeigt werden
## Ausgangsszenario

Als Ausgangspunkt dieses Beispiels, soll das Laufzeitverhalten eines Programms auf einem Linux-System analysiert werden.
Die zugrunde liegende Software wurde bisher nur auf einem Linux-Realtime Kernel verwendet,
jedoch erfordert die Implementation neuer Features eine neuere Kernel-Version, welche noch nicht als RT-Version auf dem System zur Verfügung steht.
Somit soll ermittelt werden, ob die unmodifizierte Software eins zu eins auf dem neuen System lauffähig ist und die Laufzeitandorderungen erfüllt.

Das System besteht hier aus einem `RaspberryPi 4B` mit einer angeschlossenen LED am GPIO-Port `25`
und zu testende Programm lässt diese dabei in 100ms Abständen Blinken.

```c++
#include <iostream>
#include <wiringPi.h>
#include <csignal>
using namespace std;

void signal_callback_handler(int signum) {

}

int main(int argc, char *argv[])
{
    //REGISTER SIGNAL HANDLER
    signal(SIGINT, signal_callback_handler);

    wiringPiSetup();			// Setup the library
    pinMode(0, OUTPUT);		// Configure GPIO0 as an output
    pinMode(1, INPUT);		// Configure GPIO1 as an input
    bool state = false;

    while(1)
    {
        state = !state;
	    digitalWrite(0, state);
        delay(500);
    }
	return 0;
}
```

Für den Test wurde als RT Kernel die Version `4.19.59-rt23-v7l+` verwendet, welche nicht alle Funktionaltitäten des aktuellen `5.10` Kernel besitzt.
In diesem fiktiven Beispiel, wird die `systemd-networking >V.248` Funktionalität für das Batman-Prokoll benötigt, welche den Grund für die Umstellung darstellt und nicht trivial in den `4.x` Kernel integriert werden kann.

Die Messungen wurden zuerst auf dem aktuellen `5.10 LTS` Kernel aufgezeichnet und im Anschluss wurde der RT-Kernel auf einem anderen System per Cross-Compilation aus dem `rpi-4.19.y-rt` Branch des `raspberrypi/linux` Repository gebaut.
Dieser Schritt war notwendig, da es kein fertiges RT-Kernel Image zur Verfügung stand.
Die erzeugten Dateien wurden dann auf die Boot-Partition der SD Karte geschrieben und in der `/boot/config.txt` Datei wurde der neue Kernel hinterlegt `kernel=kernel7_rt.img`.



## Aufzeichnung Trace-Log

Zur Aufzeichnung des Trace-Logs wurde `trace-cmd` verwendet. Auf dem Zielsystem wurde dabei nur die Aufzeichnung vorgenommen und die Analyse der Logs erfolgte auf einem seperaten System.

```bash
trace-cmd record -e sched ./blink
``` 

## Visualisierung und Beurteilung des Trace-Logs mittels kernelshark


# Fazit