# An Experimental OS

Building an OS to learn about how an OS works

# Building

The build system is closely based on the build system of nanobyte_os:
https://github.com/nanobyte-dev/nanobyte_os/

Install the project dependencies:

```sh
sudo apt install python3 python3-pip python3-parted scons dosfstools \
                 wget libguestfs-tools qemu-system-x86 nasm mtools guestmount
```

It is then recommended to create a python virtual environment:

```sh
python3 -m venv <env_name>
<env_name>/bin/activate
```

Install the required python packages with `python3 -m pip install -r requirements.txt`

Make sure to set the path to the toolchain directory in `build_scripts/config.py`.
By default it is set to `toolchain='../.toolchains/'` which is a directory
called .toolchains in the directory above the project directory. The toolchain
directory must exist or an error will occur.

Then run `scons toolchain` to download and install the required build tools.

Finally run `scons run` to build the OS and test it with qemu. On my system I
occasionally get a libguestfs error. If this occurs you can run `sudo chmod +r /boot/vmlinuz-*`
to allow guestmount to create a loopback device for creating an OS disk image

