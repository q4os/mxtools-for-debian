#!/bin/bash

#-----------------------------------
PODIR=po

LOG=$PODIR/TX-COMPLETE.LOG
LINGUAS=$PODIR/LINGUAS

EN_POT=apt-notifier.pot
#-----------------------------------

printf '%6s\t\t%4s\t%7s\t\t%s\t\t%s\n' "Nr." "Cnt." "Compl." "Code" "Language" | tee $LOG
printf '%6s\t\t%4s\t%7s\t\t%s\t\t%s\n' "---" "----" "------" "----" "--------" | tee -a $LOG

for P in $PODIR/*.po ; do 
    L=${P##*/}; 
    L=${L%.po};
    ll=${L%%_*}
    rr=${L##*_}
    [[ -n ${L##*_*} ]] && rr=
    L="$(printf '%-8s' ${L})";

    Z=$(msggrep --no-wrap -T -e '..' $P  | grep -c msgid); 
    T=$(grep -c msgid $EN_POT) ; 
    ((T--))
    ((Z>0)) && ((Z--))
    printf '\t%4d\t%6d%%    \t%s\t%s%s\n' $Z $((Z*100/T)) "$L" \
        "$(isoquery --iso=639-2 -n $ll     | cut -f4 )"  \
        "${rr:+ @ $(isoquery -i 3166-1 $rr | cut -f4 )}"; 
done | sort -t $'\t' -k2nr,2 -k4,4 | cat -n | tee -a $LOG

#       "${rr:+@ $(isoquery -i 3166-1 $rr | cut -f4  | cut -d, -f1 | cut -d\; -f1)}"; 

# create LINGUAS with translations completness >= 5%
#
echo "# languages codes with translation completness  >= 5%"   >  $LINGUAS
echo "# generated with tx-complete.sh at $(TZ=UTC date -R)"    >> $LINGUAS 
echo "# "                                                      >> $LINGUAS
grep -E '([5-9]|[0-9]{2})%' $LOG  | awk '{print $4}' | sort -u >> $LINGUAS

# create LINGUAS with translations completness >= 10%
# grep -E '[0-9]{2}%' $LOG | awk '{print $4}' | sort -u > $LINGUAS
echo ""
echo "$LINGUAS:"
grep '#' $LINGUAS
echo $( grep -v '#' $LINGUAS )

exit

