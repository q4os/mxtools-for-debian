
i18n = import('i18n')
i18n.merge_file(
    input: configure_file(
        input: '@0@.desktop'.format(desktop_id),
        output: '@0@.desktop.i18n'.format(desktop_id),
        capture: true,
        command: ['sed', '/^#/d', '@INPUT@'],
        ),
    type: 'desktop',
    po_dir: '../po',
    output: '@0@.desktop'.format(desktop_id),
    install: true,
    install_dir: get_option('sysconfdir') / 'xdg/autostart',
)

subdir('icons')

