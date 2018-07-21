
#include "sandbox.h"

static int numberOfBodies;
static Body** bodies;      
static double G;
static QuadTree* tree = NULL;

// deallocate the bodies array
void quit_sandbox() {
    freeBodyArray(bodies, numberOfBodies);
}


// Sets globals. Allocates array of bodies with size n
Body** init_sandbox(int n, double gravkonst, double d, double maxMass) {	
    numberOfBodies = n;
    G = gravkonst;
    bodies = allocateBodyArray(n);
    for (int i = 0; i < n; i++) bodies[i] = allocateBody();
	initBodiesRandom(bodies, n, d, maxMass);
    return bodies;
}



void updatePhysics(int collisionType, double theta, double dt, Result* res) {
	timer();
	if(tree) destroyQuadTree(tree); 
	tree = NULL;

	if(theta >= 1.0) {		
		updateForce(bodies, numberOfBodies, theta, G);
		res->updateTime += timer();
	}
	else {
		tree = buildTree(bodies, numberOfBodies);
		res->treeBuildTime += timer();
		updateForceBarnesHut(bodies, numberOfBodies, tree, theta, G);
		res->updateTime += timer();
	}
	
	updatePosition(bodies, numberOfBodies, dt);
	res->positionTime += timer();

	if(collisionType) {
		collisionHandling(bodies, numberOfBodies, collisionType);
		res->collisionTime += timer();
	}

	res->tree = tree;
}



static void initBodiesRandom(Body** b, int n, double d, double maxMass) {
    srand(time(NULL));
	double totalMass = 0;
    for (int i = 0; i < n; i++) {
        b[i]->mass = pow(maxMass,((double)rand()/(double)RAND_MAX));
		totalMass += b[i]->mass;
    }
	double escVel = escapeVelocity(totalMass, d/2, G);
	for(int i = 0; i < n; i++) {
		b[i]->position.x = d - ((double)rand()/(double)RAND_MAX)*d;
    b[i]->position.y = d - ((double)rand()/(double)RAND_MAX)*d;
		b[i]->velocity.x = ((double)rand()/(double)RAND_MAX)*escVel - escVel/2;
    b[i]->velocity.y = ((double)rand()/(double)RAND_MAX)*escVel - escVel/2;
		b[i]->radius = (pow(b[i]->mass*3/(3*3.14), 1.0/3.0))*( (double)rand()/(double)RAND_MAX + 0.5);
    b[i]->force.x = 0;
    b[i]->force.y = 0;
	}
}
