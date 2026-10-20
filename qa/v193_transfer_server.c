#include "nxt_transfer.h"
#include <stdio.h>
#include <assert.h>
#include <sys/resource.h>
static int notices;void NxtLibrary_notify(void){__atomic_add_fetch(&notices,1,__ATOMIC_RELAXED);}
int main(int argc,char**argv){assert(NxtTransfer_start(argv[1]));char url[128],pin[7];struct rusage u;NxtTransfer_status(url,sizeof(url),pin,sizeof(pin),NULL,NULL);getrusage(RUSAGE_SELF,&u);printf("%s %s %ld\n",url,pin,u.ru_maxrss);fflush(stdout);getchar();NxtTransfer_stop();getrusage(RUSAGE_SELF,&u);printf("NOTICES %d STOPPED %ld\n",notices,u.ru_maxrss);return 0;}
