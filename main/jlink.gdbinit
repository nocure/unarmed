#init
#====
target remote localhost:2331
monitor speed 12000
monitor endian little

#monitor reset
monitor go
#monitor sleep 100
monitor halt
monitor waithalt 100
#monitor step


#run
#===
set remote memory-write-packet-size 1024
set remote memory-write-packet-size fixed
#set remote memory-read-packet-size 1024
#set remote memory-read-packet-size fixed
load

#until main

