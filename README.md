# DPKG-Hider

As is well known, in debian and its derivatives, dpkg is the main package manager.
Once you installed a package, you can can execute `dpkg -l`, and notice that the package has been installed.
Information about packages is usually saved in /var/lib/dpkg/status.

DPKG-Hider lets you hide an installed package easily. After using DPKG-Hider, the requested package won't be shown when executing `dpkg -l`.
It's important to backup the /var/lib/dpkg/status file, in case you'll want to return to the original state.

USAGE: dpkg-hider PackageName

# Author
Kfir Shtober (Kfiros) 2015
