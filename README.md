# dogdoor
simple rootkit testing code

---

### usage

##### install rootkit

```
make
sudo insmod dogdoor.ko
```

##### uninstall

``` sudo rmmod dogdoor ```

##### make sure LKM(rootkit) is in place

``` lsmod | grep dogdoor ```

##### use bingo to utilize rootkit

``` ./bingo -h ```
