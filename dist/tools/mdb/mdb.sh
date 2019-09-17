#!/bin/sh

# Unified Microchip Debugger tool script for RIOT
#
# Global environment variables used:
# MDB:
# MDB_HWTOOL:
# MDB_HWTOOL_INDEX:
#
# The script supports the following actions:
#
# flash:        flash <hexfile>
#
#               flash given hex format file to the target.
#
#               options:
#               <hexfile>:      path to the hex file that is flashed
#
# reset:        triggers a hardware reset of the target board
#
#
# @author       Francois Berder <fberder@outlook.fr>

: ${MDB:=/opt/microchip/mplabx/v5.25/mplab_platform/bin/mdb.sh}
: ${HWTOOL:=PICkit3}
: ${HWTOOL_INDEX:=0}

test_hexfile() {
    if [ ! -f "${HEXFILE}" ]; then
        echo "Error: Unable to locate HEXFILE"
        echo "       (${HEXFILE})"
        exit 1
    fi
}


do_flash() {
    HEXFILE=$1
    #test_config
    #test_serial
    test_hexfile
    # clear any existing contents in burn file
    /bin/echo -n "" > ${BINDIR}/burn.seg
    printf "device ${PICKIT_DEVICE}\n" >> ${BINDIR}/burn.seg
    printf "hwtool ${HWTOOL} -p ${HWTOOL_INDEX}\n" >> ${BINDIR}/burn.seg
    printf "program ${HEXFILE}\n" >> ${BINDIR}/burn.seg
    printf "quit\n" >> ${BINDIR}/burn.seg
    # flash device
    sh -c "${MDB} '${BINDIR}/burn.seg'"
}

do_reset() {
    # clear any existing contents in burn file
    /bin/echo -n "" > ${BINDIR}/burn.seg
    printf "device ${PICKIT_DEVICE}\n" >> ${BINDIR}/burn.seg
    printf "hwtool ${MDB_HWTOOL} -p ${MDB_HWTOOL_INDEX}\n" >> ${BINDIR}/burn.seg
    printf "Reset ${HEXFILE}\n" >> ${BINDIR}/burn.seg
    printf "quit\n" >> ${BINDIR}/burn.seg
    # reset device
    sh -c "${MDB} '${BINDIR}/burn.seg'"
}

#
# parameter dispatching
#
ACTION="$1"
shift # pop $1 from $@

case "${ACTION}" in
  flash)
    echo "### Flashing Target ###"
    do_flash "$@"
    ;;
  reset)
    echo "### Resetting Target ###"
    do_reset "$@"
    ;;
  *)
    echo "Usage: $0 {flash|reset}"
    echo "          flash <hexfile>"
    ;;
esac

