# inverse-OOTF
Reverses the OOTF created by BT. 1886 and BT. 2100 in SDR and HDR video respectively.

## Compile Instructions
To compile each, run:
```
gcc -lm -O2 -fwhole-program -march=native -o change1886 change1886.c
gcc -lm -O2 -fwhole-program -march=native -o change2100 change2100.c
```
