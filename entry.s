.syntax unified
.code 32

.section .isr_vector

vectors:
# ldr pc, =ptr   is semi-position independent in a nice way that works in our favour.
# That is, you can base the code at any arbitrary location and then move this to wherever you need it
vecReset:       LDR PC, =mainEntry         /* 00  Supervisor  */
vecUndefined:   LDR PC, =isrUndefined      /* 04  Undefined   */
vecSuperCall:   LDR PC, =isrSuperCall      /* 08  Supervisor  */
vecPrefAbrt:    LDR PC, =isrPrftchAbrt     /* 0C  Abort       */
vecDataAbrt:    LDR PC, =isrDataAbrt       /* 10  Abort       */
vecNotUsed:     LDR PC, =isrNotUsed        /* 14  NA          */
vecIRQ:         LDR PC, =isrIRQ            /* 18  IRQ         */
vecFIC:         LDR PC, =isrFIQ            /* 1C  FIQ         */
# < The assembler will place a list of addresses here >









