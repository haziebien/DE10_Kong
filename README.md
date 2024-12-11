# Donkey Kong for the DE10
Donkey Kong styled game written in C for the Altera DE10_Lite using NIOS hardware
Accelerometer controlled player that has to reach the top of the map while avoiding moving objects
Implements Mario-styled movement using AABBs
Inspiration taken from Donkey Kong, Super Mario Bros, and Elden Ring




Model Used: 10M50DAF484C7G

Build Instructions
- Install Quartus 18.1

Quartus Setup
- Create New Project
- Import the provided .qsf and .sdc files
- Import .vhdl file and set as top level entity
- Device and Pin Options -> Single Uncompressed Image with Memory Initialization (512Kbits UFM)
- Use VHDL 2008
- Open Platform Designer and open provided .qsys file
- Generate HDL
- In Quartus, Add the generated .qip to the Quartus project directory in the sidebar
- Use Programmer to program to the DE10

NIOS II Setup
- Create a new directory for your project
- File -> New -> Application and BSP
- Import provided .sopcinfo file
- Generate BSP
- Right click project -> BSP Editor -> Include Small C Library and Reduced Drivers
- Add main.c
- Right click project -> Run as NIOS II Hardware
