iname = 'GPG-Key4'
sname = 'checkaptgpg'
sizes = [ 16, 22, 24, 32, 48, 64 ]

icon_dir = get_option('datadir') / 'icons'

isvg = '@0@.svg'.format(iname)
ipng = '@0@.png'.format(iname)

foreach size: sizes
    s = size.to_string()
    ssvg = '@0@_@1@.svg'.format(iname, s)
    install_data(ssvg, rename: isvg,
        install_dir: icon_dir / 'hicolor' / '@0@x@0@'.format(s) / 'apps',
        )

    install_symlink('@0@.svg'.format(sname), pointing_to : isvg ,
    install_dir: icon_dir / 'hicolor' / '@0@x@0@'.format(s) / 'apps',
    )
endforeach

install_data('@0@.png'.format(iname),
    install_dir:  get_option('datadir') / 'pixmaps',
    )

install_symlink( '@0@.png'.format(sname),
    pointing_to: '@0@.png'.format(iname),
    install_dir : get_option('datadir') / 'pixmaps',
    )

