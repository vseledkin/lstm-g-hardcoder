#include <stdio.h>
#include <stdlib.h>
#include <time.h>
FILE *file;
void connect(int j, int i, int g){

/*doesn't generate all doubles in [-.1, .1]?, not uniform*/
   fprintf(file, "\n%d, %d, %1.16e, %d", j, i, rand() * .2 / RAND_MAX - .1, g);
}
int main(int argc, char **argv){
   int numInputs = 11, numOutputs = 4, numUnits = 47, biasInput = 10, numBlocks = 8;
   int inputUnit, memoryBlock, outputUnit;
   srand(time(NULL));
   file = fopen("net.txt", "w");
   fprintf(file, "%d, %d", numInputs, numOutputs);

/*input and bias to output*/
   for(outputUnit = numUnits - numOutputs; outputUnit < numUnits; outputUnit++)
      for(inputUnit = 0; inputUnit < numInputs; inputUnit++)
         connect(outputUnit, inputUnit, -1);

   for(memoryBlock = 0; memoryBlock < numBlocks; memoryBlock++){
      int inputGate = numInputs + memoryBlock;
      int forgetGate = numInputs + memoryBlock + numBlocks;
      int memoryCell = numInputs + memoryBlock + 2 * numBlocks;
      int outputGate = numInputs + memoryBlock + 3 * numBlocks;

/*input to memory block*/
      for(inputUnit = 0; inputUnit < biasInput; inputUnit++){
         connect(inputGate, inputUnit, -1);
         connect(forgetGate, inputUnit, -1);
         connect(memoryCell, inputUnit, inputGate);
         connect(outputGate, inputUnit, -1);
      }

/*bias to memory block*/
      connect(inputGate, biasInput, -1);
      connect(forgetGate, biasInput, -1);
      connect(memoryCell, biasInput, -1);
      connect(outputGate, biasInput, -1);

/*peephole and self-connection*/
      connect(inputGate, memoryCell, -1);
      connect(forgetGate, memoryCell, -1);
      fprintf(file, "\n%d, %d, 1, %d", memoryCell, memoryCell, forgetGate);
      connect(outputGate, memoryCell, -1);

/*memory block to output*/
      for(outputUnit = numUnits - numOutputs; outputUnit < numUnits; outputUnit++)
         connect(outputUnit, memoryCell, outputGate);
   }
   return 0;
}
