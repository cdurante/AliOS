## AliOS

AliOS is a Real-time Operating System (or attempts to be real-time) for
embedded devices.

   - BSD license (your changes are yours)
   - supports dynamic ticks for low power applications
   - SMP (multicore) support
   - lwIP support
   - ROMFS support
   - Simple web server supporting dynamic pages
   - simple/clean/documented API
   - very minimal ROM and RAM requirements
   - does not need dynamic memory
   - easy to port

Ports for the following are available.

   - AVR (only tested with Atmega1280)
   - ARM (only tested with QEMU versatilepb & vexpress-a9)

Current work...

   - Renesas RX port (eval board)

Future work...

   - embedded Python (yes, for real)
   - port dropbear (SSH)
   - more ports (Renesas RL)
