#!/bin/bash
#
#	script to run sox and cserialMux so as to compare their outputs {spectrogram and time-domain plots}
#	
#	Brian Onang'o
#	Dec	2016
#
# For more info check out https://github.com/Surgbc/cserialMux


#compile the program
####Always assumes program is not compiled, which is not always the case.
#
make

#get the baseband carrier frequency
echo -n "Enter the baseband carrier frequency in Hz: "
read basefrequency

#get the passband carrier frequency
echo -n "Now enter the passband carrier frequency in Hz: "
read passfrequency

#get the file to modulate
echo -n "Now enter the file in tmp/ to use in modulation: "
read modfile

#get the length of the data to use in modulation
echo -n "How many bytes of the file do you want to use in modulation? "
read modlength

#get the modulation type
echo -n "What type of baseband modulation would you like to use? "
read modtype

#get the symbol rate
echo -n "How many symbols/period of baseband carrier? "
read symbolrate

#get the sampling rate
echo -n "What Sampling rate? "
read samplingrate

#get the modulation index
echo -n "What value for passband modulation index(beta)? "
read modindex

#cseriallMux will fail is tmp/ does not exist. So create it
mkdir -p tmp

#
#produce baseband carrier using sox
#
#
##calculate the signal duration
case "$modtype" in 
	"BPSK") bitspersymbol=1
	;;
	"BFSK") bitspersymbol=1
	;;
	"BASK") bitspersymbol=1
	;;
	"4QAM") bitspersymbol=2
	;;
	"8QAM") bitspersymbol=3
	;;
	"16QAM") bitspersymbol=4
	;;
	"32QAM") bitspersymbol=5
	;;
	"64QAM") bitspersymbol=6
	;;
	"128QAM") bitspersymbol=7
	;;
	"512QAM") bitspersymbol=8
	;;
	"1024QAM") bitspersymbol=9
	;;
	"2048QAM") bitspersymbol=10
	;;
	"BASE") bitspersymbol=1
	;;
	"PASS") bitspersymbol=1
	;;
	"SINGLT") bitspersymbol=1
	;;	
esac

wavfile="tmp/soxbasebandcarrier$basefrequency.wav"
imgfile0="tmp/soxbasebandcarrier$basefrequency-0.png"
imgfile1="tmp/soxbasebandcarrier$basefrequency-1.png"
datfile="tmp/soxbasebandcarrier$basefrequency.dat"
duration=$(echo "($modlength *8)/($bitspersymbol * $basefrequency)" | bc)
echo "Will create a signal of duration $duration seconds in tmp/soxbasebandcarrier$basefrequency.wav"
echo "sox -n -c1 tmp/soxbasebandcarrier$basefrequency.wav synth $duration sin $basefrequency"
sox -n -c1 $wavfile synth $duration sin $basefrequency

###produce baseband carrier spectrogram
range=$(echo "scale=4; 9*$basefrequency" | bc)
echo "Will produce spectrogram in $imgfile0 with range of $range"
#############################modify the range
echo "sox $wavfile -n rate $range spectrogram -l -o $imgfile0"
sox $wavfile -n rate $range spectrogram -l -o $imgfile0

###plot baseband carrier wave for 2 periods
twoperiods=$(echo "scale=4; 2.0/$basefrequency" | bc)
echo "Will produce time domain plot of baseband carrier upto $twoperiods seconds in $imgfile1"
sox $wavfile $datfile
gnuplot -p -e "set terminal png" -e "set output '$imgfile1'" -e "set yrange [-1:1]" -e "set xrange [0:$twoperiods]" -e "plot '$datfile' using 1:2 with lines"

echo "Deleting $datfile"
rm $datfile
echo "Deleting $wavfile"
rm $wavfile
#
#produce passband carrier using sox
#
#
wavfile="tmp/soxpassbandcarrier$passfrequency.wav"
imgfile0="tmp/soxpassbandcarrier$passfrequency-0.png"
imgfile1="tmp/soxpassbandcarrier$passfrequency-1.png"
datfile="tmp/soxpassbandcarrier$passfrequency.dat"
echo "Will create a signal of duration $duration seconds in tmp/soxpassbandcarrier$passfrequency.wav"
echo "sox -n -c1 $wavfile synth $duration sin $passfrequency"
sox -n -c1 $wavfile synth $duration sin $passfrequency

###produce passband carrier spectrogram
range=$(echo "scale=4; 3.1*$passfrequency" | bc)
echo "Will produce spectrogram in $imgfile0 with range of $range"
#############################modify the range
sox $wavfile -n rate $range spectrogram -l -o $imgfile0

###plot passband carrier wave for 2 periods
twoperiods=$(echo "scale=4; 2.0/$passfrequency" | bc)
echo "Will produce time domain plot of passband carrier upto $twoperiods seconds in $imgfile1"
sox $wavfile $datfile
gnuplot -p -e "set terminal png" -e "set output '$imgfile1'" -e "set yrange [-1:1]" -e "set xrange [0:$twoperiods]" -e "plot '$datfile' using 1:2 with lines"

echo "Deleting $datfile"
rm $datfile
echo "Deleting $wavfile"
rm $wavfile

#
#produce baseband carrier using cserialMux
#
### this produces tmp/rawout
echo "..."
rawfile="tmp/rawout"
wavfile="tmp/muxbasebandcarrier$basefrequency.wav"
imgfile0="tmp/muxbasebandcarrier$basefrequency-0.png"
imgfile1="tmp/muxbasebandcarrier$basefrequency-1.png"
datfile="tmp/muxbasebandcarrier$basefrequency.dat"
echo "Will now run cserialMux to produce $rawfile"

./cserialMux -f $modfile -b $basefrequency -p $passfrequency -l $modlength -m BASE -s $samplingrate -r $symbolrate -k $modindex
###produce wavfile
echo "Will now produce $wavfile from $rawfile"
sox -t raw -r $samplingrate -s -L -c1 -e float -b 32 $rawfile $wavfile

###produce baseband carrier spectrogram
range=$(echo "scale=4; 9*$basefrequency" | bc)
echo "Will produce spectrogram in $imgfile0 with range of $range"
#############################modify the range
sox $wavfile -n rate $range spectrogram -l -o $imgfile0

###plot baseband carrier wave for 2 periods
twoperiods=$(echo "scale=4; 2.0/$basefrequency" | bc)
echo "Will produce time domain plot of baseband carrier upto $twoperiods seconds in $imgfile1"
sox $wavfile $datfile
gnuplot -p -e "set terminal png" -e "set output '$imgfile1'" -e "set yrange [-1:1]" -e "set xrange [0:$twoperiods]" -e "plot '$datfile' using 1:2 with lines"

echo "Deleting $datfile"
rm $datfile
echo "Deleting $wavfile"
rm $wavfile
echo "Deleting $rawfile"
rm $rawfile
#
#produce passband carrier using cserialMux
### this produces tmp/rawout
#
echo "..."
rawfile="tmp/rawout"
wavfile="tmp/muxpassbandcarrier$passfrequency.wav"
imgfile0="tmp/muxpassbandcarrier$passfrequency-0.png"
imgfile1="tmp/muxpassbandcarrier$passfrequency-1.png"
datfile="tmp/muxpassbandcarrier$passfrequency.dat"
echo "Will again run cserialMux to produce $rawfile"

./cserialMux -f $modfile -b $basefrequency -p $passfrequency -l $modlength -m PASS -s $samplingrate -r $symbolrate -k $modindex
###produce wavfile
echo "Will now produce $wavfile from $rawfile"
sox -t raw -r $samplingrate -s -L -c1 -e float -b 32 $rawfile $wavfile

###produce passband carrier spectrogram
range=$(echo "scale=4; 3.1*$passfrequency" | bc)
echo "Will produce spectrogram in $imgfile0 with range of $range"
#############################modify the range
sox $wavfile -n rate $range spectrogram -l -o $imgfile0

###plot passband carrier wave for 2 periods
twoperiods=$(echo "scale=4; 2.0/$passfrequency" | bc)
echo "Will produce time domain plot of passband carrier upto $twoperiods seconds in $imgfile1"
sox $wavfile $datfile
gnuplot -p -e "set terminal png" -e "set output '$imgfile1'" -e "set yrange [-1:1]" -e "set xrange [0:$twoperiods]" -e "plot '$datfile' using 1:2 with lines"

echo "Deleting $datfile"
rm $datfile
echo "Deleting $wavfile"
rm $wavfile
echo "Deleting $rawfile"
rm $rawfile


#
##produce baseband modulated signal using cserialMux
#
#### this produces tmp/rawout
echo "..."
rawfile="tmp/rawout"
wavfile="tmp/muxbaseband$modtype$basefrequency.wav"
imgfile0="tmp/muxbaseband$modtype$basefrequency-0.png"
imgfile1="tmp/muxbaseband$modtype$basefrequency-1.png"
datfile="tmp/muxbaseband$modtype$basefrequency.dat"
echo "Will run cserialMux again to produce $rawfile"

./cserialMux -f $modfile -b $basefrequency -p $passfrequency -l $modlength -m $modtype -s $samplingrate -r $symbolrate -k $modindex -c BASE
###produce wavfile
echo "Will now produce $wavfile from $rawfile"
sox -t raw -r $samplingrate -s -L -c1 -e float -b 32 $rawfile $wavfile

###produce baseband modulated spectrogram
range=$(echo "scale=4; 9*$basefrequency" | bc)
echo "Will produce spectrogram in $imgfile0 with range of $range"
sox $wavfile -n rate $range spectrogram -l -o $imgfile0

###plot baseband modulated wave for 2 periods
twoperiods=$(echo "scale=4; 2.0/$basefrequency" | bc)
echo "Will produce time domain plot of baseband carrier upto $twoperiods seconds in $imgfile1"
sox $wavfile $datfile
gnuplot -p -e "set terminal png" -e "set output '$imgfile1'" -e "set yrange [-1:1]" -e "set xrange [0:$twoperiods]" -e "plot '$datfile' using 1:2 with lines"

echo "Deleting $datfile"
rm $datfile
echo "Deleting $wavfile"
rm $wavfile
echo "Deleting $rawfile"
rm $rawfile

#
##produce passband modulated signal using cserialMux
#
#### this produces tmp/rawout
echo "..."
rawfile="tmp/rawout"
wavfile="tmp/muxpassband$modtype$passfrequency.wav"
imgfile0="tmp/muxpassband$modtype$passfrequency-0.png"
imgfile1="tmp/muxpassband$modtype$passfrequency-1.png"
datfile="tmp/muxpassband$modtype$passfrequency.dat"
echo "Will run cserialMux again to produce $rawfile"

./cserialMux -f $modfile -b $basefrequency -p $passfrequency -l $modlength -m $modtype -s $samplingrate -r $symbolrate -k $modindex -c PASS
###produce wavfile
echo "Will now produce $wavfile from $rawfile"
sox -t raw -r $samplingrate -s -L -c1 -e float -b 32 $rawfile $wavfile

###produce passband modulated spectrogram
range=$(echo "scale=4; 3.1*$passfrequency" | bc)
echo "Will produce spectrogram in $imgfile0 with range of $range"
sox $wavfile -n rate $range spectrogram -l -o $imgfile0

###plot passband modulated wave for 2 periods
twoperiods=$(echo "scale=4; 2.0/$passfrequency" | bc)
echo "Will produce time domain plot of passband carrier upto $twoperiods seconds in $imgfile1"
sox $wavfile $datfile
gnuplot -p -e "set terminal png" -e "set output '$imgfile1'" -e "set yrange [-1:1]" -e "set xrange [0:$twoperiods]" -e "plot '$datfile' using 1:2 with lines"

echo "Deleting $datfile"
rm $datfile
echo "Deleting $wavfile"
rm $wavfile
echo "Deleting $rawfile"
rm $rawfile

#convert raw to wav
#sox -t raw -r 44100 -s -L -c1 -e float -b 32 tmp/rawout tmp/wavout.wav
