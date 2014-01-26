#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct line{
   int j, i, g;
   double w;
} line;
typedef struct assoc{
   int k, j;
} assoc;
typedef struct exTrace{
   int j, i, k;
} exTrace;
typedef struct exTrace2{
   int j, i, k, origIndex;
} exTrace2;
int lineOrder(const void *a, const void *b){
   if(((line *)a)->j == ((line *)b)->j)
      return ((line *)a)->i - ((line *)b)->i;
   return ((line *)a)->j - ((line *)b)->j;
}
int assocOrder(const void *a, const void *b){
   if(((assoc *)a)->j == ((assoc *)b)->j)
      return ((assoc *)a)->k - ((assoc *)b)->k;
   return ((assoc *)a)->j - ((assoc *)b)->j;
}
int exTrace2Order(const void *a, const void *b){
   if(((exTrace2 *)a)->j == ((exTrace2 *)b)->j){
      if(((exTrace2 *)a)->i == ((exTrace2 *)b)->i)
         return ((exTrace2 *)a)->k - ((exTrace2 *)b)->k;
      return ((exTrace2 *)a)->i - ((exTrace2 *)b)->i;
   }
   return ((exTrace2 *)a)->j - ((exTrace2 *)b)->j;
}
int main(int argc, char **argv){
   char name[strlen(argv[1]) + 1];
   strcpy(name, argv[1]);
   FILE *spec = fopen(name, "r");
   int j, i, g;
   double w;
   int numInputs, numOutputs, numUnits = 2;
   int numTraces = 0, numProjs = 0, numGatings = 0, numExTraces = 0;
   fscanf(spec, " %d , %d", &numInputs, &numOutputs);
   while(fscanf(spec, " %d , %d , %*e , %d", &j, &i, &g) != EOF){
      if(j >= numUnits)
         numUnits = j + 1;
      if(j != i)
         numTraces++;
      if(j > i)
         numProjs++;
      if(j != i && g >= 0 && j > g)
         numGatings++;
   }
   int selfConGaters[numUnits];
   int index = 0, index2 = 0, index3 = 0;
   while(index < numUnits)
      selfConGaters[index++] = -2; 
   line lines[numTraces];
   assoc projs[numProjs], gatings[numGatings];
   rewind(spec);
   fscanf(spec, " %*d , %*d");
   index = 0;
   while(fscanf(spec, " %d , %d , %le , %d", &j, &i, &w, &g) != EOF){
      if(j != i){
         lines[index].j = j;
         lines[index].i = i;
         lines[index].w = w;
         lines[index++].g = g;
         if(j > i){
            projs[index2].k = j;
            projs[index2++].j = i;
         }
         if(g >= 0 && j > g){//only for j != i?
            gatings[index3].k = j;
            gatings[index3++].j = g;
         }
      }
      else
         selfConGaters[j] = g;
   }

   //sizeof operates at compile time, so be sure to run lstm-g.c on the same architecture as the one(s) using the generated header
   qsort(lines, numTraces, sizeof(line), lineOrder);
   qsort(projs, numProjs, sizeof(assoc), assocOrder);
   qsort(gatings, numGatings, sizeof(assoc), assocOrder);
   int firstProjs[numUnits];
   j = -1;
   for(index = 0; index < numProjs; index++)
      if(projs[index].j > j){
         for(index2 = j + 1; index2 < projs[index].j;)
            firstProjs[index2++] = -1;
         j = projs[index].j;
         firstProjs[j] = index;
      }
   index2 = 1;
   for(index = 1; index < numGatings; index++)
      if(gatings[index].k > gatings[index - 1].k || gatings[index].j > gatings[index - 1].j){
         if(index2 == 41)
         gatings[index2].k = gatings[index].k;
         gatings[index2++].j = gatings[index].j;
      }
   numGatings = index2;
   int firstGatings[numUnits];
   j = -1;
   index2 = 0;
   for(index = 0; index < numGatings; index++){
      if(gatings[index].j > j){
         j = gatings[index].j;
         firstGatings[j] = index;
         index3 = 0;
         for(; index2 < numTraces && lines[index2].j < j; index2++);
         for(; index2 < numTraces && lines[index2].j == j; index2++)
            index3++;
      }
      numExTraces += index3;
   }
   index = strchr(name, '.') - name;
   strcpy(name + index + 1, "h");
   FILE *header = fopen(name, "w");
   strcpy(name + index, "_");
   char prefix[index + 1];
   strcpy(prefix, name);
   for(; index >= 0; index--)
      name[index] = toupper(name[index]);
   fprintf(header, "#ifndef %sH", name);
   fprintf(header, "\n#define %sH", name);
   fputs("\n#include <math.h>", header);
   fputs("\n#include <stdio.h>", header);
   fputs("\n#include <string.h>", header);
   fprintf(header, "\ndouble %sstates[%d];", prefix, numUnits - numInputs);
   fprintf(header, "\ndouble %sacts[%d];", prefix, numUnits);
   fprintf(header, "\ndouble %sweights[] = {", prefix);
   int firstLines[numUnits];
   j = lines[0].j;
   for(index2 = 0; index2 < j;)
      firstLines[index2++] = -1;
   firstLines[j] = 0;
   fprintf(header, "\n   %1.16e", lines[0].w);
   for(index = 1; index < numTraces;){
      if(lines[index].j > j){
         for(index2 = j + 1; index2 < lines[index].j;)
            firstLines[index2++] = -1;
         j = lines[index].j;
         firstLines[j] = index;
      }
      fprintf(header, ",\n   %1.16e", lines[index++].w);
   }
   exTrace exTraces[numExTraces];
   exTrace2 exTraces2[numExTraces];
   j = -1;
   index3 = 0;
   for(index = 0; index < numGatings; index++){
      if(gatings[index].j > j)
         j = gatings[index].j;
      if(firstLines[j] >= 0)
         for(index2 = firstLines[j]; index2 < numTraces && lines[index2].j == j; index3++){
            exTraces[index3].j = j;
            exTraces[index3].i = lines[index2++].i;
            exTraces[index3].k = gatings[index].k;
            exTraces2[index3].j = j;
            exTraces2[index3].i = exTraces2[index3].i;
            exTraces2[index3].k = exTraces2[index3].k;
            exTraces2[index3].origIndex = index3;
         }
   }
   qsort(exTraces2, numExTraces, sizeof(exTrace2), exTrace2Order);
   fputs("\n};", header);
   fprintf(header, "\ndouble %straces[%d];", prefix, numTraces);
   fprintf(header, "\ndouble %sexTraces[%d];", prefix, numExTraces);
   fprintf(header, "\ndouble %soldStates[%d];", prefix, numUnits - numInputs);
   fprintf(header, "\ndouble %soldActs[%d];", prefix, numTraces);
   fprintf(header, "\ndouble %soldGains[%d];", prefix, numUnits - numInputs + numTraces);
   fprintf(header, "\ndouble %sprojErrors[%d];", prefix, numUnits - numInputs - numOutputs);
   fprintf(header, "\ndouble %srespErrors[%d];", prefix, numUnits - numInputs);
   fprintf(header, "\ndouble *%sinput, *%starget, *%salpha;", prefix, prefix, prefix);
   fprintf(header, "\ndouble %stemp, %sactDeriv;", prefix, prefix);
   fprintf(header, "\nint %sindex;", prefix);
   fprintf(header, "\nFILE *%sfile;", prefix);
   fprintf(header, "\nvoid %ssave(char *filename, int onlyWeights){", prefix);
   fprintf(header, "\n   %sfile = fopen(filename, \"w\");", prefix);
   fprintf(header, "\n   for(%sindex = 0; %sindex < %d;)", prefix, prefix, numTraces);
   fprintf(header, "\n      fprintf(%sfile, \"%%1.16e\", %sweights[%sindex++]);", prefix, prefix, prefix);
   fputs("\n   if(!onlyWeights){", header);
   fprintf(header, "\n      for(%sindex = 0; %sindex < %d;)", prefix, prefix, numUnits - numInputs);
   fprintf(header, "\n         fprintf(%sfile, \"%%1.16e\", %sstates[%sindex++]);", prefix, prefix, prefix);
   fprintf(header, "\n      for(%sindex = 0; %sindex < %d;)", prefix, prefix, numTraces);
   fprintf(header, "\n         fprintf(%sfile, \"%%1.16e\", %straces[%sindex++]);", prefix, prefix, prefix);
   fprintf(header, "\n      for(%sindex = 0; %sindex < %d;)", prefix, prefix, numExTraces);
   fprintf(header, "\n         fprintf(%sfile, \"%%1.16e\", %sexTraces[%sindex++]);", prefix, prefix, prefix);
   fputs("\n   }", header);
   fprintf(header, "\n   fclose(%sfile);", prefix);
   fputs("\n}", header);
   fprintf(header, "\nvoid %sload(char *filename, int onlyWeights){", prefix);
   fprintf(header, "\n   %sfile = fopen(filename, \"r\");", prefix);
   fprintf(header, "\n   for(%sindex = 0; %sindex < %d;)", prefix, prefix, numTraces);
   fprintf(header, "\n      fscanf(%sfile, \"%%le\", %sweights[%sindex++]);", prefix, prefix, prefix);
   fputs("\n   if(!onlyWeights){", header);
   fprintf(header, "\n      for(%sindex = 0; %sindex < %d;)", prefix, prefix, numUnits - numInputs);
   fprintf(header, "\n         fscanf(%sfile, \"%%le\", %sstates[%sindex++]);", prefix, prefix, prefix);
   fprintf(header, "\n      for(%sindex = 0; %sindex < %d;)", prefix, prefix, numTraces);
   fprintf(header, "\n         fscanf(%sfile, \"%%le\", %straces[%sindex++]);", prefix, prefix, prefix);
   fprintf(header, "\n      for(%sindex = 0; %sindex < %d;)", prefix, prefix, numExTraces);
   fprintf(header, "\n         fscanf(%sfile, \"%%le\", %sexTraces[%sindex++]);", prefix, prefix, prefix);
   fputs("\n   }", header);
   fprintf(header, "\n   fclose(%sfile);", prefix);
   fputs("\n}", header);
   fprintf(header, "\nvoid %ssetInputPtr(double *inputPtr){", prefix);
   fprintf(header, "\n   %sinput = inputPtr;", prefix);
   fputs("\n}", header);
   fprintf(header, "\ndouble *%sgetOutputPtr(){", prefix);
   fprintf(header, "\n   return %sacts + %d;", prefix, numUnits - numOutputs);
   fputs("\n}", header);
   fprintf(header, "\nvoid %ssetTargetPtr(double *targetPtr){", prefix);
   fprintf(header, "\n   %starget = targetPtr;", prefix);
   fputs("\n}", header);
   fprintf(header, "\nvoid %ssetLearnRatePtr(double *alphaPtr){", prefix);
   fprintf(header, "\n   %salpha = alphaPtr;", prefix);
   fputs("\n}", header);
   fprintf(header, "\nvoid %sfastStep(){", prefix);
   fprintf(header, "\n   memcpy(%sacts, %sinput, %d);", prefix, prefix, sizeof(double) * numInputs);
   for(j = numInputs; j < numUnits; j++){
      if(selfConGaters[j] < -1)
         fprintf(header, "\n   %sstates[%d] = 0;", prefix, j - numInputs);
      else if(selfConGaters[j] >= 0)
         fprintf(header, "\n   %sstates[%d] *= %sacts[%d];", prefix, j - numInputs, prefix, selfConGaters[j]);
      index2 = -1;
      for(index = firstLines[j]; index < numTraces && lines[index].j == j; index++){
         if(selfConGaters[j] > -2 && lines[index].g < 0 && lines[index].i < numInputs)
            index2 = lines[index].i;
         else{
            fprintf(header, "\n   %sstates[%d] += ", prefix, j - numInputs);
            if(lines[index].g >= 0)
               fprintf(header, "%sacts[%d] * ", prefix, lines[index].g);
            fprintf(header, "%sweights[%d] * %sacts[%d];", prefix, index, prefix, lines[index].i);
         }
      }
      fprintf(header, "\n   %sacts[%d] = 1 / (1 + exp(-%sstates[%d]", prefix, j, prefix, j - numInputs);
      if(index2 >= 0)
         fprintf(header, " - %sacts[%d]", prefix, index2);
      fputs("));", header);
   }
   fputs("\n}", header);
   fprintf(header, "\nvoid %sfullStep(){", prefix);
   fprintf(header, "\n   memcpy(%sacts, %sinput, %d);", prefix, prefix, sizeof(double) * numInputs);
   fprintf(header, "\n   memcpy(%soldStates, %sstates, %d);", prefix, prefix, sizeof(double) * (numUnits - numInputs));
   for(j = numInputs; j < numUnits; j++){
      if(selfConGaters[j] >= 0)
         fprintf(header, "\n   %soldGains[%d] = %sacts[%d];", prefix, j - numInputs, prefix, selfConGaters[j]);
      for(index = firstLines[j]; index < numTraces && lines[index].j == j; index++){
         fprintf(header, "\n   %soldActs[%d] = %sacts[%d];", prefix, index, prefix, lines[index].i);
         if(lines[index].g >= 0)
            fprintf(header, "\n   %soldGains[%d] = %sacts[%d]", prefix, numUnits - numInputs + index, prefix, lines[index].g);
      }
      if(selfConGaters[j] < -1)
         fprintf(header, "\n   %sstates[%d] = 0;", prefix, j - numInputs);
      else if(selfConGaters[j] >= 0)
         fprintf(header, "\n   %sstates[%d] *= %sacts[%d];", prefix, j - numInputs, prefix, selfConGaters[j]);
      index2 = -1;
      for(index = firstLines[j]; index < numTraces && lines[index].j == j; index++){
         if(selfConGaters[j] > -2 && lines[index].g < 0 && lines[index].i < numInputs){
            index2 = lines[index].i;
            fprintf(header, "\n   %straces[%d] = %sacts[%d];", prefix, index, prefix, index2);
         }
         else{
            fprintf(header, "\n   %sstates[%d] += ", prefix, j - numInputs);
            if(lines[index].g >= 0)
               fprintf(header, "%sacts[%d] * ", prefix, lines[index].g);
            fprintf(header, "%sweights[%d] * %sacts[%d];", prefix, index, prefix, lines[index].i);
            fprintf(header, "\n   %straces[%d] ", prefix, index);
            if(selfConGaters[j] == -1)
               fputs("+", header);
            fputs("= ", header);
            if(selfConGaters[j] >= 0)
               fprintf(header, "%soldGains[%d] * %straces[%d] + ", prefix, j - numInputs, prefix, index);
            if(lines[index].g >= 0)
               fprintf(header, "%soldGains[%d] * ", prefix, numUnits - numInputs + index);
            fprintf(header, "%soldActs[%d];", prefix, index);
         }
      }
      fprintf(header, "\n   %sacts[%d] = 1 / (1 + exp(-%sstates[%d]", prefix, j, prefix, j - numInputs);
      if(index2 >= 0)
         fprintf(header, " - %sacts[%d]", prefix, index2);
      fputs("));", header);
   }
   j = 0;
   g = 0;
   for(index = 0; index < numExTraces; index++){
      if(exTraces[index].j > j){
         j = exTraces[index].j;
         index2 = 0;
         fprintf(header, "\n   %sactDeriv = %sacts[%d] * (1 - %sacts[%d]);", prefix, prefix, j, prefix, j);
      }
      if(exTraces[index].k != g){
         g = exTraces[index].k;
         index2 = 0;
         if(selfConGaters[g] == j)
            fprintf(header, "\n   %stemp = %soldStates[%d];", prefix, prefix, g - numInputs);
         else
            fprintf(header, "\n   %stemp = 0;", prefix);
         for(index3 = firstLines[g]; index3 < numTraces && lines[index3].j == g; index3++)
            if(lines[index3].g == j)
               fprintf(header, "\n   %stemp += %sweights[%d] * %soldActs[%d];", prefix, prefix, index3, prefix, index3);
      }
      fprintf(header, "\n   %sexTraces[%d] ", prefix, index);
      if(selfConGaters[g] == -1)
         fputs("+", header);
      fputs("= ", header);
      if(selfConGaters[g] >= 0)
         fprintf(header, "%soldGains[%d] * %sexTraces[%d] + ", prefix, g - numInputs, prefix, index);
      fprintf(header, "%sactDeriv * %straces[%d] * %stemp;", prefix, prefix, firstLines[j] + index2++, prefix);//temp may be 0? (optimization possible elsewhere too)
   }
   fputs("\n}", header);
   fprintf(header, "\ndouble %serror(){", prefix);
   fprintf(header, "\n   %stemp = 0;", prefix);
   for(index = 0; index < numOutputs; index++)
      fprintf(header, "\n   %stemp += %starget[%d] * log(%sacts[%d]) + (1 - %starget[%d]) * log(1 - %sacts[%d]);", prefix, prefix, index, prefix, numUnits - numOutputs + index, prefix, index, prefix, numUnits - numOutputs + index);
   fprintf(header, "\n   return %stemp;", prefix);
   fputs("\n}", header);
   fprintf(header, "\nvoid %slearn(){", prefix);
   for(index = 0; index < numOutputs; index++)
      fprintf(header, "\n   %srespErrors[%d] = %starget[%d] - %sacts[%d];", prefix, numUnits - numInputs - numOutputs + index, prefix, index, prefix, numUnits - numOutputs + index);
   for(j = numUnits - numOutputs - 1; j >= numInputs; j--){
      fprintf(header, "\n   %sprojErrors[%d] = 0;", prefix, j - numInputs);
      for(index = firstProjs[j]; index < numProjs && projs[index].j == j; index++){
         fprintf(header, "\n   %sprojErrors[%d] += %srespErrors[%d] * ", prefix, j - numInputs, prefix, projs[index].k - numInputs);
         for(index2 = firstLines[projs[index].k]; index2 < numTraces && lines[index2].i < j; index2++);
         if(lines[index2].g >= 0)
            fprintf(header, "%sacts[%d] * ", prefix, lines[index2].g);
         fprintf(header, "%sweights[%d];", prefix, index2);
      }
      fprintf(header, "\n   %srespErrors[%d] = 0;", prefix, j - numInputs);
      for(index = firstGatings[j]; index < numGatings && gatings[index].j == j; index++){
         g = gatings[index].k;
         if(selfConGaters[g] == j)
            fprintf(header, "\n   %stemp = %sstates[%d];", prefix, prefix, g - numInputs);
         else
            fprintf(header, "\n   %stemp = 0;", prefix);
         for(index2 = firstLines[g]; index2 < numTraces && lines[index2].j == g; index2++)
            if(lines[index2].g == j)
               fprintf(header, "\n   %stemp += %sweights[%d] * %sacts[%d];", prefix, prefix, index2, prefix, lines[index2].i);
         fprintf(header, "\n   %srespErrors[%d] += %srespErrors[%d] * %stemp;", prefix, j - numInputs, prefix, g - numInputs, prefix);//why is respErrors index not changing?
      }
      fprintf(header, "\n   %sactDeriv = %sacts[%d] * (1 - %sacts[%d]);", prefix, prefix, j - numInputs, prefix, j - numInputs);
      fprintf(header, "\n   %sprojErrors[%d] *= %sactDeriv;", prefix, j - numInputs, prefix);
      fprintf(header, "\n   %srespErrors[%d] = %sprojErrors[%d] + %sactDeriv + %srespErrors[%d];", prefix, j - numInputs, prefix, j - numInputs, prefix, prefix, j - numInputs);
   }
   index2 = 0;
   for(index = 0; lines[index].j < numUnits - numOutputs; index++){
      fprintf(header, "\n   %stemp = %sprojErrors[%d] * %straces[%d];", prefix, prefix, lines[index].j - numInputs, prefix, index);
      for(; index2 < numExTraces && exTraces2[index2].j == lines[index].j && exTraces2[index2].i == lines[index].i; index2++)
         fprintf(header, "\n   %stemp += %srespErrors[%d] * %sexTraces[%d];", prefix, prefix, exTraces2[index2].k - numInputs, prefix, exTraces2[index2].origIndex);//why is respErrors index negative?
      fprintf(header, "\n   %sweights[%d] += *%salpha * %stemp;", prefix, index, prefix, prefix);
   }
   for(; index < numTraces; index++)
      fprintf(header, "\n   %sweights[%d] += *%salpha * %srespErrors[%d] * %straces[%d];", prefix, index, prefix, prefix, lines[index].j - numInputs, prefix, index);
   fputs("\n}", header);
   fputs("\n#endif", header);
   return 0;
}
