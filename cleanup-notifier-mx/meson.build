project('cleanup-notifier-mx', 
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

package_name    = meson.project_name()
desktop_id      = package_name
gettext_package = package_name

po_dir = 'translations/po'

subdir('data')

subdir('src')

subdir(po_dir)
