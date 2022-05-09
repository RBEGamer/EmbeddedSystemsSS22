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



## Debug-Filesystem

* möglichkeiten

## Trace Events

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

## Ausgangsszenario

Als Ausgangspunkt dieses Beispiels, soll das Laufzeitverhalten eines Programms auf einem Linux-System analysiert werden.
Die zugrunde liegende Software wurde bisher nur auf einem Linux-Realtime Kernel verwendet,
jedoch erfordert die Implementation neuer Features eine neuere Kernel-Version, welche noch nicht als RT-Version auf dem System zur Verfügung steht.
Somit soll ermittelt werden, ob die unmodifizierte Software eins zu eins auf dem neuen System lauffähig ist und die Laufzeitandorderungen erfüllt.


## Aufzeichnung mittels ftrace

## Visualisierung und Beurteilung des Trace-Logs mittels kernelshark


# Fazit