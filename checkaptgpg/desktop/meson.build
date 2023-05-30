i18n.merge_file(
    input: configure_file(
        input:  '@0@.desktop'.format(desktop_id),
        output: '@0@.desktop.i18n'.format(desktop_id),
        capture: true,
        command: ['sed', '/^#/d', '@INPUT@']
        ),
    install: true,
    install_dir: get_option('datadir') / 'applications',
    output:  '@0@.desktop'.format(desktop_id),
    po_dir: '..' / subdir_po,
    type: 'desktop',
)
