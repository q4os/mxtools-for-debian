#!/bin/bash

if [ -d po ]; then
    DIR=po
else
    DIR=.
fi

if [ -f po/en.pot ]; then
    POT=po/en.pot 
else
    POT=en.pot 
fi

sed -i 's/charset=CHARSET/charset=UTF-8/' $POT

LOG=Tx-complete.log

LINGUAS=$DIR/LINGUAS
[[ -f $LINGUAS ]] && mv $LINGUAS $LINGUAS.$(date '+%Y.%m.%d-%H%M%S').orig
LINGUAS=$DIR/LINGUAS

printf '%6s\t\t%4s\t%7s\t\t%s\t\t%s\n' "Nr." "Cnt." "Compl." "Code" "Language" | tee $LOG
printf '%6s\t\t%4s\t%7s\t\t%s\t\t%s\n' "---" "----" "------" "----" "--------" | tee -a $LOG

for P in $DIR/*.po ; do 
    L=${P##*/}; 
    L=${L%.po};
    ll=${L%%_*}
    rr=${L##*_}
    [[ -n ${L##*_*} ]] && rr=
    L="$(printf '%-8s' ${L})";

    Z=$(msggrep --no-wrap -T -e '..' $P  | grep -c msgid); 
    T=$(grep -c msgid $POT) ; 
    ((T--))
    ((Z>0)) && ((Z--))
    printf '\t%4d\t%6d%%    \t%s\t%s%s\n' $Z $((Z*100/T)) "$L" \
        "$(isoquery --iso=639-2 -n $ll     | cut -f4 )"  \
        "${rr:+ @ $(isoquery -i 3166-1 $rr | cut -f4 )}"; 
done | sort -t $'\t' -k2nr,2 -k4,4 | cat -n | tee -a $LOG

# create LINGUAS with translations completness >= 1%
#
echo "# languages codes with translation completness  >= 1%"   >  $LINGUAS
echo "# generated with tx-complete.sh at $(TZ=UTC date -R)"    >> $LINGUAS 
echo "# "                                                      >> $LINGUAS
grep -E '([1-9]|[0-9]{2})%' $LOG  | awk '{print $4}' | sort -u >> $LINGUAS

echo ""
echo "$LINGUAS:"
grep '#' $LINGUAS
echo $( grep -v '#' $LINGUAS )

exit

