gcc -Wall -pg -g -std=c99 -g -IH:/GB/C/adv_dz2/curses -c main.c -o main.o
gcc -pg -no-pie -std=c99 -o snake.exe main.o H:/GB/C/adv_dz2/curses/wincon/pdcurses.a -IH:/GB/C/adv_dz2/curses
pause(0)