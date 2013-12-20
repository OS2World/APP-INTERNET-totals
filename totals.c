/****************************************************************************
 totals.c - totals for file transfers made via Penguin FTP Daemon
 3/16/95, Tom Sorensen (tom@wc51.residence.gatech.edu)

 This file is hereby released into the public domain. The author would like
 to know of any improvements/suggestions/use of this program though.

 No warranties are expressely given or implied by the author. The author
 disclaims responsibility for any damage caused by proper or improper use
 of this program.

 Version History:
    0.9  - only totaled overall statistics
    0.95 - added daily totals, single day total, overall total suppression
    1.0  - added firstday info

 Penguin FTP Daemon is copyright 1995, Paul Wagland.
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define INCL_DOSNLS
#include <os2.h>

#define SINGLE 0x80
#define ALL 0x40
#define SHOWTOTALS 0x20

void perr(int);

FILE *tfile;
#define LINELEN 128
char str1[LINELEN], *str2, *filename;
char firstday[7], curday[7], flags, matchday[7];
unsigned long int sumsent=0, sumgot=0, daysent=0, daygot=0;
int lineno=0, i;

ULONG CpList[8], CpSize;
PCOUNTRYCODE pStructure;
PCOUNTRYINFO pMemBuff;
ULONG DataLength;
APIRET rc;

int main(int argc, char *argv[])
{
    if (argc==1) {
        perr(1);
    }
    filename=NULL;

    flags=SHOWTOTALS;                /* init to show totals */
    for (i=1;i<argc;i++) {
        if ((argv[i][0]=='-') || (argv[i][0]=='/')) {
            switch(argv[i][1]) {         /* parse switches */
                case 'd':
                    flags |= SINGLE;
                    strcpy(matchday,&(argv[i][2]));     /* specific date */
                    break;
                case 'D':
                    flags |= ALL;          /* all dates */
                    break;
                case 't':
                    flags &= ~SHOWTOTALS;    /* suppress totals */
                    break;
                case '?':
                    perr(1);
                default:
                    fprintf(stderr,"Invalid switch %c\n",argv[i][1]);
                    perr(1);
            } /* switch */
        } else {
            if ((filename=malloc(strlen(argv[i])+1)) == NULL) perr(2);
            strcpy(filename,argv[i]);       /* hopefully the file name */
            if (filename[strlen(filename)-1]=='\r') filename[strlen(filename)-1]='\0';
        } /* else */
    } /* for */

    if (filename==NULL) perr(1);
    if ((tfile = fopen(filename,"r")) == NULL) perr(3);

    rc = DosQueryCp(sizeof(CpList), CpList, &CpSize);   /* get codepage info */
    if (rc != 0) perr(4);

    CpList[0]=0;
    pStructure = &CpList;
    if ((pMemBuff = malloc(sizeof(COUNTRYINFO))) == NULL) perr(2);
    rc = DosQueryCtryInfo(sizeof(COUNTRYINFO), pStructure, pMemBuff, &DataLength);        /* get values for the codepage returned by DosQueryCp */
    if (rc != 0) perr(6);
    firstday[0]='\0';

    while (!feof(tfile)) {
        if (fgets(str1,LINELEN,tfile)==NULL) break; /* get line, NULL is EOF */
        lineno++;
        if ((strtok(str1, ":") == NULL) || ((str2=strtok('\0'," "))==NULL)) return(7);
        if (firstday[0]=='\0') strcpy(firstday,str2);
        if (strcmp(curday,str2)) {
            sumsent+=daysent;
            sumgot+=daygot;
            if ((flags & ALL) || ((flags & SINGLE) && !(strcmp(matchday,curday))))
                printf("Statistics for %c%c%c%c%c%c%c%c\nbytes sent: %15ld\n\
bytes received: %11ld\n",curday[0],curday[1],
                    pMemBuff->szDateSeparator[0],curday[2],curday[3],
                    pMemBuff->szDateSeparator[0],curday[4],curday[5],daysent,
                    daygot);
            strcpy(curday,str2);
            daysent=0;
            daygot=0;
        }

        if ((strtok('\0'," ")==NULL) || (strtok('\0'," ")==NULL) ||
                ((str2 = strtok('\0'," "))==NULL)) perr(8);

        if (!strcmp(str2,"received")) {
            if ((str2=strtok('\0'," "))==NULL) perr(9);
            daysent+=atoi(str2);
        } else if (!strcmp(str2,"sent")) {
            if ((str2=strtok('\0'," "))==NULL) perr(9);
            daygot+=atoi(str2);
        } else fprintf(stderr,"Bogus string at line %d, ignoring\n",lineno);
    } /* while */
    sumsent+=daysent;
    sumgot+=daygot;
            if ((flags & ALL) || ((flags & SINGLE) && !(strcmp(matchday,curday))))
        printf("Statistics for %c%c%c%c%c%c%c%c\nbytes sent: %15ld\n\
bytes received: %11ld\n",curday[0],curday[1],
        pMemBuff->szDateSeparator[0],curday[2],curday[3],
        pMemBuff->szDateSeparator[0],curday[4],curday[5],daysent,
        daygot);
    if (flags & SHOWTOTALS)
        printf("Statistics since %c%c%c%c%c%c%c%c\n",firstday[0],firstday[1],
                pMemBuff->szDateSeparator[0],firstday[2],firstday[3],
                pMemBuff->szDateSeparator[0],firstday[4],firstday[5]);
        printf("Total bytes sent: %15ld\nTotal bytes received: %11ld\n",sumsent,sumgot);
    return(0);
} /* main */

void perr(int errnum)
{
    switch(errnum) {
        case 1:
            fprintf(stderr,"Usage is totals [-d######] [-D] [-t] d:\\path\\transfer.log\n");
            fprintf(stderr,"-d###### - daily totals for single day, ###### is date\n");
            fprintf(stderr,"-D       - daily totals for all days\n");
            fprintf(stderr,"-t       - suppress overall totals\n");
            fprintf(stderr,"-?       - this information\n");
            fprintf(stderr,"\nBy Tom Sorensen (tom@wc51.residence.gatech.edu)\n");
            fprintf(stderr,"This program is in the public domain.\n");
            break;
        case 2:
            fprintf(stderr,"Could not allocate memory\n");
            break;
        case 3:
            fprintf(stderr,"Could not open %s to read!\n",filename);
            break;
        case 4:
            fprintf(stderr,"Error getting code page information.");
            break;
        case 6:
            fprintf(stderr,"Error retrieving country specific information.");
            break;
        case 7:
            fprintf(stderr,"Could not retrieve date from %s, line %d.\n(%s)\n",filename,lineno,str1);
            break;
        case 8:
            fprintf(stderr,"Could not tokenize %s, line %d correctly.\n(%s)\n",filename,lineno,str1);
            break;
        case 9:
            fprintf(stderr,"%d,Unexpected EOL\n",lineno);
            break;
    } /* switch */
    exit(errnum);
}
