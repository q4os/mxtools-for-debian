# cli-aptiX

## Synopsis

This is a command line wrapper aound the basic "apt-get", "apt-cache", and
related commands.  It's designed to make it easy to search for and install
Debian packages. In some ways it is similar to the GUI based Synaptic
program.  If you been wanting something like Synaptic that works in the command
line then maybe "cli-aptiX" is what you've been looking for.

## Search for Packages

After the initial "apt-get update" you will normally perform a search to look
for packages by name.  For any search term you enter, there will be four
different sets of results:

#### Exact Name Match

If any packages have names that exactly match your search term, they are listed
here.  There can be more than one package in this list if you used the
" * " or "?" wildcard characters.

#### Leading Name Match

This includes all packages that match your search term at the beginning of
their name.  For example, if you use the search term "abc" then you will get a
Leading Name Match result list that looks like:

    1) (I) abcde - A Better CD Encoder
    2) abcm2ps - Translates ABC music description  ...
    3) abcmidi - converter from ABC to MIDI format ...

The "(I)" on the first line indicates that package is already installed. Notice
that all three names begin with "abc".

#### Any Name Match

This list includes all packages with names that contain your search term
anywhere.  If the search term is "abc" then the results will be something like:

    1) (I) abcde - A Better CD Encoder
    2) abcm2ps - Translates ABC music description  ...
    3) abcmidi - converter from ABC to MIDI format ...
    4) berkeley-abc - ABC - A System for Sequentia ...
    5) grabc - simple program to determine the col ...
    6) libakonadi-kabc4 - Akonadi address book acc ...
    7) libkabc4 - library for handling address boo ...
    8) python-backports-abc - Backport of the "col ...

Notice that this list contains all the packages from the previous list and the
name for every package in this list contains "abc".

#### Name or Description Match

This list contains all of the packages from the previous list plus any packages
that contain your search term in their description.

### Selecting a Package from a Search Result List

As long as the entire list fits on the screen, this is straightfoward, just
type in the number associated with the package you want to install.  But if the
list is too long to all fit on the screen at once then you will need to scroll
through the list using "Up-Arrow" and "Down-Arrow" and <Page-Up> and
"Page-Down" to scroll the list (and even "Left-Arrow" and "Right-Arrow" to
scroll sideways).  Scroll the list so the package you want to select is near
the botton of the screen and then press 'q'. You will be asked to enter the
number for the package you want.
