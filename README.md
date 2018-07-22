# N-BodySim

Description:
------------
This program was written as a part of a university asignment in concurrent programming. It simulates gravity in a N-body system and visualise this using the SFML library. The task was to improve the efficiency of this simulation by using Barnes Hut algoritm and multi-threading.

GUI keybindings:
-----------------
Arrow Keys - accelerate view in that direction
A and S - zoom view in and out x2
SPACE - stop view motion
ESC - exit application
T - toogle tree view on/off. Only woorks when Barnes Hut is used.

config.txt parameters:
----------------------
    screenWitdth:   sets the screen resolution width
    screenHeight:   sets the screen resolution height
    framerate:      sets the framerate limit
    bounds:         sets the size of the starting area for all the bodies
    maxMass:        sets the max limit for body mass. can be both scientific notation or decimal (ie. 3.2E3 or 3200.0)
    dt:             sets the simulated time of every frame. large dt will result in lower accuracy and faster simulation
    gravConst:      sets the simulations universal constant of gravity G.
    programName:    The text in the top of the screen
    collisionType:  1 = elastic (bodies bounces of each other), 2 = inelastic (bodies merges), 0 = none (ghost bodies)
    GUI:            1 if GUI is to be used, 0 if no GUI


terminal launch parameters:
----------------------------
./nbody [gnumBodies] [numSteps] [theta] [numWorkers]

    uint gnumBodies:    The number of boides to simulate. default = 50
    uint numSteps:      Number of iterations before automatic quit. default = 0xFFFFFFFF
    float theta:        >= 1.0 sets the simulation to naive algoritm. < 1.0 uses 
                    Barnes Hut algoritm. if theta > nodeSize/distance is true
                    for a specific note and a specific body, the force acting on that
                    body will be calculated with the nodes total mass and center of
                    mass. nodeSize is the width of a node in the QuadTree and
                    distance is the distance between the nodes center of mass and
                    a body. The lower theta is, the more accurate the simulation. 
                    default = 1
    numWorkers:         threads/processes/level of parallellisation that will be used
                    to update the bodies positions.



Terminal controll:
-----------------------------
    ctrl-c (sigint):    exits the simulation gracefully



Code editing:
-----------------------
-   The main function holds all the print functions to stdOut. Comment out print
    there as needed.
    
-   The sandbox module handles all calls to physics and time calculations (except gui time).

-   physics module holds the acctual physical calculation algorithms

-   graphics module holds all rendering algoritms and event handling (except signals)




Programming Tasks:
--------------------------
-   Disjoint false dependencies in the physics calculation.
    This mean making shure temporary variables are not shared, but private to
    different processes. Reusing the same memory is not the goal of this program.
    We are not memory optimizing.
    (lec 2)
    
-   concude that the final program exhibits total correctness property.
    That is both the safety property and liveliness property are satisfied.
    Safety = Nothing bad ever happens
    liveliness = Something good eventually happens
    (lec 3)

-   At small distances and high speed the simulation becomes very inacurate resulting in
    funny slingshots. An improvement would be if the simulated time and frame time were separated so that
    simulation of the physics could be done at smaller time intervalls for bodies that are undergoing a high
    change in velocity and/or acceleration.
    
