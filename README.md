# dogdoor

simple rootkit testing code

**Only tested on ubuntu 16.04**

---

## Installation of rootkit dogdoor

```shell
make
sudo insmod dogdoor.ko
```

## uninstall

```shell
sudo rmmod dogdoor
```

## usage

### make sure LKM(rootkit) is in place

```shell
lsmod | grep dogdoor
```

### use bingo to utilize rootkit

```shell
$ ./bingo -h 
Usage: bingo [-u user | -p pid]
Options:
        -p pid  : prevent kill pid
                -u user : logging to /proc/dogdoor_log lastly ten files accessed by user_name
```
