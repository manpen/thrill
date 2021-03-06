################################################################################
# run/slurm/map_ib0.awk
#
# Part of Project Thrill - http://project-thrill.org
#
# Copyright (C) 2015 Timo Bingmann <tb@panthema.net>
#
# All rights reserved. Published under the BSD-2 license in the LICENSE file.
################################################################################

BEGIN { FS=" "; PORTBASE=51000 }
/^uc1/ {
    for(i = 1; i <= NF; i++) {
        id = gensub(/^uc1n([0-9]+)$/, "\\1", "", $i);
        if (id < 256) {
            printf("172.26.4.%d:%d ", id, PORTBASE + i)
        } else if (id < 512) {
            printf("172.26.5.%d:%d ", id - 256, PORTBASE + i)
        } else if (id < 768) {
            printf("172.26.6.%d:%d ", id - 512, PORTBASE + i)
        } else {
            exit -1
        }
    }
}
/^ic2/ {
    for(i = 1; i <= NF; i++) {
        id = gensub(/^ic2n([0-9]+)$/, "\\1", "", $i);
        if (id < 256) {
            printf("172.26.20.%d:%d ", id, PORTBASE + i)
        } else if (id < 512) {
            printf("172.26.21.%d:%d ", id - 256, PORTBASE + i)
        } else if (id < 768) {
            printf("172.26.22.%d:%d ", id - 512, PORTBASE + i)
        } else {
            exit -1
        }
    }
}

################################################################################
