#include "sandbox.h"
#include "physics.h"
#ifdef _GUI
#include "graphics.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>


void printTime(Result result);
void printMeanTime(Result res);
void printBodies(Result res, Body** b, int gnumBodies);

static int exitState;
void sigintHandler(int sig_num) { exitState = 1; }
int loopCount = 0;


int main(int argc, char** argv) {
    /* Register signal handling */
	exitState = 0;
	signal(SIGINT, sigintHandler);
	
	/* Set variables from comandline arguments */
	unsigned int gnumBodies, numSteps, numWorkers; double theta;
	if(argc>1) gnumBodies = atoi(argv[1]); 
	else gnumBodies = 500; 
	if(argc>2) numSteps = atoi(argv[2]);
	else numSteps = 0xFFFFFFFF;
	if(argc>3) theta = atof(argv[3]);
	else theta = 0.7; 
	if(argc>4) numWorkers = atoi(argv[4]);
	else numWorkers = 1;
	
	omp_set_dynamic(0);
	omp_set_num_threads(numWorkers);
	n_threads = numWorkers;	

    /* Set variables from config.txt */
	double maxMass, bounds, maxSpeed, gravConst, dt;
	int fps, screenWidth, screenHeight, collisionType, GUI;
	char programName[20];
	FILE* file = fopen("config.txt", "r");
	fscanf(file, "screenWidth %d\nscreenHeight %d\nframerate %d\n", &screenWidth, &screenHeight, &fps);
	fscanf(file, "bounds %lf\nmaxMass %lg\ndt %lf\ngravConst %lg\n", &bounds, &maxMass, &dt, &gravConst);
	fscanf(file, "programName %s\ncollisionType %d\nGUI %d\n", programName, &collisionType, &GUI);
	fclose(file);
	
	/* print all initial parameters */
	printf("maxMass: %g\ngravConst: %g\ndt: %.2f\nbounds: %.2f\nfps: %d\nscreenWidth: %d\nscreenHeight: %d\nprogramName: %s\nGUI %d\n",
		maxMass, gravConst, dt, bounds, fps, screenWidth, screenHeight, programName, GUI);
	printf("gnumBodies: %u\nnumSteps: %u\ntheta: %.3lf\nnumWorker: %u\n", gnumBodies, numSteps, theta, numWorkers);
	
    /* instantiate and initialize modules and structures */
	Body** bodies = init_sandbox(gnumBodies, gravConst, bounds, maxMass);
  	Result result{NULL, 0, 0, 0, 0, 0}; 
	
#ifdef _GUI
  	Frame screen;
	if(GUI) screen.create(screenWidth, screenHeight, (float)screenWidth/bounds, fps, programName);
#endif
	/* Program main loop */
    while(!exitState && (numSteps > loopCount)) {

#ifdef _GUI
        /* Handle events */
		if(GUI) exitState = screen.handleEvents();
#endif
		/* Uppdate physics */
		updatePhysics(collisionType, theta, dt, &result);
		
#ifdef _GUI
		/* Render to screen */
		if(GUI) {
			screen.updateFrame(bodies, result.tree, gnumBodies, maxMass);
			result.guiTime += timer();
		}
#else
		/* Print to stdout */
		printBodies(result, bodies, gnumBodies); printf("\n\n");
		printTime(result); printf("\n\n");
#endif
		
		loopCount++;
    }
    
    /* cleanup and exit */
	quit_sandbox();
	printf("\n\n"); printTime(result); printf("\n\n");
    return EXIT_SUCCESS;
}


// print the tree and all bodies
void printBodies(Result res, Body** b, int gnumBodies) {
	printTree(res.tree); printf("\n");
	for(int i = 0; i < gnumBodies; i++) printf("body %d: %.3lf kg, (%.3lf N, %.3lf N), (%.3lf m, %.3lf m)\n", i, b[i]->mass, b[i]->force.x, b[i]->force.y,b[i]->position.x, b[i]->position.y );
	printf("\n\n");
}

// print all times in result
void printTime(Result res) {
	double totalTime = res.treeBuildTime + res.updateTime + res.positionTime + res.collisionTime + res.guiTime;
	printf("treeBuildingTime:\t%.8lf\n", res.treeBuildTime);
	printf("forceUpdateTime:\t%.8lf\n", res.updateTime);
	printf("positionUpdateTime:\t%.8lf\n", res.positionTime);
	printf("collisionHandlingTime:\t%.8lf\n", res.collisionTime);
	printf("renderingTime:\t\t%.8lf\n", res.guiTime);
	printf("loopCount:\t\t%d\n", loopCount);
	printf("totalTime:\t\t%.8lf\n", totalTime);
}

// print the mean time by dividing all times with loopCount 
void printMeanTime(Result res) {
	double totalTime = res.treeBuildTime + res.updateTime + res.positionTime + res.collisionTime + res.guiTime;
	printf("treeBuildingTime:\t%lf\n", res.treeBuildTime/(double)loopCount);
	printf("forceUpdateTime:\t%lf\n", res.updateTime/(double)loopCount);
	printf("positionUpdateTime:\t%lf\n", res.positionTime/(double)loopCount);
	printf("collisionHandlingTime:\t%lf\n", res.collisionTime/(double)loopCount);
	printf("renderingTime:\t\t%lf\n", res.guiTime/(double)loopCount);
	printf("loopCount:\t\t%d\n", loopCount);
	printf("meanTime:\t\t%lf\n", totalTime/(double)loopCount);
}
