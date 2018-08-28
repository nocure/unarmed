#gdb
#===
#arm-elf-gdb

#Initialize:
#===========
target remote localhost:3333

#run:
#===
#monitor reset run_and_halt
#monitor wait 200
#monitor soft_reset_halt
monitor reset
monitor wait 200
monitor arm7_9 sw_bkpt enable
monitor arm7_9 force_hw_bkpt enable
monitor arm7_9 dcc_downloads enable
monitor arm7_9 fast_memory_access enable
#monitor wait 200
#monitor rbp *
monitor working_area 0 0x0 0x4000 nobackup 0
#mem 0x30000000 0x3ffffffc rw 32 cache
#mem 0x40000000 0x4ffffffc rw 32 cache
load
#until main
