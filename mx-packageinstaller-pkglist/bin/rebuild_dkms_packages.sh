#!/bin/bash

#force rebuild of dkms packages (virtualbox, nvidia, fglrx, broadcom, ndiswrapper)
#part of mx-packageinstaller

kernel="$1"

# if parameter passed, then check to see if package is installed, exit if not
if [ -n "$kernel" ]; then
    [[ -n $(grep -A1 $kernel /var/lib/dpkg/status | grep 'install ok installed'  ) ]] || exit 0
fi	

#if no kernel parameter given, assume current kernel
if [ -z "$kernel" ]; then
	kernel="$(uname -r)"
fi

kernel=${kernel#"linux-image-"}
kernel=${kernel%-unsigned}

echo "update dkms modules for kernel: " $kernel |tee /var/log/rebuild-dkms.log
echo "see log at /var/log/rebuild-dkms.log"

/usr/lib/dkms/dkms_autoinstaller start $kernel |tee -a /var/log/rebuild-dkms.log

exit 0
