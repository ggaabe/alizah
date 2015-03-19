#include <stdio.h>
#include <stdlib.h>
/*
int discreteLog(unsigned int i){
	unsigned char answer=-1;
	while(i!=0){
		answer++;
		i>>=1;
	}
	return answer; 
}
*/

/*
This is an experimental structure for storing and running neural networks. The key motivation was to avoid storing synapses explicitly, so we instead loop through all the neurons and, if they need to fire, fire them into their "neighbors", which are determined with a simple function.

We are currently using the cumulative successor function, which follows the continuous function f(x)=(x^2)/2+x, the integral of f(x)=x+1. We can do this simply by adding the current successor to the previous value, as per the definition of a summation.

This can be parallelized by splitting up the core array of neurons amongst n processors.
*/
#define numNeurons 100000 // total number of neurons. This should probably not change - we will instead set it at the beginning so that RAM is maxed out. Changing this dynamically will weaken or destroy memories and structures
#define inhibitoryInterval 5 // every ith neuron should be inhibitory (negative power [not actually, we'll just flip it when we get to one]). For reference, the human neocortex is about 20% inhibitory neurons, so inhibitoryInterval should be 5 to match this.
#define radius 16 // this can be a global constant, as it is here, or an attribute of each neuron. It specifies how many neurons above, and below, each neuron fires into. SHOULD BE A POWER OF 2. NEVERMIND WE'LL PROBABLY USE A SPECIAL FUNCTION
#define logRadius 4 // this is just so we can bitshift over to radius, and is why radius should be a power of 2

#define numFields 4
#define logNumFields 2
//fields of each neuron
#define accum 0 // Stored energy so far. Neurons can be made leaky by decrementing this at every time step. Obviously you don't need to actually index this
#define density 1 // How many repetitions of the successor function this neuron uses when firing. (e.g. d=3->1,1,1,2,2,2,3,3,3,4,4,4,...)
#define threshold 2 // How much energy this neuron needs before it will fire. When it fires, just decrement accum by threshold. Do so (efficiently!) within a single timestep until accum<threshold.
#define timestamp 3 // Timestamp from the last time this neuron fired. This is used to determine the time between fires

unsigned int table[numNeurons<<logNumFields]; // THIS IS THE BRAIN. Neurons are implicity stored here as a list of (here) 4 attributes: accum, power, threshold, and lastFired

int main(){
	printf("entered main\n");

	/*
	randomize!
	*/
	unsigned int *tableP=table+((numNeurons-1)<<logNumFields);
	while(tableP>=table){
		int randy=rand();
		*tableP=randy%6;
		*(tableP+density)=1+(randy>>3)%3;
		*(tableP+threshold)=1+(randy>>7)%3;
		*(tableP+timestamp)=0;
		tableP-=numFields;
	}
	/*
	*/

	unsigned int cTime=0; // keep track of the current timestep to timestap neuron firings
	while(1){ // THIS IS THE SECRET TO IMMORTALITY! MWAHAHAHA
		printf("cTime: %d\n",cTime);
		// in each timestep, we loop through each neuron in our array with the following:
		unsigned int *currN=table+((numNeurons-1)<<logNumFields); // we start at the last neuron and loop backwards. The bitshift multiplies our address by 4, which is the number of fields per neuron
		while(currN>=table){ // for each neuron

			unsigned int *cAccum=currN; // grab its accum
			unsigned int *cThresh=currN+threshold; // and its threshold
			if((*cAccum)>(*cThresh)){ // if it's over its threshold
				printf("firing %d!\n",currN);
				// calculate delay and reset timestamp
				unsigned int *cTimestamp=cAccum+timestamp;
				unsigned int tDiff=cTime-*cTimestamp; // density should decrease with longer delays
				*cTimestamp=cTime;
				unsigned int *prev=table+accum;
				unsigned int counter=1;
				while(counter<=radius){
					(*(prev+=(counter>>density)))++;
					if((unsigned int)currN%inhibitoryInterval){
						counter++;
					}else{
						counter--;
					}
				}

				//determine firepower. SHOULD WE "DIVIDE" ACCUM BY THRESHOLD, NO PROBABLY JUST SUBTRACT AND IT CAN FIRE AGAIN NEXT TIME. LESS-EFFICIENT BUT MORE BIOMEMETIC. OR, IF IT'S OVER ITS THRESHOLD WE CAN JUST FIRE THE WHOLE ACCUM!
				(*cAccum)=0; // reset its accum to zero.
			}
			currN-=numFields;
		}
		cTime++;
	}
}