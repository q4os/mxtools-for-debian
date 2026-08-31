---
title: VERIFY-ISO-SIG
section: 1
header: User Commands
footer: verify-iso-sig
date: July 2026
---

# NAME

verify-iso-sig - verify the GPG signature of a downloaded ISO image

# SYNOPSIS

**verify-iso-sig** \[*iso-file*\]

**verify-iso-sig** \[**--cli**\|**--gui**\] \[*options*\] *iso-file* \[*sig-file*\]
\[*checksum-listing-file*\]

**verify-iso-sig** \[**--cli**\|**--gui**\] **--manage-keys**

**verify-iso-sig** **--keep-key** *fingerprint* \[**--from-ring**=*path*\]

**verify-iso-sig** **--is-cached** *iso-file* \[*sig-file*\]

**verify-iso-sig** **--list-trusted-keys**

**verify-iso-sig** **--list-known-keys**

**verify-iso-sig** **--list-keyservers**

**verify-iso-sig** **--untrust-key** *fingerprint*

**verify-iso-sig** **--export-trusted-keys** *path* \[*fingerprint...*\]

**verify-iso-sig** **--inspect-key-file** *path*

**verify-iso-sig** **--import-trusted-keys** *path* \[*fingerprint...*\]

# DESCRIPTION

**verify-iso-sig** checks that a downloaded `.iso` file really is
what it claims to be, by verifying its GPG signature. It works with
any distro's ISO, not just MX Linux/antiX - it has built-in recognition
of the documented MX Linux/antiX signing keys, but any other signing
key is still fully checked cryptographically; the tool just asks for
explicit confirmation before trusting an unrecognized one (see
**RECOGNITION RULES** below).

Run with no arguments, or a single file (e.g. from a file manager's
"Open With"), it opens a picker window when a graphical session is
available, falling back to this same command-line tool otherwise - see
**--cli**/**--gui** below to force one or the other regardless of what's
actually available. The rest of this manual describes the command-line
tool's own behavior; the picker GUI covers the same ground
interactively. See also **--manage-keys**.

Three signing conventions are supported automatically:

- **Direct signature**: the ISO itself is signed, with a companion
  *iso-file*`.sig`/`.asc`/`.gpg` (or `.sign`) file. This is checked
  first. When both *iso-file* and *sig-file* are given, either order
  works as long as exactly one of the two looks like a signature file -
  by extension, or by content when the extension is unusual and
  doesn't give it away. If both look like one, the first argument is
  treated as *iso-file* and the second as *sig-file*; if neither looks
  like one, by name or content, this tool refuses instead of guessing
  which is which.
- **Checksum-listing convention**: many distros other than MX Linux/antiX
  sign a checksum-listing file instead of the ISO directly (e.g.
  Debian's `SHA256SUMS`/`SHA256SUMS.sign`, Ubuntu's
  `SHA256SUMS`/`SHA256SUMS.gpg`, Linux Mint's
  `sha256sum.txt`/`sha256sum.txt.gpg`), so one signature covers a
  whole family of ISOs. If no direct signature file exists next to the
  ISO, this tool automatically looks in the ISO's own directory for a
  known checksum-listing file (`SHA512SUMS`, `SHA256SUMS`, `SHA1SUMS`,
  `MD5SUMS`, the lowercase `*sum.txt`-style variants - strongest hash
  first - or any file literally named `checksum`/`checksums`,
  case-insensitive) paired with a signature file
  (`.sig`/`.asc`/`.gpg`/`.sign`), verifies *that* file's signature
  (using the same recognition rules as a direct signature), and then
  verifies the ISO's own hash against the line in it that names this
  exact ISO file.

  A bad/untrusted signature on the checksum file, and a valid
  signature but a mismatched ISO hash, are reported as two clearly
  different failures - the first means nothing in the listing can be
  trusted; the second means the listing itself is fine but this
  particular download is not.

  If a checksum-listing filename (e.g. `SHA256SUMS`) is itself passed
  as *iso-file* - e.g. its own signature file was picked directly, not
  an actual ISO - this tool does not just check that listing's own
  signature and call it done, since a checksum listing can cover many
  ISOs and that would never actually verify any of them. Instead it
  looks for exactly one of the ISOs the listing mentions that is
  actually present next to it and checks that one; it refuses with a
  clear error naming the ISOs found (or noting that none are present)
  if the answer isn't unambiguous. That ambiguity can be resolved
  explicitly instead: passing the checksum-listing file together with a
  specific *iso-file* as the two arguments (either order) pins exactly
  which ISO to check against it, skipping the "which one did you mean"
  guesswork - the listing's own signature is still found and verified
  the normal way either way.

  All three files - the ISO, the checksum listing, and the listing's
  own signature - can also be given explicitly at once, in any order,
  when they are not all in the same directory (auto-discovery only
  looks next to the ISO). The same also works for a plain per-file
  checksum (e.g. `.sha512`) instead of a shared listing: give the ISO,
  the checksum file, and either that file's own signature or the ISO's
  own direct signature, in any order. If the three files don't fit one
  of these known shapes (e.g. a real signature file named outside the
  usual convention), any two of them that do form a valid ISO plus
  signature pair are used instead, ignoring the third file.
- **Inline-signed checksum listing**: some distros (e.g. Fedora) sign
  the checksum listing itself instead of attaching a separate signature
  file - the listing begins with a `-----BEGIN PGP SIGNED MESSAGE-----`
  header and ends with its own inline signature block (an OpenPGP
  cleartext signature, RFC 4880 §7), verified directly with no separate
  `.sig`/`.asc`/`.gpg`/`.sign` file needed. Recognized both by the fixed
  listing names above and by release-specific filename patterns (e.g.
  Fedora's own `*-CHECKSUM`/`CHECKSUM` convention); otherwise behaves
  exactly like the checksum-listing convention above, including
  two-argument pinning and the "pointed straight at the listing"
  redirect just described.

# RECOGNITION RULES

The signing key's fingerprint is read directly out of the signature
packet - no key material is needed for that step. That fingerprint is
then checked against three things, any one of which is enough to
consider it recognized:

1. This tool's own built-in list of recognized signing keys - mostly
   the MX/antiX keys documented at
   <https://mxlinux.org/wiki/system/signed-iso-files/> (a source
   independent of the ISO/sig download itself), plus a small number of
   separately-vetted third-party respin keys not on that page. Run
   **--list-known-keys** to print this list directly and offline,
   without needing to visit the wiki.
2. Whether the key is already cached and usable (valid, or merely
   expired - see below) in `~/.gnupg/trustedkeys.gpg`. Getting a key
   into that keyring already requires a deliberate
   **--keep**/**--trust-key**/**--keep-key** decision, a manual `gpg
   --import`, or importing it through the trusted-keys manager
   (**--manage-keys**, also reachable from the picker's own "Manage
   Trusted Keys" button) - so its presence there counts as recognized
   too, exactly like being on the built-in list.
3. Whether the key is in `~/.gnupg/pubring.kbx` (where GUI tools such
   as Seahorse / GNOME "Passwords & Keys" store manually-imported
   keys) **and** has been *explicitly* marked marginally/fully/
   ultimately trusted there - not merely imported, which carries no
   trust signal on its own, but a deliberate trust decision made
   through those tools counts the same as an explicit **--trust-key**.

This matters because without some form of this check, an attacker who
supplies both a malicious ISO and a matching malicious signature file
would otherwise be trusted just as readily as the real thing.

A key recognized via rule 3 is automatically copied into
`~/.gnupg/trustedkeys.gpg` (no flag or prompt needed) - the trust
decision was already made explicitly by the user elsewhere, so this
just persists it into the keyring `gpgv` actually reads, the same way
**--trust-key** does for a key approved through this tool directly.

Before ever touching the network: `~/.gnupg/trustedkeys.gpg` (this
tool's own dedicated keyring) is checked first and reused if the key
is present there, either fully valid or merely expired - this works
fully offline, as long as a previous run (with **--keep**,
**--trust-key**, or **--keep-key**) or a manual `gpg --import` put it
there. Key expiry is a lifecycle signal, not a cryptographic weakness -
unlike revocation, a permanent assertion from the key's own owner that
it must never be trusted again - so an expired cached key is still
used normally, with a warning printed and one quiet, best-effort
attempt to find a renewed copy first (checking `pubring.kbx`, then a
keyserver, silently updating `trustedkeys.gpg` in place if a
non-expired copy turns up). `~/.gnupg/pubring.kbx` is checked
read-only as a fallback - it is never written to. Only if the key is
missing everywhere, or revoked, does this tool fetch it from a
keyserver as part of the actual verification, and only with **--keep**
is that fetched copy saved into `trustedkeys.gpg` for next time.

# OPTIONS

**-h**, **--help**
:   Show a short usage summary and exit.

**-V**, **--version**
:   Show the version (`yyyy.mm.PP`) and exit.

**--man**
:   Show this manual page.

**--cli**
:   Force plain command-line behavior for the rest of this invocation,
    even with a graphical session available - useful from a terminal
    when a window popping up isn't wanted. Must come first; consumed,
    not forwarded to anything else.

**--gui**
:   Force the picker GUI for the rest of this invocation, even with no
    graphical session detected (`yad` then fails with its own real
    error - this doesn't second-guess an explicit request). Must come
    first; consumed, not forwarded.

**--manage-keys**
:   Open the trusted-keys manager: list, forget, export, or import keys
    saved in `~/.gnupg/trustedkeys.gpg` (also reachable from the
    picker's own "Manage Trusted Keys" button). Opens as a GUI window
    when a display is available (respecting a leading
    **--cli**/**--gui**); otherwise prints the equivalent command-line
    flags (**--list-trusted-keys**, **--untrust-key**,
    **--export-trusted-keys**, **--inspect-key-file**,
    **--import-trusted-keys**) and exits.

**--drag-and-drop**
:   Same as **--gui**, but also shows the picker's drag-and-drop pane.
    Not the default: most users already have their file in hand (opened
    via a file manager's "Open With", or one of this tool's own MIME
    associations) and never need to drag one in, so the plain picker
    form is simpler and opens faster. Also reachable from the app
    menu's own "Verify with Drag & Drop" entry. Has no effect under
    Wayland - the drag-and-drop pane needs `yad`'s `--paned`/`--plug`
    window embedding, which Wayland doesn't support; the plain form is
    shown there regardless.

**--quiet**
:   Suppress informational `[*]` log lines (`gpg`/`gpgv` output is
    still shown).

**--debug**
:   Print every `gpg`/`gpgv` command line (as `debug: ...`) right
    before it runs, so it's clear exactly what's being executed and
    why a step is taking time (e.g. hashing a large ISO vs. waiting on
    a keyserver).

**--status-fd** *fd*
:   Also print machine-readable `[VERIFY-ISO-SIG:] TAG value` lines
    (alongside the normal, human-readable output) to file descriptor
    *fd* - e.g. `SIGNATURE_FPR <fingerprint>`, `CLAIMED_IDENTITY <uid>`,
    `ALREADY_IN_TRUSTED_GPG`. These tags are a stable protocol, always
    in English regardless of locale - the same mechanism this tool's own
    `gpgv --status-fd` calls already use internally, one level up. Not
    needed for everyday use - only useful for a script driving this tool
    programmatically from a separate process.

    The trusted-keys manager GUI no longer uses this at all: it sources
    this script directly and reads its functions' own return values
    instead. The main GUI also sources this script directly rather
    than invoking it as a subprocess, but still ends up relying on this
    same tag stream for its main verify/`--is-cached` calls (the
    progress dialog it shows during a check has to run that call in a
    background subshell, which makes reading any other function-return
    value back out impossible) - it gets the tag stream automatically in
    that case, without passing `--status-fd` itself.

**--keyserver** *list*
:   Use these keyserver(s) instead of the built-in fallback list (see
    **KEYSERVER** under ENVIRONMENT below) for this run only - overrides
    the **KEYSERVER** environment variable if both are set. *list* may
    name one or more keyservers, separated by commas and/or spaces
    (mixing both is fine); any entry with no `scheme://` already in it
    is assumed to be `hkps://`. Example:
    `--keyserver="keys.openpgp.org, hkp://pool.example.org"`.

**--keep**
:   If the key had to be fetched from a keyserver, save it into
    `~/.gnupg/trustedkeys.gpg` so future runs don't need the network
    at all. Without this flag a freshly-fetched key is used once and
    discarded.

**--allow-unrecognized-key**
:   Proceed just this once even if the signing key's fingerprint is
    not one of the recognized keys (see **RECOGNITION RULES**).
    Without this flag or **--trust-key**, an unrecognized key causes
    the tool to refuse to verify.

**--trust-key**
:   Like **--allow-unrecognized-key**, but if the signature actually
    verifies, also saves the key into `~/.gnupg/trustedkeys.gpg` so
    future runs recognize it automatically - use this for a signing
    key you've checked and decided to trust (e.g. a respin/variant not
    on the MX/antiX wiki page).

**--no-checksum-fallback**
:   Disable the checksum-listing convention entirely - restores the
    plain "cannot read signature file" error when no direct signature
    exists.

**--checksum-file**=*path*, **--checksum-file** *path*
:   Force a specific checksum-listing file instead of auto-detecting
    one. Its own signature file is still auto-detected via the usual
    `.sig`/`.asc`/`.gpg`/`.sign` suffixes.

**--checksum-algo**=*algo*, **--checksum-algo** *algo*
:   One of `sha256`, `sha512`, `sha1`, or `md5`. Only needed together
    with **--checksum-file** when that file isn't one of the
    recognized names (so the hash algorithm can't be inferred from its
    name).

**--keep-key** *fingerprint*
:   Internal mode used by **--trust-key**/the main GUI's own trust
    flow - not really meant for everyday standalone use: makes sure
    *fingerprint* ends up cached in `~/.gnupg/trustedkeys.gpg` -
    checking `~/.gnupg/pubring.kbx` and fetching from a keyserver if
    needed - without touching any ISO/signature or running `gpgv`. A
    key present and valid in `trustedkeys.gpg` is treated as recognized
    on later runs even if it isn't in the built-in list.

**--from-ring**=*path*, **--from-ring** *path*
:   Internal plumbing for the main GUI's own trust flow, not something
    to construct by hand: with **--keep-key**, reuse a key already
    exported to *path* by an earlier **--export-key-to**=*path* run
    instead of fetching it again from a keyserver (falls back to a
    normal lookup/fetch if *path* doesn't actually contain the key).

**--export-key-to**=*path*, **--export-key-to** *path*
:   Internal plumbing for the main GUI's own trust flow, not a general
    "export this key" feature: *path* ends up holding a raw GnuPG
    keyring file meant only for a later **--keep-key
    --from-ring**=*path* call - not a portable, standalone key export
    (no `--armor`, not something `gpg --import` elsewhere would make
    sense of). After a normal verify run resolves the signing key (from
    cache or a keyserver), writes it there so trusting a key the user
    just saw doesn't need a second keyserver round-trip.

**--is-cached**
:   Internal plumbing for the main GUI (picks accurate progress-dialog
    wording before a real run) - not really useful standalone: reads
    the signing key's fingerprint out of the signature file and
    reports, via exit status only (0 = yes, 1 = no), whether that key
    is already cached and valid in `trustedkeys.gpg` or `pubring.kbx` -
    i.e. whether a real run would need the network. No recognition
    check, no fetch, no `gpgv`.

**--list-trusted-keys**
:   A separate mode: lists every key currently saved in
    `~/.gnupg/trustedkeys.gpg`, one per line, as
    *fingerprint*\|*validity*\|*claimed-identity*\|*known*\|*expiration*
    - `known` is the literal word `known` if this fingerprint is also
    one of the documented MX/antiX keys built into this script
    (removing it from `trustedkeys.gpg` will **not** make it
    unrecognized - that recognition doesn't depend on
    `trustedkeys.gpg` at all), empty otherwise. `expiration` is the
    key's expiration date as Unix epoch seconds, empty if it has none.
    No `trustedkeys.gpg` yet, or nothing saved - no output, exit 0 (an
    empty list, not an error).

**--list-known-keys**
:   A separate mode: lists the built-in signing keys this tool
    recognizes out of the box, one per line, as *fingerprint*\|*label*
    - mostly the MX/antiX keys documented at
    <https://mxlinux.org/wiki/system/signed-iso-files/>, printed
    directly instead of requiring a visit to that page, plus a small
    number of separately-vetted third-party respin keys not on that
    page (their own label makes this clear). Entirely offline and
    independent of `trustedkeys.gpg`/`pubring.kbx` - unlike
    `--list-trusted-keys`, this is "what does this tool recognize by
    default", not "what's cached locally". To add a new key, edit the
    `KNOWN_KEYS`/`KNOWN_THIRD_PARTY_KEYS` arrays near the top of the
    script itself - there is no runtime way to add to this list.

**--list-keyservers**
:   A separate mode: prints the actual, already-resolved keyserver list
    this run would try, one per line, in the order they'd be tried -
    reflects **--keyserver**/**KEYSERVER** if either is set, not just
    the built-in defaults. Useful to check what's actually in effect
    without **--debug** or a real fetch.

**--untrust-key**=*fingerprint*, **--untrust-key** *fingerprint*
:   A separate mode: removes exactly that fingerprint from
    `~/.gnupg/trustedkeys.gpg` (the reverse of
    **--keep**/**--trust-key**/**--keep-key**) - use this to undo a
    trust decision made by mistake. Dies with a clear error if
    `trustedkeys.gpg` doesn't exist or doesn't contain that
    fingerprint.

**--export-trusted-keys**=*path*, **--export-trusted-keys** *path* \[*fingerprint...*\]
:   A separate mode: exports key(s) from `~/.gnupg/trustedkeys.gpg` to
    *path*, ASCII-armored - useful for carrying trusted keys to another
    install, or keeping an offline backup so a fresh setup doesn't need
    the network or a real ISO verification to repopulate them. With no
    fingerprints given, exports every key currently trusted; a given
    fingerprint that isn't found is a warning, not a hard failure -
    only an empty final set is an error. Overwrites *path* if it
    already exists.

**--inspect-key-file**=*path*, **--inspect-key-file** *path*
:   A separate, read-only mode: lists the key(s) found in an arbitrary
    external key file (never imports anything, never touches any
    keyring), in the same
    *fingerprint*\|*validity*\|*claimed-identity*\|*known*\|*expiration*
    format as **--list-trusted-keys**. Refuses with a clear error if the
    file contains private/secret key material.

**--import-trusted-keys**=*path*, **--import-trusted-keys** *path* \[*fingerprint...*\]
:   A separate mode: imports key(s) from an external key file (as
    produced by **--export-trusted-keys**, or exported by any other
    OpenPGP tool) into `~/.gnupg/trustedkeys.gpg`. With no fingerprints
    given, imports every key found in the file; a given fingerprint
    that names a subkey rather than a primary key is resolved to its
    owning primary key automatically. Refuses with a clear error if the
    file contains private/secret key material - this check runs before
    anything is imported, not just before the final save.

# ENVIRONMENT

**KEYSERVER**
:   Use these keyserver(s) instead of the built-in fallback list
    (`hkps://keys.openpgp.org`, `hkps://keyserver.ubuntu.com`,
    `hkps://pgpkeys.eu` - tried in that order, first success wins;
    useful if one is unreachable on your network). Same
    comma/space-separated, `hkps://`-assumed format as **--keyserver**
    (see above), which overrides this variable if both are set. Every
    fetch uses `gpg`'s `import-clean`/`import-minimal` options, which
    strip excessive or unusable signatures regardless of which server
    answered.

# EXIT STATUS

**0**
:   Signature (and, in checksum-file mode, hash) verified
    successfully; or, for **--is-cached**, the key is already cached;
    or, for **--list-trusted-keys**, the list was printed (even if empty).

**1**
:   Signature did not verify, the ISO's hash didn't match, an
    unrecognized key was refused, a required file was missing/
    unreadable, or (for **--is-cached**) the key is not cached.

# FILES

`~/.gnupg/trustedkeys.gpg`
:   This tool's own keyring of keys explicitly trusted via
    **--keep**/**--trust-key**/**--keep-key**, or copied in
    automatically from an explicit pubring.kbx trust decision.

`~/.gnupg/pubring.kbx`
:   The user's normal GnuPG keyring (e.g. managed by Seahorse/GNOME
    "Passwords & Keys") - checked read-only, never written to by this
    tool directly.

# SEE ALSO

**gpg**(1), **gpgv**(1)

Full project documentation: `README.md` in the project's own
directory. This tool's own built-in recognized keys: run
**--list-known-keys**, or see the documented MX/antiX signing keys at
<https://mxlinux.org/wiki/system/signed-iso-files/>

# AUTHORS

fehlix@mxlinux.org  
MX Linux development team
