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
So werden in der eingesetzten Ringbuffer-Implementierung die Debug-Informationen gespeichert und ein Auslesen dieser ist mittels der Einträge im Debug-(+fs) möglich.

## Debug-Filesystem

Das Debug-(+fs) wurde in der Kernel-Version `2.6.10-rc3`[@dbgfs] eingeführt. Es bietet Zugriff auf Diagnose und Debug-Informationen auf Kernel-Ebene.

Ein Vorteil gegenüber des Prozess-(+fs) `/proc` ist, dass jeder Entwickler hier auch eigene Daten zur späteren Diagnose einpflegen kann.
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

Durch das Debug-(+fs) ist der Zugriff auf die Debug und insbesondere auf die Tracing-Daten möglich.
Im Debug-(+fs) ist nach der Aktivierung die `Tracing`-Ordnerstruktur vorhanden.
In diesem werden verfügbaren Events in gruppiert in Ordnern dargestellt, auf welche im späteren Verlauf reagiert werden kann.
Zudem können hier auch die Verfügbaren `tracer` angezeigt und aktiviert werden, welche noch weitere Debugging-Optionen bereitstellen.


### Tracer

```bash
# GET TRACERS
$ cat /sys/kernel/debug/tracing/available_tracers
hwlat blk mmiotrace function_graph wakeup_dl wakeup_rt wakeup function nop
# USE SPECIFIC TRACER
$ echo function_graph > /sys/kernel/debug/tracing/current_tracer
# DISABLE TRACER USAGE
$ echo nop > /sys/kernel/debug/tracing/current_tracer
```

`tracer` sind zusätzliche Tracing-Tools, welche eine gezieltere Aggregierung von Events z.B. Filterung und somit tiefergehende Analyse erlauben.
Zum Beispiel erlaubt der `ftrace`-Tracer eine detaillierte Ereignis-Filterung auf spezifizierte Events[@ftraceintroducation].

Der `function_graph`-Tracer gibt bei Verwendung zusätzliche Informationen, wie z.B. die Laufzeit von einzelnen Funktionen[@fgtrace].

Auch kann dieser den Stacktrace und den Call-Stack übersichtlich darstellen, indem hier die Namen der aufgerufenen Funktionen ausgegeben werden. 

```bash
# CALL STACK USING FUNCTION_GRAPH TRACER
$ echo function_graph > /sys/kernel/debug/tracing/current_tracer
$ cat /sys/kernel/debug/tracing/trace
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

Ein Event kann zum Beispiel durch das Lesen und Schreiben auf den System (+i2c)-Bus vom Kernel ausgelöst werden. Wenn das Event im Debug-(+fs) aktiviert wurde, stellt dieses die Informationen des Events bereit. Je nach Typ können unterschiedlichste Informationen dem Nutzer bereitgestellt werden.
In der `ls`-Ausgabe des `events`-Ordners des Debug-(+fs) ist zu sehen, welche Events abgefangen und mittels der Linux-Tracing-Tools protokoliert werden können.

```bash
# GET AVAILABLE EVENT LIST
$ cd /sys/kernel/debug/tracing/events
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

Alle Events sind in Gruppen gebündelt. Alle Events, welche das `ext4`-(+fs) betreffen, befinden sich im `ext4`-Ordner.
Die Auflistung zeigt einige der für das `ext4` zur Verfügung stehenden Events.
Zudem befinden sich zwei zusätzliche Dateien `enable`, `filter `in diesem Ordner.
Durch diese ist es später möglich anzugeben, ob dieses Event aufgezeichnet werden soll.

```bash
$ cd /sys/kernel/debug/tracing/events/ext4
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
$  cat /sys/kernel/debug/tracing/events/sched/sched_wakeup/format
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
$ cd /sys/kernel/debug/tracing/events/ext4
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
Danach kann die `uprobe` im Debuf-(+fs) registriert werden unter Angabe des Offsets.
Als letzter Schritt, muss das neu erstellte `uprobe`-Event noch aktiviert werden und die Aufzeichnung oder Ausgabe der `uprobe`.

```bash
# CREATE EXECUTE OBJECT
$ gcc ./test.c -o ./tmp/test
# GET OFFSET
$ objdump -F -S -D ./test | less | grep main
0000000000001149 <main> (File Offset: 0x1149):
# REGISTER A uprobe_event
$ echo "p:my_uprobe /tmp/test:0x1149" > /sys/kernel/debug/tracing/uprobe_events
# ACTIVATE UPROBE EVENTS
$ echo 1 > /sys/kernel/tracing/events/uprobes/enable
# EXECUTE PROGRAM
$ /root/hello
Hello uprobe
[...]
# PRINT TRACED EVENTS
$ cat /sys/kernel/debug/tracing/trace
# tracer: nop
# TASK-PID  CPU#  TIMESTAMP  FUNCTION
#  |   |     |        |         |
test-24842 [006] 258544.995456: printf: [...]
[...]
```

Ein weiterer Anwendungsfall ist die Inspektion von System-Bibliotheken.

## Ressourcen

Beim Tracing werden zusätzliche Ressourcen benötigt, die Auswirkungen auf die reale Ausführzeit haben.
Bei Echtzeitbetriebsystemen können diese zu Problemen führen, wenn dieses bereits mit den maximalen Ressourcen arbeitet.

Die Aufzeichnung eines Trace-Logs benötigt je nach aktivierten Events, eine nicht unerhebliche Menge an Speicherplatz auf dem System.
Zusätzlich muss das Medium auf welchem die Logs gespeichert werden sollen, ein Mindestmaß an Bandbreite zur verfügung stellen.

Wird zusätzlich die Auswertung auf dem zu analysierenden Gerät durchgeführt, benötigt diese weitere Ressourcen und kann somit den Betrieb und die Aufzeichnung beeinflussen.

Um diese nachteiligen Effekte zu minimieren, sollte die Auswertung auf einem seperaten Gerät durchgeführt werden.
Vor der Aufzeichnung sollen nur die Events aktiviert werden, welche im Fokus der analyse stehen.
Somit kann Bandbreite und Speicherplatz verringert werden, welche die zu analysierende Anwendung ggf. selbst benötigt.

# Tools

Allgemein sind keine speziellen Programme notwendig, um die Laufzeiteigenschaften eines Programms aufzuzeichnen.
Der Linux-Kernel bringt bereits alle nötigen Funktionalitäten mit, jedoch gibt es Tools, welche eine visuelle Darstellung der aufgezeichneten Events ermöglichen.
Somit kann die Aufzeichnung headless auf dem Ziel-System geschehen und die spätere Analyse mit entsprechenden Tools auf einem anderen System erfolgen.


## Trace-Log Aufzeichnung

![Trace-Log \label{trace-log}](images/trace-log-print.png)


Für die Log-Aufzeichnung wird der zuvor beschriebene Ringbuffer genutzt. Das Aufzeichnen in den Ringpuffer ist standardmäßig aktiviert. 
Kann aber bei Bedarf deaktiviert werden.

```bash
# ENABLE TRACING
$ echo 1 > tracing_on
# DISBALE TRACING
$ echo 0 > tracing_on
```

Mit dem folgenden Befehl kann der Inhalt des Ringbuffers auch während einer Aufzeichnung, ausgebeben werden.
Somit sind im Allgemeinen keine besonderen Tools notwendig. Anwendungen zum Ausgeben von Dateien wie z.B.`cat` oder `less`, welche sich auch auf kleinen Systemen befinden, sind ausreichend.\ref{trace-log} 

```bash
$ less /sys/kernel/tracing/trace
```
Das Lesen während einer Aufzeichnung mittels Trace hat keinerlei Einfluss auf den Inhalt des Ringbuffers.



Die bisherigen Aufzeichnungen der Ereignisse können mit dem Leeren der `trace`-Datei entfernt werden:

```bash
$ echo ''> trace
```
Um einen Überlauf an Informationen zu verhindern kann die Aufzeichnung auch konsumierend gelesen werden. Somit werden beim Lesen zeitgleich diese aus dem Ringbuffer entfernt.



Ein weiterer Kernpunkt ist, dass in Mehrkernsystemen für jeden einzelnen Core ein separater Ringbuffer existiert. Damit die Analyse von verschiedenen Events getrennt werden kann, kann mit jeder weiteren Instanz pro Core ein weiterer Ringbuffer angelegt werden. Dies erfolgt im Untervereziechnis `instances/`.
Das Debug-(+fs) legt nach dem Anlegen des Ordners, die benötigten Dateien wie die `trace`-Datei automatisch an. Alle weiteren Operationen können dann auch auf dieser ausgeführt werden.

```bash
$ cd  /sys/kernel/tracing/instances
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
none on /sys/kernel/tracing type tracefs (rw,relatime,seclabel)
## ONLY SCHEDULER EVENTS
$ echo sched_wakeup >> /sys/kernel/debug/tracing/set_event
## ALL EVENTS USING set_event
$ echo *:* > /sys/kernel/debug/tracing/set_event
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

Dieses Beispiel zeigt, wie der Empfang von (+tcp)-Netzwerkpaketen auf Paketverlust auf einem System überprüft werden kann.
Hierbei soll analysiert werden, wie das System auf eine unerwartet große Menge an (+tcp)-Paketen reagiert.
Dies kann zum Beispiel bei (+iot)-Anwendungen der Fall sein, bei denen das (+mqtt)-Protokoll verwendet wird.
Hierbei können viele kleine Netzwerkpakete von (+iot)-Sensoren einen starken Traffics am Server zur Folge haben.

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

Um eine Lastspitze auf dem System zu erzeugen, wurde das Netzwerkbenchmark-Tool `ntttcp`[@ntttcp] verwendet. Mit diesem ist es möglich, UDP und (+tcp) Pakete mit verschiedenen Paketgrößen zu generieren.
Hierzu werden zwei Instanzen benötigt, der Server und der Client, welche auf dem gleichen System aber auch auf verschiedenen Systemen ausgeführt werden können.

## Aufzeichnung Trace-Log

Um die Messung zu starten, wurde zuerst der `ntttcp`-Server gestartet; dieser empfängt die vom Sender gesendeten Pakete.
Im zweiten Schritt wurde der `ntttcp`-Client auf dem anderen System gestartet. Hier wurde mittels `-t` Parameter die Laufzeit auf unendlich gestellt, somit werden durchgehend Pakete an den Server gesendet. Die Paketgröße wurde hier auf `4096Kbyte` gestellt umso eine Fragmentierung des (+tcp)-Paketes bei einer MTU von `1500byte` zu erzwingen.

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

Die Ausgabe der Logdatei stellt Textbasiert nicht nur dar, ob ein (+tcp)-Paket verloren wurde, sondern gibt auch zusätzliche Informationen aus. Jeder Event-Trigger des `tcp_drop()` Events wird dabei mit der Systemzeit, Prozess-ID und dem Programm eingeleitet unter welches das Event ausgelöst hat. In diesem Fall wurde der Paketverlust durch ein Empfangenes Paket der `ntttcp`-Anwendung ausgelöst.
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

Somit ist aus den Logs zu entnehmen, dass unter den getesteten Bedingungen auf dem System (+tcp)-Pakete verloren gingen, eine tiefergehende Untersuchung des Kernel-Stacktrace kann hierzu genauere Informationen bereitstellen.
Das Beispiel zeigt auch, dass nicht nur das Auslösen von Events protokolliert werden kann, sondern auch mittels einfacher Skript-Befehle komplexe Debug-Informationen systematisch gewonnen werden können.



# Beispiel - Identifikation von Laufzeitproblemen

In diesem Abschnitt soll an einem einfachen Beispiel gezeigt werden, wie es mittels Tracing möglich ist, eine Analyse der Nachrichten auf einem (+i2c)-Bus durchzuführen.

## Ausgangsszenario

Als Ausgabgspunkt für dieses Beispiel, kommuniziert ein Eingebettetes-System mit einem Sensor über den (+i2c)-Bus.
Das Programm welches mit dem Sensor kommuniziert ist eine Black-Box. Somit steht kein Quellcode zur Verfügung.
Hier soll das Protokoll analysiert werden, um dieses in einer späteren Anwendung nachbauen zu können.
Auch gibt es hier keinen Zugriff auf die elektrische Ebene des Bus-Systems, somit kann kein Logic-Analyzer verwendet werden.

## Aufzeichnung Trace-Log

Zur Aufzeichnung des Trace-Logs wurde `trace-cmd`[@trace-cmd] verwendet. Auf dem Zielsystem wurde dabei nur die Aufzeichnung vorgenommen und die Analyse der Logs erfolgte auf einem seperaten System.
Für den Test wird zuerst die `Tracing`-Funktionalität aktiviert und alle `sched` und `gpio`-Events aktiviert.

```bash
# DANGER: RUN ALL NEXT COMMANDS AS ROOT
$ sudo su
# ACTIVATE DEBUG FS
$ mount -t debugfs none /sys/kernel/debug
# USE NOP TRACER
$ echo nop > current_tracer
# CLEAR RECENT EVENT LOG
$ echo > /sys/kernel/debug/tracing/trace
# ENABLE ALL I2C-BUS EVENTS
echo 1 > /sys/kernel/debug/tracing/events/i2c/enable
# ENABLE TRACING
$ echo 1 > /sys/kernel/debug/tracing/tracing_on
```

Nachdem das Tracing aktiviert wurde, wurde das Trace-Log manuell analysiert.

```bash
$ cat /sys/kernel/debug/tracing/trace
# tracer: nop
#
# entries-in-buffer/entries-written: 96/96 
#           _-----=> irqs-off
#          / _----=> need-resched
#         | / _---=> hardirq/softirq
#         || / _--=> preempt-depth
#         ||| / _-=> migrate-disable
#         |||| /     delay
#TASK-PID |||||  TIMESTAMP  FUNCTION
# |   |   |||||     |         |
7127  987.236090: i2c_write: i2c-1 #0 a=020 f=0000 l=1 [01]
7127  987.236656: i2c_result: i2c-1 n=1 ret=1
7127  987.737266: i2c_write: i2c-1 #0 a=020 f=0000 l=1 [02]
7127  987.737827: i2c_result: i2c-1 n=1 ret=1
7127  988.238418: i2c_write: i2c-1 #0 a=020 f=0000 l=1 [04]
7127  988.238977: i2c_result: i2c-1 n=1 ret=1
7127  988.739545: i2c_write: i2c-1 #0 a=020 f=0000 l=1 [08]
7127  988.740132: i2c_result: i2c-1 n=1 ret=1
[...]

```

## Auswertung

Das Tracelog zeigt nun einige Events vom Typ `i2c_write`. Diese werden für die Analyse benötigt, da hier die vom System über den (+i2c)-Bus gesendeten Nachrichten stehen. Zu sehen ist, dass über den `i2c-1` Bus des Systems gesendet wird. Die Zieladresse ist dabei `0x20` und wird im Log mit dem Präfix `a=` angegeben. Die Länge der gesendeten Bytes ist mit `l=1` angegeben. Somit wurde nur ein Byte an den Slave gesendet. Die eigentlichen Daten sind in Hex-Array-Schreibweise angegeben. Hier wurde nacheinander `[01]`, `[02]`, `[04]`, `[08]` gesendet.[@bussnooping]

Die korrektheit der Adresse kann zusätzlich mittels des `i2cdetect`-Befehls überprüft werden. Die Ausgabe zeigt, dass am System nur ein (+i2c)-Slave mit der Adresse `0x20` angeschlossen ist, welches somit mit der Ausgabe im Trace-Log übereinstimmt.

```bash
$ i2cdetect -y 1
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:                         -- -- -- -- -- -- -- -- 
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
20: 20 -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
```
Anhand der (+i2c)-Adresse und der einfachheit der gesendeten Daten, kann auf einen `PCF8574` Port-Expander-(+ic) geschlossen werden.
Dieser nimmt jeweils an Adresse `0x20` ein Byte entgegen und schaltet somit die jeweiligen Ausgangspins.
Der Quellcode bestätigt diese Erkenntnisse ebenfalls. Die in Python geschriebene "Black-Box" verwendet das `smbus`-Modul um auf den (+i2c)-Bus-1 zuzugreifen und sendet in einer Dauerschleife jeweils ein Byte.


```python
#!/bin/env python3
import smbus
import time
# USE I2C-BUS 1
bus = smbus.SMBus(1)
 
while True:
	bus.write_byte(0x20, 0x01)
	time.sleep(0.5)
	bus.write_byte(0x20, 0x02)
	time.sleep(0.5)
	bus.write_byte(0x20, 0x04)
	time.sleep(0.5)
	bus.write_byte(0x20, 0x08)
	time.sleep(0.5)
```

Somit konnte das Protokoll des (+i2c)-Slave mittels Tracing ohne Kenntnis der Software analysiert und reverse engineered werden.


