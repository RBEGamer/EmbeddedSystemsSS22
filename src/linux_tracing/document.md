# Einleitung

Tracing ist die spezielle Verwendung der Protokollierung zur Aufzeichnung von Informationen über den Ausführungsablauf eines Programms.
Oft werden mit eigenständig hinzugefügte Print-Messages der Code debuggt. Somit verfolgt man die Anweisungen mit einem eigenem tracing-System.
Linux bringt einige eigenständige Tools mit, mit denen es möglich ist Vorgänge innerhalb von einem Embedded-System nachvollziehen und analysieren zu können.
Die Linux-Tracing Funktionalität und die bestehenden Tools, welche im Linux-Kernel integriert sind, helfen so dabei bei der Identifikation von Laufzeiten, Nebenläufigkeiten  und der Untersuchung von Latenzproblemen,

## Relevanz

* multicore systeme unterscheid mikotrkontroller

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

## Beispiel für Events

* file operartionen
* netzzwerk zugriffe



### Scheduler
### IRQ

## Abfangen von Events

```bash
 $ cd /sys/kernel/debug/tracing
 $ echo 1 > events/sched/enable
```
### kprobes

Kprobes können dazu verwendet werden, Laufzeit und Performance-Daten des Kernels zu sammeln.
Der Vorteil and diesen ist, dass diese Daten ohne Unterbrechnung der Ausführung auf CPU-Instruktions-Ebene aggregiert werden können, anders wie bei dem Debuggen eines Programms mittels Breakpoints.
Ein weiterer Vorteil ist, dass das Registrieren der Kprobes dynamisch zur Laufzeit und ohne Änderungen des Programmcodes geschieht.

### kretprobes
### CPU-Traps

### uprobes

* für anwendungn
* system libs


## Nachteile / verscfälschung

* welche effekte können entstehen
* tracing bracuth ressoucen
* last minimieren auf traget minimieren
* nur aufzeichnen und später analysieren z.B. auf einem anderen system
* wie verhindern


# Tracing auf Mikrokontrollern ?????

# Tools

Allgemein sind keine speziellen Programme notwending um die Laufzeiteigenschaften eines Programms aufzuzeichnen.
Der Linux-Kernel bringt bereits alle nötigen Funktionalitäten mit. Jedoch gibt es Tools die eine visuelle Darstellung der aufgezeichneten Events ermöglichen.


## Trace-Log Aufzeichnung

Für die Log-Aufzeichnung wird ein Ringbuffer genutzt. Das Aufzeichnen in den Ringpuffer ist Standardmäßig aktiviert. 

```bash
# Disable the Recording on the ringbuffer
$ echo 0 > tracing on
```

Mit dem folgenden Befehl kann der Inhalt des Ringuffers, auch während einer Aufzeichung, ausgebeben werden:

```bash
$ less trace
```
Das Lesen während einer Aufzeichnung mit trace hat keinerlei Einfluss auf den Inhalt des Ringpuffers. Die Ausgabe des letzten Kommandos wird dabei in einem menschenlesbaren Format dargestellt:

![Trace-Log \label{trace-log}](images/trace-log-print.png)

Die bisheirgen Aufzeichnungen der Ereignisse können mit einem einfachen Befehl entfernt werden:

```bash
$ echo > trace
```
Um einen Überlauf an Informationen zu verhinden kann die Aufzeichnung auch konsumierend gelesen werden. Somit werden beim Lesen zeitgleich diese aus dem Ringbuffer entfernt.

Eine weitere Kernpunkt ist, dass in Mehrkernsystemen für jeden einzelnen Core ein separater Ringbuffer existiert. Damit die Analyse von verschiedenen Events getrennt werden kann, kann mit jeder weiteren Instanz pro Core ein weiterer Ringbuffer angelgt werden. Dies erfolgt im Untervereziechnis `instances/`.

```bash
$ cd instances
$ mkdir inst0
$ mkdir inst1

# Remove if the instances is not needed anymore
# rmdir inst0
```

![Ringbuffer \label{ringbuffer}](images/ringbuffer.png)

### trace-cmd

Das Tool `trace-cmd` ist das bekannteste und am meisten genutzte Hilfmittel zur Aufzeichnung. Dies ist ein kommandozeilenwerkzeug, das auf den meisten gängingen Linux Distrubitionen bereits vorinstalliert ist.

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

* output erklörung

Mit dem letzten Befehl werden die ganzen Events zu Scheduler aufgezeichnet. Dabei werden während der Aufzeichnung kontinuierlich die Ringbuffer in konsumierender Form ausgelesen und in die Datei `trace.dat` geschrieben, falls mit dem `-o` keine eigene Datei eingegeben wurde. Als Informationen werden zu dem Inhalt des Ringbuffers auch zusätzlich notwendige Informationen über das Target, für die Auswertung auf beliebigen System gespeichert.

Die `trace-cmd` Konsolenanwendung dient nicht nur zur Aufzeichnung der Trace-Events, sondern bietet auch die Möglichkeit augezeichnetet Reports visuell darzustellen.
Die Ausgabe erfolgt mit dem Befehl `trace-cmd report [-i <Dateiname>]` als Tabelle in der Konsole und ist somit rein Textbasiert, siehe dazu die nächste Abbildung.

![trace-cmd Report \label{trace-cmd-report}](images/trace-png.jpg)

Auf diese Aufzeichnung zusätzlich ein Filter angewendet werden, um die Suche auf bestimten Erreignissen einzugrenzen.

Mit dem Tool ist es einfach die Teilschritte zu automatisieren.

### bpftrace

Seit der Kernelversion `>4.x`, kann ein weiteres Tool mit dem Namen `bpftrace` verwendet werden.
Dieses bietet jedoch zusätzlich eine eigene Skripsprache mit der nicht nur Aggreation, sondern auch die Eventfilter und die Verarbeitung der Ergebnisse automatisiert werden können.

```bash
# Block I/O latency as a histogram EXAMPLE
$ wget https://raw.githubusercontent.com/iovisor/bpftrace/master/tools/biolatency.bt
$ bftrace ./biolatency.bt
# @usecs:
# [512, 1K)             10 |@                       |
# [ 1K, 2K)            426 |@@@@@@@@@@@@@@@@@@      |
# [2K, 4K)             230 |@@@@@@@@@@@@@@          |
# [4K, 8K)               9 |@                       |
# [8K, 16K)            128 |@@@@@@@@@@@@@@@         |
# [16K, 32K)            68 |@@@@@@@@                |
# ...
```

### Netzwerk Paketanalyse Beispiel

* mit bfttrac verworfene netzwerkpakete zählen wenn zu viel trafic

### Kernelshark

Das zuvor erklärte `tace-cmd` ist wie oben erwähnt nur ein textbasiertes Analysetool. Das kommende Kernelshark Tool bietet dem Anwender
die Möglichkeit die Traceaufzeichnungen grafisch zu analysieren. Dabei sind die beiden Tools aufeinander abgestimmt und werden
gemeinsam entwickelt. Auch dieses Tool ist in den meisten Linux Distrubutionen vorinstalliert.

Das vom trace-cmd erzeugte `trace.dat-Format` wird im Kernelshark als Eingabe erwartet. Wenn im folgendem ersten Befehl nichts eingegeben, dann wird nach der entsprechenden `trace.dat` im Verzeichnis gesucht.

```bash
$ kernelshark
$ kernelshark -i <Dateiname>
```

Im folgenden ist die grafische Darstellung zu sehen. Dabei besitzt jeder Task ein eigenen Farbton. Für jede CPU wird eine eigene Zeile dargestellt. Dieses Tool hat eine gewisse Ähnlichkeit mit der von uns genutzten Logic 2 Software.

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

### Aktivierung der Events

```bash
$ echo 1 > /sys/kernel/debug/tracing/tracing_on
$ cat /sys/kernel/debug/tracing/trace
$echo > /sys/kernel/debug/tracing/trace

```


```bash
trace-cmd record -e sched ./blink
``` 

## Visualisierung und Beurteilung des Trace-Logs mittels kernelshark


# Fazit
