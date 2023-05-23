inames = [
         package_name,
         ]

icon_dir = get_option('datadir') / 'icons'
pix_maps = get_option('datadir') / 'pixmaps'

fs = import('fs')
foreach iname: inames
    isvg = '@0@.svg'.format(iname)
    if not fs.is_file(isvg)
        continue
    endif
    install_data(
        isvg,
        install_dir: icon_dir / 'hicolor' / 'scalable' / 'apps',
        )
    install_data(
        isvg,
        install_dir: pix_maps,
        )
endforeach
