a

/*



./cserialmux -f TrialFile.txt -b 0.159154943 -m  BASE -p 12.5 -l 1 -r 1 -k 0.5 -s 8|sox -b 32 -t raw -r 8 -s -L -c1 -e float -b 32 - "outpute.wav" &&  sox outpute.wav outpute.dat



./cserialmux -f TrialFile.txt -b 1591.54943 -m  BASE -p 12.5 -l 10 -r 1 -k 0.5 -s 44100|sox -b 32 -t raw -r 44100 -s -L -c1 -e float -b 32 - "outpute.wav" &&  sox outpute.wav outpute.dat && sox outpute.wav -n spectrogram



signal at 10 Khz = 591.54943 rad/s

./cserialmux -f TrialFile.txt -b 1591.54943 -m  BASE -p 12.5 -l 10000 -r 1 -k 0.5 -s 44100|sox -b 32 -t raw -r 44100 -s -L -c1 -e float -b 32 - "signal10k.wav" && sox signal10k.wav -n spectrogram -l -o signal10k.png



signal at 5KHz

./cserialmux -f TrialFile.txt -b 795.774715 -m  BASE -p 12.5 -l 10000 -r 1 -k 0.5 -s 44100|sox -b 32 -t raw -r 44100 -s -L -c1 -e float -b 32 - "signal5k.wav" && sox signal5k.wav -n spectrogram -l -o signal5k.png



signal at 15k

./cserialmux -f TrialFile.txt -b 2387.324145 -m  BASE -p 12.5 -l 10000 -r 1 -k 0.5 -s 44100|sox -b 32 -t raw -r 44100 -s -L -c1 -e float -b 32 - "signal15k.wav" && sox signal15k.wav -n spectrogram -l -o signal15k.png



carrier at 10k

./cserialmux -f TrialFile.txt -b 1591.54943 -m  PASS -p 1.59154943 -l 10000 -r 1 -k 0.5 -s 44100|sox -b 32 -t raw -r 44100 -s -L -c1 -e float -b 32 - "carrier10k.wav" && sox carrier10k.wav -n spectrogram -l -o carrier10k.png



carrier at 5KHz

./cserialmux -f TrialFile.txt -b 1 -m  PASS -p 0.795774715 -l 1 -r 1 -k 0.5 -s 44100|sox -b 32 -t raw -r 44100 -s -L -c1 -e float -b 32 - "carrier5k.wav" && sox carrier5k.wav -n spectrogram -l -o carrier5k.png



carrier at 15k

./cserialmux -f TrialFile.txt -b 1 -m  PASS -p 2.387324145 -l 1 -r 1 -k 0.5 -s 44100|sox -b 32 -t raw -r 44100 -s -L -c1 -e float -b 32 - "carrier15k.wav" && sox carrier15k.wav -n spectrogram -l -o carrier15k.png







./cserialmux -f TrialFile.txt -b 5 -m  BASE -p 10 -l 100 -r 1 -k 0.5 -s 44100|sox -b 32 -t raw -r 44100 -s -L -c1 -e float -b 32 - "ctemp.wav" && sox ctemp.wav -n rate 20 spectrogram -o ctemp.png





./cserialmux -f TrialFile.txt -b 2 -m  SINGLT -p 12 -l 100 -r 1 -k 0.2 -s 30|sox -b 32 -t raw -r 30 -s -L -c1 -e float -b 32 - "ctemp.wav" && sox ctemp.wav -n rate 40 spectrogram -o ctemp.png

sox -r 44100 -e unsigned -b 8 -c 1 <RAW_FILE> <TARGET_FILE>
sox -t raw -r 44100 -s -L -c1 -e float -b 32 - "ctemp.wav"


./cserialmux -f TrialFile.txt -b 2 -m  BASK -p 12 -l 100 -r 1 -k 0.2 -s 30|sox -b 32 -t raw -r 30 -s -L -c1 -e float -b 32 - "ctemp.wav" && sox ctemp.wav -n rate 40 spectrogram -o ctemp.png



./cserialmux -f TrialFile.txt -b 2 -m  BPSK -p 12 -l 100 -r 1 -k 0.2 -s 30|sox -b 32 -t raw -r 30 -s -L -c1 -e float -b 32 - "ctemp.wav" && sox ctemp.wav -n rate 40 spectrogram -o ctemp.png





./cserialmux -f TrialFile.txt -b 2 -m  BFSK -p 12 -l 100 -r 1 -k 0.2 -s 30|sox -b 32 -t raw -r 30 -s -L -c1 -e float -b 32 - "ctemp.wav" && sox ctemp.wav -n rate 40 spectrogram -o ctemp.png



./cserialmux -f TrialFile.txt -b 2 -m  4QAM -p 12 -l 10 -r 1 -k 0.2 -s 30|sox -b 32 -t raw -r 30 -s -L -c1 -e float -b 32 - "ctemp.wav" && sox ctemp.wav -n rate 40 spectrogram -o ctemp.png



-----------------

sox -n -r 48000 -c1 tmp.wav synth 1 sine 100 sine 200 && sox tmp.wav tmp.dat && sox tmp.wav -n rate 1000 spectrogram -o tmp.png



sox -n -r 48000 -c1 tmp1.wav synth 1 sine 100 && sox -n -r 48000 -c1 tmp2.wav synth 1 sine 200 && sox -m tmp1.wav tmp2.wav tmp.wav && sox tmp.wav tmp.dat && sox tmp.wav -n rate 1000 spectrogram -o tmp.png

---------------

gnuplot -p -e "plot 'tmp.dat' using 1:2"



gnuplot -p -e "set terminal png" -e "set output 'tmpx.png'" -e "set yrange [-1:1]" -e "set xrange [0:0.05]" -e "plot 'tmp.dat' using 1:2 with lines"



*/
