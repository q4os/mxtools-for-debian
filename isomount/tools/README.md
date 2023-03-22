Tools for the isomount suite
============================


The tools make-screenshoots, set-dpi, and parse-images are supposed
to semi-automate the process of calculating constants in the isomount
program for scaling the height of a yad table according to the
current screen DPI (dpi) and the number of rows in the table.

These are currently the four constants:

    YAD_HT_DATA_PPC
    YAD_HT_DATA_PAD
    YAD_HT_DATA_OFF
    YAD_HT_DATA_PPR

They appear in this formula for the hight in pixels:

    (YAD_HT_DATA_PAD + (YAD_HT_DATA_PPC * d_rows)) * dpi  / 96 / 100)
     + YAD_HT_DATA_OFF / 100 + YAD_HT_DATA_PPR * d_rows / 100

For simplicity, let's remove the scaling constants 96 and 100, and
to further simplify, let's remove the YAD-HT-DATA prefix and re-arrange:

    OFF + PAD * dpi + PPC * (dpi * rows) + PPR * rows = height

Inside the program the uppercase things are constants and the
lower case ones are variables (dpi, d-rows).  But to find the
values of the constants we will treat them as variables and
create a series of equations based on different values of dpi
and d-row.  For each set of [dpi, rows] we get a different
linear equation calculating the height in terms of the uppercase
variables/constants we want to find.

For example, if the dpi is in {100, 120} and the rows are in {3, 9}
then we get four equations:

    OFF + PAD * 100 + PPC * (3 * 100) + PPR * 3 = height_1
    OFF + PAD * 120 + PPC * (3 * 120) + PPR * 3 = height_2
    OFF + PAD * 100 + PPC * (9 * 100) + PPR * 9 = height_3
    OFF + PAD * 120 + PPC * (9 * 120) + PPR * 9 = height_4

Each height comes from the measurement of the height of the
table in pixels on the screen.  In theory, if we took four
measurements (very accurately) then we could determine the four
constants.  In practices, it helps to use more than four
measurements and average out the noise.

The program make-screenshots (along with set-dpi) are designed
top create the yad windows you need to measure and offer to take
a screen shot.  Always select the area of the table in yad.

The filenames of the images contain the date and the time, and
they also contain the dpi, rows, and height information we need
to write down the set of equations.   The program parse-images,
reads in these filenames and creates a data file containing the
information we need in a format that can be read in by Octave
(and MatLab).  

The yad-ht-data-mat.m script will take a series of N [dpi, rows]
rows and create a matrix that will transform a vector of our
four (uppercase) constants into the heights for each set of dpi,
and rows.  We then invert this matrix and feed it the measured
heights to get the four constants we want popping out on the
other end.

But it didn't work on my first try so I think there is a minor
bug lurking somewhere.


