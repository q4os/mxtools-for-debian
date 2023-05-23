project('mx-launcher-l10n', 
    meson_version: '>=1.0.0',
    version: configure_file(
        command: [
        'dpkg-parsechangelog', '-l', 
        meson.project_source_root() / 'debian/changelog',
        '-SVersion'],
        capture: true,
        output: 'version',
        )
)

subdir('translations/po')
