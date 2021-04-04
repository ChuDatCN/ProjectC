#include<string.h>
#include<stdlib.h>
#include "../btree/inc/btree.h"
#include "../btree/inc/bc.h"
#include "../btree/inc/bt.h"
#include "../btree/inc/btree_int.h"

KEYENT keyent;

KEYENT* custom_bsrhbk(BTint blk, char *key, int *loc, char *val, BTint *link1, BTint *link2, int *result){
    int quit,idx,lo,hi,md,ioerr,nkeys;
    KEYENT* keyentp = NULL;
    
    ioerr = brdblk(blk,&idx);
    if (idx < 0) {
        bterr("BSRHBK",QRDBLK,itostr(blk));
        goto fin;
    }
    nkeys = ((btact->memrec)+idx)->infblk[ZNKEYS];
#if DEBUG > 0
    printf("BSRHBK: blk: " ZINTFMT ", nkeys: %d\n",blk,nkeys);
    printf("BSRHBK: loc: %d, val: " ZINTFMT ", link1: " ZINTFMT ", link2: "
           ZINTFMT "\n",*loc,*val,*link1,*link2);
#endif  
    if (*loc >= 0) {
        if (*loc >= nkeys) {
            /* requested loc not in block range */
            *result = -1;
            goto fin;
        }
        else {
            *result = 0;
            strncpy(key,((btact->memrec)+idx)->keyblk[*loc].key,ZKYLEN);
            key[ZKYLEN-1] = '\0';
        }
    }
    else {
        /* binary search */
        *loc = -1; /* init location at impossible position */
        *result = -1;
        quit = FALSE;
        lo = 0;
        hi = nkeys-1;
        md = -1;
        while (lo <= hi && !quit) {
            quit = (lo == hi);
            md = (lo+hi)/2;
            *result = strncmp(key,((btact->memrec)+idx)->keyblk[md].key, strlen(key)); /*locate*/
#if DEBUG >= 2
            printf("key: \"%s\" vs \"%s\"\n",key,
                   ((btact->memrec)+idx)->keyblk[md].key);
            printf("lo: %d, hi: %d, md: %d\n",lo,hi,md);
            printf("result: %d, quit: %d\n",*result,quit);
#endif
            if (*result > 0) {
                lo = md+1;
            }
            else  if (*result < 0) {
                hi = md-1;
            }
            else {
                break;
            }
        }
        *loc = md;
    }

    if (*loc < 0) {
        *val = 0;
        *link1 = ZNULL;
        *link2 = ZNULL;
    }
    else {
        strcpy(val,((btact->memrec)+idx)->keyblk[*loc].key);
        *link1 = ((btact->memrec)+idx)->lnkblk[*loc];
        *link2 = ((btact->memrec)+idx)->lnkblk[*loc+1];
        keyent = ((btact->memrec)+idx)->keyblk[*loc];
        keyentp = &keyent;
#if DEBUG >=2
        fprintf(stderr,"BSRHBK: val: " ZINTFMT ", link1: " ZINTFMT ", link2: "
                ZINTFMT "\n",*val,*link1,*link2);
#endif      
    }
fin:
    return keyentp;
}

int custom_bfndky(BTA *b,char *key,char *val){
    BTint cblk, link1, link2, newblk;
    int index, result, nkeys, status;
    char lkey[ZKYLEN];
    BTint duplicates_allowed;
        
    bterr("",0,NULL);
    status = QNOKEY;
    if ((result=bvalap("BFNDKY",b)) != 0) return(result);

    btact = b;      /* set context pointer */

    if (btact->shared) {
        if (!block()) {
            bterr("BFNDKY",QBUSY,NULL);
            goto fin;
        }
    }

    /* take local copy of key, truncating if necessary */
    strncpy(lkey,key,ZKYLEN);
    lkey[ZKYLEN-1] = '\0';
    
    /* initialise stack etc */
    cblk = btact->cntxt->super.scroot;
    duplicates_allowed = bgtinf(cblk,ZMISC);
    bstkin();
    bclrlf();
    strncpy(btact->cntxt->lf.lfkey,lkey,ZKYLEN);
    btact->cntxt->lf.lfkey[ZKYLEN-1] = '\0';

    while (cblk != ZNULL) {
#if DEBUG >= 2
        fprintf(stderr,"BFNDKY: searching block " ZINTFMT "\n",cblk);
#endif      
        nkeys = bgtinf(cblk,ZNKEYS);
        if (nkeys == ZMXKEY && btact->cntxt->super.smode == 0) {
            /* split if block full and updating permitted */
            bsptbk(cblk,&newblk);
            if (newblk < 0) {
                bterr("BFNDKY",QSPLIT,NULL);
                break;
            }
            /* if split occured, then must re-examine parent */
            if (cblk != btact->cntxt->super.scroot) {
                index  = btact->cntxt->lf.lfpos;
                cblk = btact->cntxt->lf.lfblk;
                btact->cntxt->lf.lfpos = bpull();
                btact->cntxt->lf.lfblk = bpull();
            }
        }
        else {
            index = -1;
            bpush(btact->cntxt->lf.lfblk);
            bpush(btact->cntxt->lf.lfpos);
            custom_bsrhbk(cblk,lkey,&index,val,&link1,&link2,&result);
            btact->cntxt->lf.lfblk = cblk;
            btact->cntxt->lf.lfpos = index; /* if block is empty, leave lfpos at -1 */
            if (result < 0) {
                /* must examine left block */
                cblk = link1;
            }
            else if (result > 0) {
                /* must examine right block */
                cblk = link2;
                /* increment index to indicate this key "visited" */
                btact->cntxt->lf.lfpos++;
            }
            else {
                status = 0;
                btact->cntxt->lf.lfexct = TRUE;
                cblk = ZNULL;
            }
        }
    }
  fin:
    if (btact->shared) bulock();
    /* non-zero status indicates no such key found */
    if (status) bterr("BFNDKY",QNOKEY,lkey);
    return(btgerr());
}

int tabcomplete(BTA *b, char *key){/* Custom bfndky using custom bsrhbk to find prefix word */
    char val[3000];
    if(!custom_bfndky(b, key, val)){
        strcpy(key, val);
        return 1;
    }
    return 0;
};