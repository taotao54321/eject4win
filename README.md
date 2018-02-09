# eject4win - eject(1) for Windows

[eject(1)](http://man7.org/linux/man-pages/man1/eject.1.html) for
Windows (not completely compatible). For now, this supports only
optical disk drive.

This should work on Windows NT or above (I tested on Windows 7 x64).
Windows 9x is not supported.


## Usage

```sh
$ eject     # open tray
$ eject -t  # close tray
$ eject -T  # toggle tray
$ eject X:  # open tray of drive X:
```


## Build

Install [MinGW](https://nuwen.net/mingw.html), and run `make`.


## References

* [How To Ejecting Removable Media in Windows NT/Windows 2000/Windows XP](https://support.microsoft.com/en-us/help/165721/how-to-ejecting-removable-media-in-windows-nt-windows-2000-windows-xp)
* [util-linux](https://github.com/karelzak/util-linux) (I stole the logic of tray toggling)
