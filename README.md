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
        -u user : logging to /proc/dogdoor_log lastly ten files accessed by user_name
        -p pid  : prevent kill pid
```

### Preview

#### -u user

![render1579880372926](https://user-images.githubusercontent.com/16812446/73081875-bdd0f680-3f0b-11ea-82bf-c1550c7783a0.gif)

#### -p pid

![render1579881011406](https://user-images.githubusercontent.com/16812446/73082626-2ff60b00-3f0d-11ea-81c5-a526b84a6f99.gif)

