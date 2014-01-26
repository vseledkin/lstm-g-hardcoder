/*gcc example.c -lm <- must explicitly link to math stuff?*/
#include "net.h"
int main(int argc, char **argv){
   net_save("dump.bin", 0);
   double input[11], *output, target[4], alpha;
   net_setInputPtr(input);
   output = net_getOutputPtr();
   net_setTargetPtr(target);
   net_setLearnRatePtr(&alpha);
   return 0;
}
