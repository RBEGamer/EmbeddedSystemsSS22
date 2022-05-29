# Einleitung

Tracing ist die spezielle Verwendung der Protokollierung zur Aufzeichnung von Informationen über den Ausführungsablauf eines Programms.
Oft werden mit eigenständig hinzugefügten Print-Messages der Code um Debug-Ausgaben erweitert.
Somit verfolgt man die Anweisungen mit einem eigenen Tracing-System.

Linux bietet einige eigenständige Tools, welche ermöglichen, Vorgänge innerhalb von einem Linux-System nachzuvollziehen und analysieren zu können.
Die Linux-Tracing Funktionalität und die zur Verfügung stehenden Tools helfen so bei der Identifikation von Laufzeiten, Nebenläufigkeiten und der Untersuchung von Latenzproblemen. Hierzu sind bereits alle nötigen Tools und Funktionalitäten im Linux-Kernel integriert. [@dbgfs]

## Relevanz

Bei Mikrokontrollern und auch im Zusammenhang mit Echtzeit-Betriebssystemen ist jede Aktion, die Ausgeführt wird, von hoher Bedeutung. Moderne Linux Systeme sind sehr komplex und bestehen aus vielen Softwaremodulen, welche auf unterschiedlichsten Weisen untereinander interagieren.
Um diese Interaktionen nachzuvollziehen können und um zu verstehen, wie sich Softwarekomponenten im Verbund verhalten, ist es wichtig, das System systemnah zu debuggen, um diese analysieren zu können. Oft können Fehler reproduziert und mit solchen Analysen identifiziert werden. Zusätzlich besteht bei Custom Driver die Möglichkeit während des Bootvorgangs zu debuggen. 

# Grundlagen

Um die Tracing-Funktionalität auf einem Linux-System verwenden zu können, muss das System für deren Verwendung vorbereitet werden.
Hierzu muss unter anderem das Debug-Filesytem auf dem Ziel-System aktiviert werden und die entsprechende Art des zu Tracings entsprechend der Anwendung gewählt werden.

## Ringbuffer

Bei einem Ringbuffer handelt sich um eine Datenstruktur, die das asynchrone Lesen und Schreiben von Informationen erleichtert. Der Puffer wird in der Regel als Array mit zwei Zeigern implementiert. Einem Lesezeiger und einem Schreibzeiger. Man liest aus dem Puffer, indem man den Inhalt des Lesezeigers liest und dann den Zeiger auf das nächste Element erhöht und ebenso beim Schreiben in den Puffer mit dem Schreibzeiger.
So werden in der eingesetzten Ringbuffer-Implementierung die Debug-Informationen gespeichert und ein Auslesen dieser ist mittels der Einträge im Debug-Filesystem möglich.

## Debug-Filesystem

Das Debug-Filesystem wurde in der Kernel-Version `2.6.10-rc3`[@dbgfs] eingeführt. Es bietet Zugriff auf Diagnose und Debug-Informationen auf Kernel-Ebene.

Ein Vorteil gegenüber des Prozess-Filesystem `/proc` ist, dass jeder Entwickler hier auch eigene Daten zur späteren Diagnose einpflegen kann.
Um das Dateisystem nutzen zu können, muss dies zuerst aktiviert werden. Nach der Aktivierung stehen die Ordner unter dem angegebenen Pfad zur Verfügung.


```bash
# ENABLE DEBUG FS
$ sudo mount -t debugfs debugfs /sys/kernel/debug
$ ls -lah /sys/kernel/debug | awk '{print $9}'
hid
usb 
Tracing
[...]
```


## Tracing

Durch das Debug-Filesystem ist der Zugriff auf die Debug und insbesondere auf die Tracing-Daten möglich.
Im Debug-Filesystem ist nach der Aktivierung die `Tracing`-Ordnerstruktur vorhanden.
In diesem werden verfügbaren Events in gruppiert in Ordnern dargestellt, auf welche im späteren Verlauf reagiert werden kann.
Zudem können hier auch die Verfügbaren `tracer` angezeigt und aktiviert werden, welche noch weitere Debugging-Optionen bereitstellen.


### Tracer

```bash
# GET TRACERS
$ cat /sys/kernel/debug/Tracing/available_tracers
hwlat blk mmiotrace function_graph wakeup_dl wakeup_rt wakeup function nop
# USE SPECIFIC TRACER
$ echo function_graph > /sys/kernel/debug/Tracing/current_tracer
# DISABLE TRACER USAGE
$ echo nop > /sys/kernel/debug/Tracing/current_tracer
```

`tracer` sind zusätzliche Tracing-Tools, welche eine gezieltere Aggregierung von Events z.B. Filterung und somit tiefergehende Analyse erlauben.
Zum Beispiel erlaubt der `ftrace`-Tracer eine detaillierte Ereignis-Filterung auf spezifizierte Events[@ftraceintroducation].

Der `function_graph`-Tracer gibt bei Verwendung zusätzliche Informationen, wie z.B. die Laufzeit von einzelnen Funktionen[@fgtrace].

Auch kann dieser den Stacktrace und den Call-Stack übersichtlich darstellen, indem hier die Namen der aufgerufenen Funktionen ausgegeben werden. 

```bash
# CALL STACK USING FUNCTION_GRAPH TRACER
$ echo function_graph > /sys/kernel/debug/Tracing/current_tracer
$ cat /sys/kernel/debug/Tracing/trace
# tracer: function_graph
# CPU-  DURATION          FUNCTION CALLS
# |     |   |            |  |   |
 0)               |      getname() {
 0)   4.442 us    |         kmem_cache_alloc() {
 0)   1.382 us    |             __might_sleep();
 0)   2.478 us    |        }
 [...]
```

### Events

Ein Event kann zum Beispiel durch das Lesen und Schreiben auf den System I2C-Bus vom Kernel ausgelöst werden. Wenn das Event im Debug-Filesystem aktiviert wurde, stellt dieses die Informationen des Events bereit. Je nach Typ können unterschiedlichste Informationen dem Nutzer bereitgestellt werden.

Im Beispiel des I2C-Events kann nachvollzogen werden, welche Nachrichten an eine bestimmte Adresse gesendet wurden.

```bash
$ mount -t debugfs none /sys/kernel/debug
$ cd /sys/kernel/debug/Tracing/
$ echo nop > current_tracer
$ echo 1 > events/i2c/enable
$ echo 1 > Tracing_on
$ cat /sys/kernel/debug/Tracing/trace
i2c_write: i2c-5 0 a=048 f=0001 l=8
i2c_read: i2c-5 1 a=048 f=0002 l=9
```

In der `ls`-Ausgabe des `events`-Ordners des Debug-Filesystems ist zu sehen, welche Events abgefangen und mittels der Linux-Tracing-Tools protokoliert werden können.

```bash
# GET AVAILABLE EVENT LIST
$ cd /sys/kernel/debug/Tracing/events
$ ls -lah | awk '{print $9}'
alarmtimer
drm
exceptions
ext4 #READPAGE, WRITEPAGE, ERROR, FREE_BLOCKS
filelock
filemap
gpio #GPIO_DIRECTION, GPIO_VALUE
hda
i2c
irq
net
smbus #READ, WRITE, REPLY
sock #STATE_CHANGED, EXCEED_BUFFER_LIMIT, REC_QUEUE_FULL
spi
tcp
timer #TIMER_STOP, TIMER_INIT, TIMER_EXPIRED
[...]
```

Alle Events sind in Gruppen gebündelt. Alle Events, welche das `ext4`-Filesystem betreffen, befinden sich im `ext4`-Ordner.
Die Auflistung zeigt einige der für das `ext4` zur Verfügung stehenden Events.
Zudem befinden sich zwei zusätzliche Dateien `enable`, `filter `in diesem Ordner.
Durch diese ist es später möglich anzugeben, ob dieses Event aufgezeichnet werden soll.

```bash
$ cd /sys/kernel/debug/Tracing/events/ext4
$ ls -lah | awk '{print $9}'
# EVENTS FOR EXT4
ext4_write_end
ext4_writepage
ext4_readpage
ext4_error
[...]
# INTERFACE FOR EVENT SETUP
enable
filter
format
```

Die optionale `format`-Datei kann zusätzliche Informationen durch das Event bereitgestellte Format der Ausgabe wiedergeben.
Das folgende Beispiel zeigt das Ausgabeformat für das Scheduler-Wakeup `sched_wakeup`-Event.
Somit kann nicht nur in Erfahrung gebracht werden, wann und ob das Event ausgelöst hat, sondern es können auch weitere Event-Spezifische Informationen durch das Event gemeldet werden.

```bash
$  cat /sys/kernel/debug/Tracing/events/sched/sched_wakeup/format
ID: 318
format:
	field:unsigned short common_type;	offset:0;	size:2;	signed:0;
	field:unsigned char common_flags;	offset:2;	size:1;	signed:0;
	field:unsigned char common_preempt_count;	offset:3;	size:1;	signed:0;
	field:int common_pid;	offset:4;	size:4;	signed:1;
	field:char comm[16];	offset:8;	size:16;	signed:1;
	field:pid_t pid;	offset:24;	size:4;	signed:1;
	field:int prio;	offset:28;	size:4;	signed:1;
	field:int success;	offset:32;	size:4;	signed:1;
	field:int target_cpu;	offset:36;	size:4;	signed:1;
```



## Abfangen von Events

Um ein `event`[@events] abfangen zu können, muss dies zuerst für die gewünschten Events aktiviert werden.
Hierzu werden die Event-Interface-Dateien verwendet, welche sich in jeder Event-Gruppe befinden.
Die simpelste Methode ist es, eine `1` oder `0` in die `enable`-Datei der Gruppe zu schreiben.
Ein spezifisches Event kann mit der gleichen Methode aktiviert werden. Hierzu wird die `enable`-Datei im eigentlichen Event-Ordner verwendet anstatt jene, welche sich in der Event-Gruppe befindet.


```bash
$ cd /sys/kernel/debug/Tracing/events/ext4
# ENABLE ALL EVENTS FROM THIS GROUP
$ echo 1 > ./enable
# DISBALE ALL EVENTS
$ echo 0 > ./enable
# ENBABLE SPECIFIC EVENT
$ echo 1 > ./ext4_readpage/enable
$ echo 1 > ./ext4_writepage/enable
```

Nach dem Aktivieren der Events, können diese z.B. `Trace-Log` Aufgezeichnet oder anderweitig ausgegeben werden.



## Probes



### Kernel-Probes

`kprobes`[@kprobes] können dazu verwendet werden, Laufzeit und Performance-Daten des Kernels zu sammeln.
Der Vorteil dieser ist, dass Daten ohne Unterbrechung der Ausführung auf CPU-Instruktions-Ebene aggregiert werden können, anders als bei dem Debuggen eines Programms mittels Breakpoints.
Ein weiterer Vorteil ist, dass das Registrieren der Kprobes dynamisch zur Laufzeit und ohne Änderungen des Programmcodes geschieht.
Somit ist es möglich, zu verschiedenen Laufzeiten des zu analysierenden Systems oder Programms, Daten zu verschiedenen Laufzeiten gezielt sammeln zu können.

Der `kretprobes`[@kretprobes] ermöglicht uns auf den Rückgabewert jeder Kernel- oder Modulfunktion zuzugreifen! Die Möglichkeit, den Rückgabewert einer bestimmten Funktion dynamisch nachzuschlagen, kann in einem Debug-Szenario ein entscheidender Vorteil sein.

### User-Level-Probes

Eine Weiterentwicklung zu den `kprobes` sind die `uprobes`. Mit diesem können zur Laufzeit Events in eine Applikation eingebunden werden. 
Wenn ein `uprobes` hinzugefügt werden soll, muss davor noch was gemacht werden. 

```c++
//test.c
#include <stdio.h>
int main(void)
{
    int i;
    for (i = 0; i < 5; i++)
        printf("Hello uprobe\n");
    return 0;
}
```

Bei der Nutzung von `kprobes` kann ein einfacher Symbolnamen spezifiziert werden. Aufgrund der Tastsache, dass alle Applikationen ihren eigenen virtuellen Adressraum besitzen, haben diese auch einen anderen Adressbasis. Beim Erzeugen eines `uprobes` wird das Adressoffset im Textsegment der jeweiligen Applikation benötigt.
Der obere C++-Code, stellt ein einfaches Beispiel dar, indem die `printf`-Anweisung, mittels einer `uprobe` aufgezeichnet werden soll.
Der Adressoffset kann mittels `objdump` und dem Pfad des zu analysierenden Programms.
Danach kann die `uprobe` im Debuf-Filesystem registriert werden unter Angabe des Offsets.
Als letzter Schritt, muss das neu erstellte `uprobe`-Event noch aktiviert werden und die Aufzeichnung oder Ausgabe der `uprobe`.

```bash
# CREATE EXECUTE OBJECT
$ gcc ./test.c -o ./tmp/test
# GET OFFSET
$ objdump -F -S -D ./test | less | grep main
0000000000001149 <main> (File Offset: 0x1149):
# REGISTER A uprobe_event
$ echo "p:my_uprobe /tmp/test:0x1149" > /sys/kernel/debug/Tracing/uprobe_events
# ACTIVATE UPROBE EVENTS
$ echo 1 > /sys/kernel/Tracing/events/uprobes/enable
# EXECUTE PROGRAM
$ /root/hello
Hello uprobe
[...]
# PRINT TRACED EVENTS
$ cat /sys/kernel/debug/Tracing/trace
# tracer: nop
# TASK-PID  CPU#  TIMESTAMP  FUNCTION
#  |   |     |        |         |
test-24842 [006] 258544.995456: printf: [...]
[...]
```

Ein weiterer Anwendungsfall ist die Inspektion von System-Bibliotheken.

## Ressourcen

Beim Tracing werden zusätzliche Ressourcen benötigt, die Auswirkungen auf die reale Ausführzeit haben. Bei Realtime OS können diese zu Problemen führen, wenn diese bereits mit den maximalen Ressourcen arbeitet.

* welche Effekte können entstehen
* Tracing braucht Ressourcen
* last minimieren auf traget minimieren
* nur aufzeichnen und später analysieren z.B. auf einem anderen System
* wie verhindern
* Große Datei Größen bei langen logs

# Tools

Allgemein sind keine speziellen Programme notwendig, um die Laufzeiteigenschaften eines Programms aufzuzeichnen.
Der Linux-Kernel bringt bereits alle nötigen Funktionalitäten mit, jedoch gibt es Tools, welche eine visuelle Darstellung der aufgezeichneten Events ermöglichen.
Somit kann die Aufzeichnung headless auf dem Ziel-System geschehen und die spätere Analyse mit entsprechenden Tools auf einem anderen System erfolgen.


## Trace-Log Aufzeichnung

Für die Log-Aufzeichnung wird der zuvor beschriebene Ringbuffer genutzt. Das Aufzeichnen in den Ringpuffer ist standardmäßig aktiviert. 
Kann aber bei Bedarf deaktiviert werden.
```bash
$ echo 1 > Tracing on
$ echo 0 > Tracing on
```

Mit dem folgenden Befehl kann der Inhalt des Ringbuffers auch während einer Aufzeichnung, ausgebeben werden.
Somit sind im Allgemeinen keine besonderen Tools notwendig. Anwendungen zum Ausgeben von Dateien wie z.B.`cat` oder `less`, welche sich auch auf kleinen Systemen befinden, sind ausreichend.\ref{trace-log} 
```bash
$ less /sys/kernel/Tracing/trace
 
```
Das Lesen während einer Aufzeichnung mittels Trace hat keinerlei Einfluss auf den Inhalt des Ringbuffers.

![Trace-Log \label{trace-log}](images/trace-log-print.png)

Die bisherigen Aufzeichnungen der Ereignisse können mit dem Leeren der `trace`-Datei entfernt werden:

```bash
$ echo > trace
```
Um einen Überlauf an Informationen zu verhindern kann die Aufzeichnung auch konsumierend gelesen werden. Somit werden beim Lesen zeitgleich diese aus dem Ringbuffer entfernt.

Ein weiterer Kernpunkt ist, dass in Mehrkernsystemen für jeden einzelnen Core ein separater Ringbuffer existiert. Damit die Analyse von verschiedenen Events getrennt werden kann, kann mit jeder weiteren Instanz pro Core ein weiterer Ringbuffer angelegt werden. Dies erfolgt im Untervereziechnis `instances/`.
Das Debug-Filesystem legt nach dem Anlegen des Ordners, die benötigten Dateien wie die `trace`-Datei automatisch an. Alle weiteren Operationen können dann auch auf dieser ausgeführt werden.

```bash
$ cd  /sys/kernel/Tracing/instances
$ mkdir ./inst0
$ mkdir ./inst1
```



### trace-cmd

![trace-cmd Report \label{trace-cmd-report}](images/trace-cmd.png)

Das Tool `trace-cmd`[@trace-cmd] ist das bekannteste und meistgenutzte Hilfsmittel zur Aufzeichnung. Dies ist ein Kommandozeilenwerkzeug, das auf den meisten gängigen Linux-Distributionen bereits vorinstalliert ist.

Mit dem letzten Befehl werden alle Events vom Scheduler aufgezeichnet. Dabei werden während der Aufzeichnung kontinuierlich die Ringbuffer in konsumierender Form ausgelesen und in die Datei `trace.dat` geschrieben, falls mit dem `-o` keine eigene Datei eingegeben wurde. Als Informationen werden zu dem Inhalt des Ringbuffers auch zusätzlich notwendige Informationen über das Target, für die Auswertung auf beliebigen Systemen gespeichert.

```bash
# CHECK IF TRACING IS ENABLED
$ sudo mount | grep tracefs
none on /sys/kernel/Tracing type tracefs (rw,relatime,seclabel)
## ONLY SCHEDULER EVENTS
$ echo sched_wakeup >> /sys/kernel/debug/Tracing/set_event
## ALL EVENTS USING set_event
$ echo *:* > /sys/kernel/debug/Tracing/set_event
# RECORD
$ trace-cmd record ./program_executable
# RECORD SPECIFIC EVENT
$ trace-cmd record -e sched ./program_executable
# USING A ADDITIONAL TRACER
$ trace-cmd -t function ./program_executable
```

Die `trace-cmd` Konsolenanwendung dient nicht nur zur Aufzeichnung der Trace-Events, sondern bietet auch die Möglichkeit aufgezeichnete Reports visuell darzustellen.
Die Ausgabe erfolgt mit dem Befehl `trace-cmd report [-i <Dateiname>]` als Tabelle in der Konsole und ist somit rein Textbasiert\ref{trace-cmd-report}.
Auf Aufzeichnungen können zusätzliche Filter angewendet werden, um die Suche auf bestimmten Ereignissen einzugrenzen.
Mit dem Tool ist es einfach die Teilschritte zu automatisieren.

### bpftrace

Seit der Kernelversion `>4.x`, kann ein weiteres Tool mit dem Namen `bpftrace`[@bpftrace] verwendet werden.
Dieses bietet jedoch zusätzlich eine eigene Skriptsprache, mit welcher nicht nur Aggregation, sondern auch die Eventfilter und die Verarbeitung der Ergebnisse automatisiert werden können.

```bash
# Block I/O latency as a histogram EXAMPLE
$ wget https://raw.githubusercontent.com/iovisor/bpftrace/master/tools/biolatency.bt
$ bftrace ./biolatency.bt
@usecs:
[512, 1K)             10 |@                       |
[ 1K, 2K)            426 |@@@@@@@@@@@@@@@@@@      |
[2K, 4K)             230 |@@@@@@@@@@@@@@          |
[4K, 8K)               9 |@                       |
[8K, 16K)            128 |@@@@@@@@@@@@@@@         |
[16K, 32K)            68 |@@@@@@@@                |
[...]
```


### Kernelshark

Das zuvor vorgestellte `tace-cmd` ist wie oben erwähnt nur ein textbasiertes Analysetool.
Kernelshark Tool bietet dem Anwender die Möglichkeit, die Trace-Aufzeichnungen grafisch zu analysieren. Dabei sind die beiden Tools aufeinander abgestimmt und werden gemeinsam entwickelt.
Auch dieses Tool ist in den meisten Linux Distributionen vorinstalliert.

Das vom trace-cmd erzeugte `trace.dat-Format` wird im Kernelshark als Eingabe erwartet. Wenn im folgendem ersten Befehl nichts eingegeben, dann wird nach der entsprechenden `trace.dat` im Verzeichnis gesucht.

```bash
# OPEN KERNELSHARK WITH trace.dat
$ kernelshark
# OPEN KERNELSHAR WITH SPECIFIED TRACELOG
$ kernelshark -i <Dateiname>
```

Im Folgenden ist die grafische Darstellung\ref{kernelshark} zu sehen. Dabei besitzt jeder Task einen eigenen Farbton. Für jede CPU wird eine eigene Zeile dargestellt.

![Kernelshark \label{kernelshark}](images/kernelshark.png)




# Beispiel - TCP Paketanalyse

Dieses Beispiel zeigt, wie der Empfang von TCP-Netzwerkpaketen auf Paketverlust auf einem System überprüft werden kann.
Hierbei soll analysiert werden, wie das System auf eine unerwartet große Menge an TCP-Paketen reagiert.
Dies kann zum Beispiel bei IOT-Anwendungen der Fall sein, bei denen das MQTT-Protokoll verwendet wird.
Hierbei können viele kleine Netzwerkpakete von IOT-Sensoren einen starken Traffics am Server zur Folge haben.

## bpftrace Installation
Dabei wird auf dem zu analysierenden System `bpftrace`[@bpftrace] verwendet. Unter Debian-Systemen kann dies einfach über den APT-Package-Manager installiert werden, jedoch ist diese Version, welche in der Registry hinterlegt ist, meist nicht aktuell.
Das folgende Beispiel erfordert die Version `>= 0.14`.
Somit muss `bpftrace` aus den Quellen gebaut werden, da in der APT-Registry nur die Version `~0.11` zur Verfügung stand.

```bash
# INSTALL FROM SOURCE
$ git clone https://github.com/iovisor/bpftrace ./bpftrace
$ cd ./bpftrace && mkdir -p build
$ cmake -DCMAKE_BUILD_TYPE=Release . && make -j20
$ sudo make install
# GET TCP DROP EXAMPLE
$ cp ./bpftrace/tools/tcpdrop.bt ~/
```

## TCPDROP.BT

Das `tcpdrop.bt` Skript, welches in diesem Beispiel verwendet wird, registriert eine `kprobe` auf die `tcp_drop()` Funktion und 
nutzt anschließend `printf` Funktion, um die Informationen in den Userspace zu loggen.

```c++
// tcpdrop.bt - SIMPLIFIED
kprobe:tcp_drop
{
    // GET SOCKET INFORMATION
    $sk = ((struct sock *) arg0);
    $inet_family = $sk->__sk_common.skc_family;
    //ADRESSES
    $daddr = ntop($sk->__sk_common.skc_daddr);
    $saddr = ntop($sk->__sk_common.skc_rcv_saddr);
    // PORTS
    $dport = $sk->__sk_common.skc_dport;
    $dport = $sk->__sk_common.skc_dport;
    //LOG INTO USERSPACE
    printf("%39s:%-6d %39s:%-6d %-10s\n", $saddr, $lport, $daddr, $dport, $statestr);
}
```

Um eine Lastspitze auf dem System zu erzeugen, wurde das Netzwerkbenchmark-Tool `ntttcp`[@ntttcp] verwendet. Mit diesem ist es möglich, UDP und TCP Pakete mit verschiedenen Paketgrößen zu generieren.
Hierzu werden zwei Instanzen benötigt, der Server und der Client, welche auf dem gleichen System aber auch auf verschiedenen Systemen ausgeführt werden können.

## Aufzeichnung Trace-Log

Um die Messung zu starten, wurde zuerst der `ntttcp`-Server gestartet; dieser empfängt die vom Sender gesendeten Pakete.
Im zweiten Schritt wurde der `ntttcp`-Client auf dem anderen System gestartet. Hier wurde mittels `-t` Parameter die Laufzeit auf unendlich gestellt, somit werden durchgehend Pakete an den Server gesendet. Die Paketgröße wurde hier auf `4096Kbyte` gestellt umso eine Fragmentierung des TCP-Paketes bei einer MTU von `1500byte` zu erzwingen.

Im Anschluss wurde `bpftrace` gestartet, welches die Events als Logdatei `tcpdrop_log` in einem lesbaren Textformat ausgeben soll.

```bash
# START SERVER
$ ntttcp -r
NTTTCP for Linux 1.4.0
---------------------------------------------------------
21:27:58 INFO: 17 threads created


# RUN bpftrace RECORD
$ sudo bpftrace -o ~/tcpdrop_log -f text -v ~/tcpdrop.bt 
INFO: node count: 171
Program ID: 146
The verifier log: 
processed 374 insns (limit 1000000) max_states_per_insn 0
Attaching BEGIN
[...]

# START CLIENT # PACKET SIZE 16Byte
$ ntttcp -s10.11.12.1 -t -l 16
NTTTCP for Linux 1.4.0
---------------------------------------------------------
21:28:52 INFO: running test in continuous mode.
21:28:52 INFO: 64 threads created
21:28:52 INFO: 64 connections created in 5656 microseconds
21:28:52 INFO: Network activity progressing...
```

Nach einigen Sekunden wurde `ntttcp` und `bpftrace` die Aufzeichnung manuell gestoppt.
Das aufgezeichnete Trace für das `tcp_drop`-Event befindet sich in der `tcpdrop_log` Datei.

## Ausgabe

Die Ausgabe der Logdatei stellt Textbasiert nicht nur dar, ob ein TCP-Paket verloren wurde, sondern gibt auch zusätzliche Informationen aus. Jeder Event-Trigger des `tcp_drop()` Events wird dabei mit der Systemzeit, Prozess-ID und dem Programm eingeleitet unter welches das Event ausgelöst hat. In diesem Fall wurde der Paketverlust durch ein Empfangenes Paket der `ntttcp`-Anwendung ausgelöst.
Die Senderichtung des Pakets kann anhand der Quell- und Empfangs-IP-Adresse ermittelt werden.
Danach folgt der Kernel-Stacktrace, in welchem der Funktionsaufruf-Verlauf bis zum Auslösen des überwachten Events aufgeführt ist.

```bash
$ cat ~/tcpdrop_log
[..]
# tcp_drop() TIME   PID  APPLICATION    SOURCE  DESTINATION
21:36:57 18157 ntttcp 10.11.12.1:5014 10.11.12.2:59012
        #CALLSTACK
        # LAST FUNCTION CALL
        tcp_drop+1
        tcp_v4_do_rcv+196
        __release_sock+120
        __tcp_close+444
        tcp_close+37
        inet_release+72
        __sock_release+66
        sock_close+21
        __fput+156
        ____fput+14
        task_work_run+112
        exit_to_user_mode_prepare+437
        syscall_exit_to_user_mode+39
        do_syscall_64+110
        entry_SYSCALL_64_after_hwframe+68
        # FIRST FUNCTION CALL
[...]
```

Somit ist aus den Logs zu entnehmen, dass unter den getesteten Bedingungen auf dem System TCP Pakete verloren gingen, eine tiefergehende Untersuchung des Kernel-Stacktrace kann hierzu genauere Informationen bereitstellen.
Das Beispiel zeigt auch, dass nicht nur das Auslösen von Events protokolliert werden kann, sondern auch mittels einfacher Skript-Befehle komplexe Debug-Informationen systematisch gewonnen werden können.



# Beispiel - Identifikation von Laufzeitproblemen

In diesem Abschnitt soll an einem einfachen Beispiel gezeigt werden, wie es mittels Tracing möglich ist, eine Laufzeitanalyse auf verschiedenen Systemen für eine Anwendung durchzuführen.


## Ausgangsszenario

Als Ausgangspunkt dieses Beispiels, soll das Laufzeitverhalten eines Programms auf einem Linux-System analysiert werden.
Die zugrunde liegende Software wurde bisher nur auf einem Linux-Realtime Kernel verwendet,
jedoch erfordert die Implementation neuer Features eine neuere Kernel-Version, welche noch nicht als RT-Version auf dem System zur Verfügung steht.
Somit soll ermittelt werden, ob die nicht-modifizierte Software eins zu eins auf dem neuen System lauffähig ist und die Laufzeitanforderungen erfüllt.

Das System besteht hier aus einem `RaspberryPi 4B` mit einer angeschlossenen LED am GPIO-Port `24`
und zu testende Programm toggelt dabei den GPIO in einer Dauerschleife.

```c++
//gpio_test.cpp
#include <iostream>
#include <pigpio.h>
#include <csignal>
using namespace std;

volatile bool running = true;
const int GPIO = 24;
void signal_callback_handler(int signum) {
    running = false;
}

int main(int argc, char *argv[])
{
    //REGISTER SIGNAL HANDLER
    signal(SIGINT, signal_callback_handler);
  
    if (gpioInitialise() < 0){
        return -1;
    }
    gpioSetMode(GPIO, PI_OUTPUT);
    bool state = false;
    while(running){
        state = !state;
	    gpioWrite(GPIO, (int)state);
    }
	return 0;
}
```

Das Programm kann mittels `g++` Compiler für das Zielsystem übersetzt werden.
Da zur Ansteuerung des GPIO Ports die `pigpio` Bibliothek verwendet wurde, welche eine Alternative zur obsoltenen `WiringPi` Bibliothek darstellt, muss diese noch installiert werden.

```bash
# INSTALL PIGPIO LIB
$ git clone https://github.com/joan2937/pigpio.git ~/pigpio
$ cd ~/pigpio && make && sudo make install && cd ~
# COMPILE
$ g++ -Wall -pthread -o gpio_test gpio_test.cpp -lpigpio -lrt
# TEST
$ sudo ./gpio_test
```


## Kernel des Testsystems

Für den Test wurde als RT-Kernel die Version `4.19.59-rt23-v7l+` verwendet, welche nicht alle Funktionalitäten des aktuellen `5.10` Kernel besitzt.
In diesem fiktiven Beispiel wird die `systemd-networking >V.248` Funktionalität für das Batman-Protokoll benötigt, welche den Grund für die Umstellung darstellt und nicht trivial in den `4.x` Kernel integriert werden kann. Die Messungen wurden zuerst auf dem aktuellen `5.10 LTS` Kernel aufgezeichnet und im Anschluss wurde der RT-Kernel auf einem anderen System per Cross-Compilation[@rpi4rt] aus dem `rpi-4.19.y-rt` Branch des `raspberrypi/linux` Repository gebaut. Dieser Schritt war notwendig, da es kein fertiges RT-Kernel Image zur Verfügung stand.
Die erzeugten Dateien wurden dann auf die Boot-Partition der SD-Karte geschrieben und in der `/boot/config.txt` Datei wurde der neue Kernel installiert `kernel=kernel7_rt.img`.


## Aufzeichnung Trace-Log

Zur Aufzeichnung des Trace-Logs wurde `trace-cmd`[@trace-cmd] verwendet. Auf dem Zielsystem wurde dabei nur die Aufzeichnung vorgenommen und die Analyse der Logs erfolgte auf einem seperaten System.
Für den Test wird zuerst die `Tracing`-Funktionalität aktiviert und alle `sched` und `gpio`-Events aktiviert.

```bash
# ENABLE TRACING
$ echo 1 > /sys/kernel/debug/Tracing/Tracing_on
$ cat /sys/kernel/debug/Tracing/trace
# CLEAR RECENT EVENT LOG
$ echo > /sys/kernel/debug/Tracing/trace
# ENABLE ALL SCHEDULER EVENTS
echo 1 > /sys/kernel/debug/Tracing/events/sched/enable
# ENABLE ALL GPIO EVENTS
echo 1 > /sys/kernel/debug/Tracing/events/gpio/enable
# RUN TEST
sudo trace-cmd record -e sched -e gpio -o ./gpio_test_trace_lts ./gpio_test
```

Im Anschluss wurde das Programm gestartet und die Events mittels `trace-cmd` aufgezeichnet.
Da das Testprogramm nicht automatisch terminiert (wie z.B. `sleep 5`), muss dieses mittels `Ctl+C` manuell beendet werden.
Die resultierende Ausgabedatei `gpio_test_trace_*` enthält die von `trace-cmd` geloggten Daten.



## Auswertung


## Fazit
