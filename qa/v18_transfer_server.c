#include "nxt_transfer.h"
#include <stdio.h>
#include <assert.h>
#include <sys/resource.h>
int main(int argc,char**argv){assert(NxtTransfer_start(argv[1]));char url[128],pin[7];struct rusage u;NxtTransfer_status(url,sizeof(url),pin,sizeof(pin),NULL,NULL);getrusage(RUSAGE_SELF,&u);printf("%s %s %ld\n",url,pin,u.ru_maxrss);fflush(stdout);getchar();NxtTransfer_stop();getrusage(RUSAGE_SELF,&u);printf("STOPPED %ld\n",u.ru_maxrss);return 0;}
