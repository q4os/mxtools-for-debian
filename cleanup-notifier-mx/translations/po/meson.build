i18n = import('i18n')
sendbugs = 'https://forum.mxlinux.org/viewforum.php?f=96'
i18n.gettext(
        gettext_package,
        args: [ '--package-name=@0@'.format(gettext_package),
                '--join-existing',
                '--no-location',
                '--no-wrap',
                '--from-code=UTF-8',
                '-c', '-cTRANSLATOR',
                '--msgid-bugs-address=@0@'.format(sendbugs),
                '-o', '../@0@.pot'.format(gettext_package),
                ],
        preset: 'glib'
        )
