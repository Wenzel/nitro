# nitro

Virtual Machine Introspection for KVM.

This is the userland component named `nitro`.
It will receive the events generated by KVM and display them.

# Requirements

- `python 3`
- `docopt`
- `libvirt`
- [`pebble`](https://pypi.python.org/pypi/Pebble)
- `libvmi` (optional)
- `libvmi Python 2` (optional)
- `Zeromq Python 3` (optional)
- `Zeromq Python 2` (optional)
- `rekall` (optional)

# Setup

- Setup a VM. Make sure to use the `qemu:///system` connection.
Go to the `tests` folder to find a packer template and an import script if
you don't have one already.

(Nitro only supports for now `Windows XP x64` and `Windows 7 x64`, see the `Note` section below)


# Usage

- Make sure that you have loaded the modified kvm modules. 
(`cd kvm-vmi && make modules && make reload`)

- Start the VM that you would like to monitor.

- Wait for the desktop to be available on the VM.

- Start `Nitro` as root

~~~
"""Nitro.

Usage:
  nitro.py [options] <vm_name>

Options:
  -h --help     Show this screen.
  --nobackend   Don't analyze events
  --stdout      Display events on stdout, not in a log file

"""
~~~

Nitro monitors the given `<vm_name>` syscalls by activating a set of traps in KVM.
The optional components listed above are needed only if you want to extract more information
about the captured events. See the Backend section.

Here i will assume that you have installed only the required ones.
Therefore you have to run Nitro with the option `--nobackend`.

It will run until the user sends a `CTRL+C` to stop it, in which case Nitro
will unset the traps and write the captured events in a file named `events.json`.

In case the option `--stdout` is enabled, Nitro will display the events immeditely,
and skip the log file.

An event should look like this output
~~~JSON
  {
    "direction": "enter",
    "rax": "0x1005",
    "vcpu": 0,
    "type": "syscall",
    "cr3": "0x1b965000"
  },
~~~


A successful run should give the following output :

~~~
$ sudo ./nitro.py --nobackend --stdout nitro_win7x64
Setting traps to False
Finding QEMU pid for domain nitro_win7x64
Detected 1 VCPUs
Setting traps to True
Start listening on VCPU 0
{'cr3': '0x6cdc000',
 'direction': 'exit',
 'rax': '0x3f',
 'type': 'syscall',
 'vcpu': 0}
{'cr3': '0x6cdc000',
 'direction': 'enter',
 'rax': '0x138',
 'type': 'syscall',
 'vcpu': 0}
{'cr3': '0x6cdc000',
 'direction': 'exit',
 'rax': '0x0',
 'type': 'syscall',
 'vcpu': 0}
{'cr3': '0x6cdc000',
 'direction': 'enter',
 'rax': '0x58',
 'type': 'syscall',
 'vcpu': 0}
{'cr3': '0x6cdc000',
 'direction': 'exit',
 'rax': '0x0',
 'type': 'syscall',
 'vcpu': 0}
{'cr3': '0x6cdc000',
 'direction': 'enter',
 'rax': '0x138',
 'type': 'syscall',
 'vcpu': 0}
{'cr3': '0x6cdc000',
 'direction': 'exit',
 'rax': '0x0',
 'type': 'syscall',
 'vcpu': 0}
{'cr3': '0x6cdc000',
 'direction': 'enter',
 'rax': '0x5f',
 'type': 'syscall',
 'vcpu': 0}
Setting traps to False
~~~

# Backend

The Backend is supposed to analyze raw nitro events, and extract useful
informations, such as:
- process name
- process PID
- syscall name

First, `Rekall` is used in `symbols.py` to extract the syscall table from
the memory dump.

Then, a subprocess is started to run `libvmi` (it is only Python2),
and the communication is managed with `ZeroMQ`.

On a debian based distro, you can install the following packages:
- `python-zmq`
- `python3-zmq`

Unfortunately, `Rekall` is not available as a Debian package.
For now you will have to install it system-wide with `pip`.

## libvmi

- Compile and install `libvmi`. See the [install notes](http://libvmi.com/docs/gcode-install.html)

- Configure the file `libvmi.conf`, which is already provided in the repo

Configure the name of your vm that you want to monitor :
(only `Windows 7 x64` is supported here)

~~~
nitro_win7x64 {
    ostype      = "Windows";
    win_tasks   = 0x188;
    win_pdbase  = 0x28;
    win_pid     = 0x180;
    win_pname   = 0x2e0;
}
~~~

At least, the following keys are required :
- `win_tasks`
- `win_pdbase`
- `win_pid`
- `win_pname`

If you have installed everything correctly, you can run Nitro :
`sudo ./nitro.py nitro_win7x64`

An event should now look like this:
~~~JSON
  {
    "event": {
      "cr3": "0xbda6000",
      "direction": "enter",
      "type": "syscall",
      "vcpu": 0,
      "rax": "0x14"
    },
    "name": "nt!NtQueryValueKey",
    "process": {
      "name": "services.exe",
      "pid": 456
    }
  },
~~~


# Note

We need to use the root user because:
- `libvmi` uses only `qemu:///system` as libvirt connection
- even if you are in the `libvirt` group, and therefore can access `qemu:///system`,
the memory dump produced by `libvirt` is created as `root:root`...
