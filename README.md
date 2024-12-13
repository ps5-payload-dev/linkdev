# PS5 link device
This is a simple homebrew that enables local remote play on jailbroken PS5s
without having to access PSN. It uses the same approach as
[ps5-remoteplay-get-pin][idlesauce], but with a graphical user interface and does
not rely on the ptrace syscall.

## Prerequisites
A jailbroken PS5 that runs [ps5-payload-websrv][websrv] and has been rebooted
after activing the logged in user account using [OffAcct][offact].

## Quick-start
- Download [LinkDev][linkdev], decompress the zip file, and transfer its content
  to your PS5 at /data/homebrew/LinkDev
- Launch it according to the instructions from [ps5-payload-websrv][websrv],
  and pair your device using the parameters displayed on the screen.

## Reporting Bugs
If you encounter problems with linkdev, please [file a github issue][issues].
If you plan on sending pull requests which affect more than a few lines of code,
please file an issue before you start to work on you changes. This will allow us
to discuss the solution properly before you commit time and effort.

## License
linkdev is licensed under the GPLv3+.

[issues]: https://github.com/ps5-payload-dev/linkdev/issues/new
[idlesauce]: https://github.com/idlesauce/ps5-remoteplay-get-pin
[websrv]: https://github.com/ps5-payload-dev/websrv
[offact]: https://github.com/ps5-payload-dev/offact
[linkdev]: https://github.com/ps5-payload-dev/linkdev/releases/latest/download/LinkDev.zip
