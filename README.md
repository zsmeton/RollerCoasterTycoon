Jeff Bezo's Private Amusement Park
Killa Chimpu, Blonde Python
Midterm Project / Roller Coaster Tycoon
A brief, high level description of what the program is / does

# Controls #
Q - Exits the game
ESC - Exits the game
X - toggle split screen
shift + 1 - set camera to Arc Ball Camera
shift + 2 - set camera to Free Camera

Arc Ball Camera
    WASD - Move around wandering character when the camera is on it
    click + drag - move camera around object
    ctrl + click + drag - Change distance of camera to object
    1 - target the wandering player
    2 - target the time stepping player
    3 - target the distance stepping player
Free Camera
    W,S - move forwards/backwards along axis
    click + drag - change heading
First Person View
    shift + 3 - toggle what character the camera is targeting

# Compiling #
Compile away

# Bugs/Implementation details #
Probably won't handle retina screens very well...

# File Format Description #
// The actual file will not contain comments
// Player's Faery Control Points
2 // Number of players with control points
4 // Number of control points
-5,0,5 // x,y,z
-5,5,5
0,5,-5
0,0,-5
4
-5,0,5
-5,5,5
0,5,-5
0,0,-5
# Bezier Surface Control Points #
16 // Number of control points
-100,7,-100 // x,y,z
-100,-10,-34
-100,2,32
-100,13,98
-34,-12,-100
-34,10,-34
-34,14,32
-34,5,98
32,-8,-100
32,14,-34
32,-19,32
32,-7,98
98,-13,-100
98,-9,-34
98,12,32
98,-2,98
// Bezier Roller Coaster Control Points
19 // number of control points
0,21,50 // Position x,y,z
60,41,60
60,21,30
45,21,30
// Environment Placing
3 // number of objects
Ball 1.0 0.0 // Object x z (in bezier surface coords 16 points -> [0,1] 64 -> [0,2])
Rock 0.0 0.0
Bush 0.5 0.7

# Responsibilites and Contributions #
Blonde Python - Snake Character (time incrementing coaster character), Roller Coaster, Environment (Balls) Presentation
Killa Chimpu - Cart Hero (wander and distance incrementing coaster character), Bezier Patch, Cameras,
               View Ports, File IO, Environment (Rock and Bush)

# Time #
50 - 80 Hours (couldn't even tell you)

# Lab #
Bezier Patch, Distance Movement, View ports all not covered super well (presentation did an okay job for theoretical
                                                                         side, but implementation... not at all)

# Fun #
So so so fun!!!





