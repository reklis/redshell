# Red Shell

easily execute windows commands from linux using zeromq

![Red Shell v1.0.0](./redshellicon.png "Red Shell v1.0.0")

## goals

Generally speaking, the primary goal is seamless client/host integration.  The ability to type a command from inside a vm, and have it run that command on the vm host.

Specifically I use this to launch editor windows on my windows host from inside linux vms because quite frankly when it comes to shells, cygwin is just not good enough. In theory this can be used for any linux to windows remote communication if configured properly or as a host exploit if configured poorly.

## files and directories

* `win-systray` contains the source code for the system tray application.  This app listens on a socket and allows remote hosts to execute a set of whitelisted commands

* `win-systray/x64/Debug` is a pre-compiled version of this system tray application with example configuration files

* `node-client` is an example client script and configuration files that can be used to send a command from inside a linux vm to a windows host

* `shell-wiring` contains an example of what you might want to add to your `.zshrc` or `.bash_profile` inside your guest vm shell configuration to change your editors to use remote commands

## systray server configuration

The system tray app uses two ini files for configuration.  The first is `network.ini` which contains the information on what host address to bind to.  For security purposes, I create a virtualbox host only adapter and then add a second NIC to the virtualbox client machine.  Then, I have the system tray application listen only on this vbox host only network.  This way other machines on the lan or wifi can't execute commands.

Example network.ini

    [bind]
    ; virtualbox host-only network adapter is by default 192.168.56.*
    addr=tcp://192.168.56.1:5555

The second file is `commands.ini` and is a dictionary of what commands the remote clients can execute and what exe files they corrospond to on the windows host.

Example commands.ini

    [sub]
    exe=c:\program files\Sublime Text 3\sublime_text.exe

    [notepad]
    exe=c:\Windows\System32\notepad.exe

    [gvim]
    exe=C:\Program Files (x86)\Vim\vim74\gvim.exe

Hopefully this file format is obvious.  The key `[sub]` is the command a remote client can execute.  The `exe` value is what it will be resolved to at run time if a client sends that command.

## node client configuration

For starters, you'll need a working node and npm to run the script.  Use your package manager.  I have tested this with v0.12.7.  The client script has zeromq as it's only dependency.  After downloading, you can `npm install` to install it locally if you don't have it available globally on your system already.

Because we are editing files on the host machine and sending paths from the guest vm, we need to translate the linux path to the windows path before sending the command.  This is done by adjusting the `settings.json` with information about how the shared folders are setup.

Example `settings.json`

    {
      "socket": "tcp://192.168.56.1:5555",
      "mountpoints": [
        { "vm": "/mnt/code/",
          "host": "c:\\dev\\" },
        { "vm": "/mnt/nthome/",
          "host": "c:\\Users\\myuser\\" }
      ]
    }

Here I have created two shared folders inside virtualbox.  One named `nthome` and the other named `code`.  I mounted them on the guest os like so:

    mount -t vboxsf code /mnt/code
    mount -t vboxsf nthome /mnt/nthome

If you want to automatically mount your virtual box shared folders you can tell virtual box it is a permanent mount, and then add some lines like these to your global fstab

Example `/etc/fstab` entries

    # virtualbox shares
    nthome                  /mnt/nthome     vboxsf          uid=myuser,gid=mygroup,rw,dmode=700,fmode=600,comment=systemd.automount              0 0
    code                    /mnt/code       vboxsf          uid=myuser,gid=mygroup,rw,dmode=700,fmode=600,comment=systemd.automount              0 0

Please note `myuser` and `mygroup` should probably be replaced with your actual linux user and group names so you have proper permissions to the mounts.

## wire protocol

The command structure is extremely simple

    CMD\vPARAMS

The message is split on the vertical tab character.  This character is interesting in that it's not easy to type, but it's easy to program with.  There probably is not a vertical tab character on your keyboard, but there is a convenient escape sequence for it, so we can easily embed it inside strings without much effort.  It's also obscure enough that it won't be in your file names or your command names unlike a hyphen or pipe.

Upon receiving the command, the host will find the first vertical tab character in the character sequence and split the message into two parts

Example:

    sub\v-n foo.txt
    ---^ split

    command: sub
    params: -n foo.txt

The command 'sub' is then looked up in the set of configured commands and replaced with the actual host command to execute.  In this case, the full path to the Sublime Text 3 exe.

The rest of the systray code is basically a bunch of boilerplate to create a hidden window, register it with the tray, and add a right click menu along with some reading of the configurating files using some horrifying windows calls.  It's ugly, but it works.

## contributing

Fork and send a pull request or file a bug with your thoughts and preferably a standard unix patch file

Specifically, I'd love if others would provide alternative client scripts in other languages like python, perl, ruby, etc so that node is not a hard requirement for the guest os.

## license

GPLv3

