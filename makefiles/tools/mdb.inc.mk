FLASHER = $(RIOTTOOLS)/mdb/mdb.sh
RESET ?= $(RIOTTOOLS)/mdb/mdb.sh

FLASHFILE ?= $(HEXFILE)

FFLAGS ?= flash $(FLASHFILE)
RESET_FLAGS ?= reset

