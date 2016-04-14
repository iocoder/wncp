/* Upload the operating system to Nexys 2 ROM. */
/* By: Mostafa Abd El-Aziz, for 6502 FPGA computer project. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dpcdecl.h>
#include <dmgr.h>
#include <depp.h>

unsigned short ram_read(HIF hif, unsigned int addr) {
    BYTE addr1, addr2, addr3, data1, data2;
    /* read the address bytes */
    addr  = addr & 0xFFFFFFFE;
    addr1 = (addr>> 0)&0xFF;
    addr2 = (addr>> 8)&0xFF;
    addr3 = (addr>>16)&0xFF;
    DeppPutReg(hif, 0, 0x3F, fFalse);
    DeppPutReg(hif, 1, addr1, fFalse);
    DeppPutReg(hif, 2, addr2, fFalse);
    DeppPutReg(hif, 3, addr3, fFalse);
    DeppGetReg(hif, 6, &data1, fFalse);
    DeppGetReg(hif, 6, &data2, fFalse);
    return (data2<<8)|data1;
}

void ram_write(HIF hif, unsigned int addr, unsigned short data) {
    BYTE addr1, addr2, addr3, data1, data2;
    /* read the address bytes */
    addr  = addr & 0xFFFFFFFE;
    addr1 = (addr>> 0)&0xFF;
    addr2 = (addr>> 8)&0xFF;
    addr3 = (addr>>16)&0xFF;
    data1 = (data>> 0)&0xFF;
    data2 = (data>> 8)&0xFF;
    DeppPutReg(hif, 0, 0x3F, fFalse);
    DeppPutReg(hif, 1, addr1, fFalse);
    DeppPutReg(hif, 2, addr2, fFalse);
    DeppPutReg(hif, 3, addr3, fFalse);
    DeppPutReg(hif, 6, data1, fFalse);
    DeppPutReg(hif, 6, data2, fFalse);
}

unsigned short rom_read(HIF hif, unsigned int addr) {
    BYTE addr1, addr2, addr3, data1, data2;
    /* read the address bytes */
    addr  = addr & 0xFFFFFFFE;
    addr1 = (addr>> 0)&0xFF;
    addr2 = (addr>> 8)&0xFF;
    addr3 = (addr>>16)&0xFF;
    /* enable the chip and enter word mode */
    /* oe   = 1 -> output disabled
     * we   = 1 -> write disabled
     * mt-ce= 1 -> ram not selected
     * st-ce= 1 -> flash rom not selected
     * prg  = 1 -> module in programming
     * word = 1 -> word mode
     */
    DeppPutReg(hif, 0, 0x3F, fFalse);
    /* load memory address bus */
    DeppPutReg(hif, 1, addr1, fFalse);
    DeppPutReg(hif, 2, addr2, fFalse);
    DeppPutReg(hif, 3, addr3, fFalse);
    /* enter read array mode */
    DeppPutReg(hif, 4, 0xFF, fFalse);
    /* generate write sequence */
    DeppPutReg(hif, 0, 0x35, fFalse);
    DeppPutReg(hif, 0, 0x37, fFalse);
    DeppPutReg(hif, 0, 0x3F, fFalse);
    /* load memory address bus */
    DeppPutReg(hif, 1, addr1, fFalse);
    DeppPutReg(hif, 2, addr2, fFalse);
    DeppPutReg(hif, 3, addr3, fFalse);
    /* get data */
    DeppGetReg(hif, 7, &data1, fFalse);
    DeppGetReg(hif, 7, &data2, fFalse);
    /* return word */
    return (data2<<8)|data1;
}

void rom_write(HIF hif, unsigned int addr, unsigned short data) {
    BYTE addr1, addr2, addr3, data1, data2, SR;
    /* read address and data bytes */
    addr  = addr & 0xFFFFFFFE;
    addr1 = (addr>> 0)&0xFF;
    addr2 = (addr>> 8)&0xFF;
    addr3 = (addr>>16)&0xFF;
    data1 = (data>> 0)&0xFF;
    data2 = (data>> 8)&0xFF;
    /* enable the chip and enter word mode */
    DeppPutReg(hif, 0, 0x3F, fFalse);
    /* load memory address bus */
    DeppPutReg(hif, 1, addr1, fFalse);
    DeppPutReg(hif, 2, addr2, fFalse);
    DeppPutReg(hif, 3, addr3, fFalse);
    DeppPutReg(hif, 7, data1, fFalse);
    DeppPutReg(hif, 7, data2, fFalse);
}

void rom_write_bulk(HIF hif, unsigned int addr, BYTE *data, int size) {
    printf("Writing the software to Flash ROM...\n");
    BYTE addr1, addr2, addr3;
    /* read the address bytes */
    addr  = addr & 0xFFFFFFFE;
    addr1 = (addr>> 0)&0xFF;
    addr2 = (addr>> 8)&0xFF;
    addr3 = (addr>>16)&0xFF;
    /* enable the controller in word mode */
    DeppPutReg(hif, 0, 0x3F, fFalse);
    /* load address */
    DeppPutReg(hif, 1, addr1, fFalse);
    DeppPutReg(hif, 2, addr2, fFalse);
    DeppPutReg(hif, 3, addr3, fFalse);
    /* write data */
    DeppPutRegRepeat(hif, 7, data, size, fFalse);
}

int rom_verify_bulk(HIF hif, unsigned int addr, BYTE *data, int size) {
    BYTE addr1, addr2, addr3, *buf;
    int err = 0, i;
    printf("Verifying data...\n");
    /* allocate buffer */
    buf = malloc(size);
    /* read the address bytes */
    addr  = addr & 0xFFFFFFFE;
    addr1 = (addr>> 0)&0xFF;
    addr2 = (addr>> 8)&0xFF;
    addr3 = (addr>>16)&0xFF;
    /* enable the controller in word mode */
    DeppPutReg(hif, 0, 0x3F, fFalse);
    /* load memory address bus */
    DeppPutReg(hif, 1, addr1, fFalse);
    DeppPutReg(hif, 2, addr2, fFalse);
    DeppPutReg(hif, 3, addr3, fFalse);
    /* enter read array mode */
    DeppPutReg(hif, 4, 0xFF, fFalse);
    /* generate write sequence */
    DeppPutReg(hif, 0, 0x35, fFalse);
    DeppPutReg(hif, 0, 0x37, fFalse);
    DeppPutReg(hif, 0, 0x3F, fFalse);
    /* load address */
    DeppPutReg(hif, 1, addr1, fFalse);
    DeppPutReg(hif, 2, addr2, fFalse);
    DeppPutReg(hif, 3, addr3, fFalse);
    /* read data */
    DeppGetRegRepeat(hif, 7, buf, size, fFalse);
    /* compare */
    for (i = 0; i < size && !err; i++) {
        if (buf[i] != data[i]) {
            fprintf(stderr, "Error at address: 0x%06X.\n", addr+i);
            fprintf(stderr, "ROM reads 0x%02X, but it should be 0x%02X.\n",
                            buf[i], data[i]);
            err = -5;
        }
    }
    /* deallocate buffer */
    free(buf);
    /* done */
    return err;
}

void rom_erase(HIF hif) {
    /* there are 128 blocks */
    BYTE SR;
    int i;
    printf("Erasing ROM...\n");
    for (i = 0; i < 128; i++) {
        DeppPutReg(hif, 0, 0x3F, fFalse);
        DeppPutReg(hif, 1, 0,    fFalse);
        DeppPutReg(hif, 2, 0,    fFalse);
        DeppPutReg(hif, 3, i*2,  fFalse);
        DeppPutReg(hif, 4, 0x20, fFalse); /* erase */
        DeppPutReg(hif, 0, 0x15, fFalse); /* write sequence */
        DeppPutReg(hif, 0, 0x17, fFalse);
        DeppPutReg(hif, 0, 0x1F, fFalse);
        DeppPutReg(hif, 4, 0xD0, fFalse); /* confirm erase */
        DeppPutReg(hif, 0, 0x15, fFalse); /* write sequence */
        DeppPutReg(hif, 0, 0x17, fFalse);
        DeppPutReg(hif, 0, 0x1F, fFalse);
        do {
            DeppPutReg(hif, 0, 0x36, fFalse); /* read sequence */
            DeppGetReg(hif, 5, &SR,  fFalse);
            DeppPutReg(hif, 0, 0x3F, fFalse);
        } while (!(SR & 0x80));
    }
}

int main(int argc, char *argv[]) {
    HIF hif;
    FILE *f;
    BYTE *buf, *tbuf;
    unsigned int addr;
    unsigned int size;
    unsigned int cur = 0, blksize = 0x100000;
    int err = 0;
    /* check arguments */
    if (argc != 4 && !(argc == 2 && !strcmp(argv[1], "erase"))) {
        fprintf(stderr, "Error: Your arguments are invalid.\n");
        fprintf(stderr, "Usage: %s <file> BASE_ADDR SIZE\n", argv[0]);
        return -3;
    }
    /* try to open the device */
    if (!DmgrOpen(&hif, "Nexys2")) {
        fprintf(stderr, "Cannot open the device!\n");
        return -1;
    }
    /* enable EPP protocol */
    DeppEnable(hif);
    /* program file/erase ROM */
    if (strcmp(argv[1], "erase")) {
        /* read arguments */
        sscanf(argv[2], "%i", &addr);
        if ((cur=addr) % 2) {
            fprintf(stderr, "Error: Address must be even.\n");
            return -3;
        }
        sscanf(argv[3], "%i", &size);
        if (size % 2) {
            fprintf(stderr, "Error: Size must be even.\n");
            return -3;
        }
        /* open the file */
        if (!(f = fopen(argv[1], "r"))) {
            fprintf(stderr, "Error: cannot open the file.\n");
            return -4;
        }
        /* allocate memory */
        tbuf = buf = malloc(size);
        /* read the file */
        fread(buf, size, 1, f);
        /* close the file */
        fclose(f);
        /* upload file */
        printf("Uploading %s to flash ROM.\n", argv[1]);
        cur = addr;
        blksize = 0x100000;
        tbuf = buf;
        while (!err && cur < addr + size) {
            /* last block and not 1MB? */
            if (cur + blksize > addr + size)
                blksize = size % blksize;
            /* print info */
            printf("Address: 0x%06X, Size: 0x%06X\n", cur, blksize);
            /* write the file */
            rom_write_bulk(hif, cur, tbuf, blksize);
            /* update pointer */
            cur += blksize;
            tbuf += blksize;
        }
        /* Verifying */
        printf("Verifying all data...\n", argv[1]);
        cur = addr;
        blksize = 0x100000;
        tbuf = buf;
        while (!err && cur < addr + size) {
            /* last block and not 1MB? */
            if (cur + blksize > addr + size)
                blksize = size % blksize;
            /* print info */
            printf("Address: 0x%06X, Size: 0x%06X\n", cur, blksize);
            /* verify */
            err = rom_verify_bulk(hif, cur, tbuf, blksize);
            /* update pointer */
            cur += blksize;
            tbuf += blksize;
        }
        /* deallocate the buffer */
        free(buf);
    } else {
        /* erase the flash rom */
        rom_erase(hif);
    }
    /* disable EPP protocol */
    DeppDisable(hif);
    /* close the device */
    DmgrClose(hif);
    /* make the user feel happy if successful */
    if (!err)
        printf("The file %s was successfully written to ROM :D\n", argv[1]);
    /* exit */
    return err;
}
