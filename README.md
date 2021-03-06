# midi-interval-processor
for c++11

##  Usage
Run on one file:
```
make midiToInterval
./midiToInterval inputFileName 
```
Run with all tests:
```
docker build ./ --tag midiIntervalTests
```
Process midi data in a directory called ClassicalComposers, with the data grouped into directories by composer name:
```
bash processMIDICorpus.sh
```

## Citations for Test Data
Found at mutopiaproject.org

### Symphony No. 3 in Eb Major (Eroica)
https://www.mutopiaproject.org/cgibin/piece-info.cgi?id=249

Composer: L. V. Beethoven (1770–1827)

Maintainer: Jay Anderson    

Contact: horndude77 (at) gmail.com

Copyright: Creative Commons Attribution-ShareAlike 3.0 

### Die Zauberflöte (The Magic Flute) - No. 4 Arie
https://www.mutopiaproject.org/cgibin/piece-info.cgi?id=1655

Composer: W. A. Mozart (1756–1791)

Maintainer: Deborah Lowrey  

Contact: drlowrey (at) karmaresources.com

Copyright: Public Domain


