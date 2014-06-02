/*
* Colonies by Dan Nicholson  - Edit ifdefs as appropriate
* NOTE: Several methods have been adapted from tutorials at: lazyfooproductions.com
*/
 
/*
 * Include files for SDL and SDL_image - update if necessary.
 */ 
#include "SDL/SDL.h"
#include "SDL/SDL_image.h"

#include <string>
#include <fstream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <vector>
#include <cmath>


/******************* MAIN VARIABLES ********************************/
//The attributes of the window
const int SCREEN_WIDTH = 1024;//1280;
const int SCREEN_HEIGHT = 768;//1024;
const int FULL_SCREEN_MODE = 1;
const int SCREEN_BPP = 32;

//The frame rate
const int FPS = 20;

//The dimensions of the level
const int LEVEL_WIDTH = 15000;
const int LEVEL_HEIGHT = 14990;

//Tile variables
const int TILE_WIDTH = 510;
const int TILE_HEIGHT = 357;
const int TOTAL_TILES = 1260;

// Gameplay variables
const int MAX_PLAYERS = 10;
const int MAX_PLANETS = 12;
const int SHOOTING_STARS = 25;


// Ship Types
const int LIGHT1 = 0;
const int HEAVY1 = 1;
const int MOTHER1 = 2;
const int LIGHT2 = 3;
const int HEAVY2 = 4;
const int MOTHER2 = 5;

// AI Personalities
const int SCOUT = 0;
const int GUARDIAN = 1;
const int HUNTER = 2;
const int MOTHERSHIP = 3;

//The surfaces that will be used
SDL_Surface *menu[4]; // Menu Textures
SDL_Surface *ships[8][16][2]; //A array storing the images of the ships in [type][angle][moving] order.
SDL_Surface *planetTex[4]; // Planet Textures
SDL_Surface *screen = NULL; // Screen representation
SDL_Surface *tileSheet = NULL; // Tile sheet
SDL_Surface *laser = NULL; // Laser texture
SDL_Surface *shieldTexture[8]; // Sheild textures
SDL_Surface *explosion[6][11]; // Explosion textures
SDL_Surface *displaybar[2]; // Display
SDL_Surface *healthbar[8]; // healthbar textures
SDL_Surface *shieldbar = NULL; // shieldbar texture
SDL_Surface *radar_screen[2]; // Radar texture
SDL_Surface *radar_dot[8]; // Radar dot texture
SDL_Surface *planet_owner[2]; // Planet owner textures
SDL_Surface *pod[16]; // Pod textures
SDL_Surface *shooting_star = NULL; // Shooting star texture
SDL_Surface *button[9][2]; // Button Textures (menu)

//Event structure
SDL_Event event; 

// Variables relating to the ships playing
int this_mothership; // Position in array of user's team's mothership
int enemy_mothership; // Position in array of enemy mothership 
int red_mother, blue_mother;
SDL_Rect mothership[2]; // Coords of both motherships (0 = friendly, 1 = enemy)

// Menu variables
bool paused = true;
int menu_position = 0; 
int active_button = 0;
int player_type = 0;

//Camera respresentation
SDL_Rect camera = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };

/********************************* CIRCLE STRUCTURE ***********************************/
//Structure of a circle (used in collision detection)
struct Circle
{
    int x, y; // X and Y location on map
    int r; // Radius
};
// Distance between two points
double distance( int x1, int y1, int x2, int y2 )
{
    return sqrt( pow( (double)x2 - (double)x1, 2 ) + pow( (double)y2 - (double)y1, 2 ) );
}
// Check collision between two circles
bool check_collision( Circle &A, Circle &B )
{
    //If the distance between the circles is less than the sum of their radii
    if( distance( A.x, A.y, B.x, B.y ) < ( A.r + B.r ) )
    {
        //The circles have collided
        return true;
    }
    return false;    
}
bool collision( SDL_Rect &A, SDL_Rect &B )
{
    //The sides of the rectangles
    int leftA, leftB;
    int rightA, rightB;
    int topA, topB;
    int bottomA, bottomB;

    //Calculate the sides of rect A
    leftA = A.x;
    rightA = A.x + A.w;
    topA = A.y;
    bottomA = A.y + A.h;
        
    //Calculate the sides of rect B
    leftB = B.x;
    rightB = B.x + B.w;
    topB = B.y;
    bottomB = B.y + B.h;
            
    //If any of the sides from A are outside of B
    if( bottomA <= topB )
    {
        return false;
    }
    
    if( topA >= bottomB )
    {
        return false;
    }
    
    if( rightA <= leftB )
    {
        return false;
    }
    
    if( leftA >= rightB )
    {
        return false;
    }
    
    //If none of the sides from A are outside B
    return true;
}

void apply_surface( int x, int y, SDL_Surface* source, SDL_Surface* destination)
{
    //Make a temporary rectangle to hold the offsets
    SDL_Rect Offset;
    SDL_Rect* clip = NULL;
    
    //Give the offsets to the rectangle
    Offset.x = x;
    Offset.y = y;
    
    //Blit surface
    SDL_BlitSurface( source, clip, destination, &Offset );
}

/************************************ TILE CLASS ****************************************/
class Tile
{
    private:
	//The image attached to the tile
    SDL_Surface *image;
    //Tile dimensions and location
    SDL_Rect box;
    
    public:
    Tile( int x, int y, SDL_Surface *img );
    //Show the tile
    void show( SDL_Surface *screen );
    //Return box
    SDL_Rect &get_box();
};
Tile::Tile( int x, int y, SDL_Surface *img )
{
    //Get the offsets
    box.x = x;
    box.y = y;

    //Get the image
	image = img;
    
    //Set the collision box
    box.w = img->w;
    box.h = img->h;
}

void Tile::show( SDL_Surface *screen )
{
    //If the tile is on screen
    if( collision( camera, box ) == true )
    {
        //Show the tile
        apply_surface( box.x - camera.x, box.y - camera.y, tileSheet, screen);    
    }
}    

SDL_Rect &Tile::get_box()
{
    return box;
}






/********************************************** PLANET CLASS **************************************/
/*
 * Defines the planets used in the game
*/
class Planet
{
	private:
	//Holds coordinates and dimensions of the collision box
	SDL_Rect box;
	Circle circle;
	//Number of resource points remaining
	int capture_time;
	// Type of planet
	int type;
	
	int owner;
	// X and Y Velocity
	int red_team, blue_team;
	
	public:
	Planet();
	// Displays planet on screen
	void show(SDL_Surface *screen);
	void create(int type, int x, int y);
	void capture(int strength, int team);
	void init(int i);
	int get_owner();
	SDL_Rect get_coords();
	Circle get_circle();
};

Planet::Planet()
{
}

int Planet::get_owner()
{
	return owner;
}
SDL_Rect Planet::get_coords()
{
	return box;
}
Circle Planet::get_circle()
{
	return circle;
}
void Planet::show( SDL_Surface *screen )
{    
    //Show the planet
	if (type > -1)
	{
		apply_surface( box.x - camera.x, box.y - camera.y, planetTex[type], screen);
	}
}

// Start planet in a rand location on screen with a rand velocity
void Planet::init(int i)
{
	type = i;
	red_team = 0;
	blue_team = 0;
	owner = 0;
	circle.x = box.x;
	circle.y = box.y;
	circle.r = box.w/2;
}

// Create planet to given type specifications
void Planet::create(int i, int x, int y)
{
	if (i == 0)
	{
		capture_time = 900;
		box.w = 366;
		box.h = 366;
		box.x = x;
		box.y = y;
		init(i);
	}
	if (i == 1)
	{
		capture_time = 1200;
		box.w = 409;
		box.h = 403;
		box.x = x;
		box.y = y;
		init(i);

	}

	if (i == 2)
	{
		capture_time = 900;
		box.w = 360;
		box.h = 360;
		box.x = x;
		box.y = y;
		init(i);

	}
	if (i == 3)
	{
		capture_time = 900;
		box.w = 360;
		box.h = 366;
		box.x = x;
		box.y = y;
		init(i);

	}
}

// Drain resources to a given value
void Planet::capture(int strength, int team)
{
	if (team == 1)
	{
		if (blue_team < capture_time) blue_team += strength;
		if (red_team > 0) red_team -= strength;
	}
	if (team == 2)
	{
		if (red_team < capture_time) red_team += strength;
		if (blue_team > 0 ) blue_team -= strength;
	}
	if (red_team >= capture_time) 
	{
		owner = 2;
	}
	if (blue_team >= capture_time) 
	{
		owner = 1;
	}
	
}



/******************************************** PROJECTILE CLASS ****************************************/
/*
  Projectile fired from ship
*/

class Projectile
{
    private:
    //The projectile's collision box
	SDL_Rect box;
    
	// Related velocities
	int xVel, yVel, distance, maxDistance;
	bool isActive;
	
	public:
    Projectile();
	void shoot(int x, int y, int velX, int velY);
	SDL_Rect get_coords();
	SDL_Rect next_coords();
	bool active();
	void move();
	void show();
	void destroy();
};
// Creates a new projectile
Projectile::Projectile()
{
	maxDistance = 5;
	isActive = false;
	distance = maxDistance;
	box.x = -1;
	box.y = -1;
	box.w = 0;
	box.h = 0;
}
// Destroys the projectile
void Projectile::destroy()
{
	xVel = 0;
	yVel = 0;
	box.x = 0;
	box.y = 0;
}
void Projectile::move()
{
	box.x += xVel;
	box.y += yVel;
	distance ++;
	// Make projectile inactive if max distance has been exceeded.
	if (distance >= maxDistance) isActive = false;
}
void Projectile::show()
{
	apply_surface( box.x - camera.x, box.y - camera.y, laser, screen);
}
// Returns true if projectile is active
bool Projectile::active()
{
	if (isActive){ return true;}
	else return false;
}
// Sets the projectile away
void Projectile::shoot( int x, int y, int velX, int velY)
{
	//Initialize the offsets
    box.x = x;
    box.y = y;
    box.w = 20;
    box.h = 20;

	distance = 0;
	isActive = true;
	//Initialize the velocity
	xVel = velX*3;
	yVel = velY*3;
	
}
SDL_Rect Projectile::get_coords()
{
	return box;
}

/********************************************* EXPLOSION CLASS ****************************************/
/*
  Explosion created by destroying a ship
*/
class Explosion
{
    private:
    //The Explosion's collision box
	SDL_Rect box;
    int stage;
	// Related velocities
	bool active;

	public:
    Explosion();
	void show(int x, int y, int type);
	void reset();
};
// Creates a new Explosion
Explosion::Explosion()
{
	stage = -1;
	active = false;
	box.x = -1;
	box.y = -1;
	box.w = 0;
	box.h = 0;
}

void Explosion::show(int x, int y, int type)
{
	if (stage == -1)
	{
		box.x = x;
		box.y = y;
		active = true;
		switch(type)
		{
			case 0:
			box.w = 110;
			box.h = 110;
			break;
			
			case 2:
			box.w = 470;
			box.h = 470;
			break;
			
			default: break;
		}
	}
	if(active)
	{
		stage++;
		if(type > 2) type -= 3;
		apply_surface( box.x - camera.x, box.y - camera.y, explosion[type][stage], screen);
		if (stage >= 7) active = false;
	}
	
}
void Explosion::reset()
{
	stage = -1;
}


/***************************************** ESCAPE POD CLASS ************************************/
/*
  Escape pod created when a ship is destroyed
*/

//The Pod that will move around on the screen
class Pod
{
    private:

    //The Pod's collision box
	Circle circle;
    int xVel, yVel, speed, team, angle, desired_angle, think;
	public:

	Pod();
	void set_team(int t);
	Circle get_coords();
	
	// Spawns the Pod with re-initialised settings
	void spawn(int x, int y);
	void destroy();
	void move();
	void show(SDL_Surface *screen);
	void reset();
	void set_angle(Circle one, Circle two);
	int get_distance(Circle one, Circle two);
};

Pod::Pod()
{
	
	circle.x = 0;
	circle.y = 0;
	circle.r = 0;
	speed = 60;
	think = 0;
}
int Pod::get_distance(Circle one, Circle two)
{
	int x = 0;
	int y = 0;
	Circle a = one;
	Circle b = two;
	
	// if a < b, subtract it from b, else vice versa
	if (a.x < b.x) x = b.x - a.x;
	else x = a.x - b.x;
	
	if (a.y < b.y) y = b.y - a.y;
	else y = a.y - b.y;
	
	return x + y;
}
void Pod::set_team(int t)
{
	if (t < 3) {team = 1;}
	else team = 2;
	
	circle.x = 0;
	circle.y = 0;
}
Circle Pod::get_coords()
{
	return circle;
}
void Pod::set_angle(Circle one, Circle two)
{
	int x;
	int y;
	if (one.x < two.x) x = two.x - one.x;
	else x = one.x - two.x;
	if (one.y < two.y) y = two.y - one.y;
	else y = one.y - two.y;

	if(one.x == two.x && one.y == two.y){}
	else if(one.x == two.x && one.y > two.y) angle = 0;
	else if (one.x < two.x && one.y == two.y) angle = 4;
	else if (one.x == two.x && one.y < two.y) angle = 8;
	else if (one.x > two.x && one.y == two.y) angle = 12;
	else
	{
		float radians = atan((float)x/(float)y);
	
		if (one.x < two.x && one.y > two.y){} //  add 0
		else if (one.x < two.x && one.y < two.y) radians += (float)1.57079633; // add 90
		else if (one.x > two.x && one.y < two.y) radians += (float)3.14159265; // add 180
		else if (one.x > two.x && one.y > two.y) radians = (float)6.28318531 - radians;// 4.71238898; // add 270
		
		desired_angle = (int)((float)radians / 0.392697);
	}

}
void Pod::move()
{
	Circle temp;
	int mship;
	if (team == 1) mship = blue_mother;
	else mship = red_mother;
	temp.x = mothership[mship].x + (mothership[mship].w/2);
	temp.y = mothership[mship].y + (mothership[mship].h/2);
	temp.r = 10;
	if (think < 0) think++;
	else 
	{
		think = -5;
		set_angle(circle, temp);
	}
	int rotate = 0;
	if (angle != desired_angle)
	{
		if (angle < desired_angle) rotate = 1;
		else rotate = -1;
		
	}
	else rotate = 0;
	// Rotate to meet desired angle
	if (rotate == -1)
	{
		if (angle-1 > -1) angle -= 1;
		else angle = 15;
	}
	if (rotate == 1)
	{
		if (angle+1 < 16) angle += 1;
		else angle = 0;
	}
	float rad = (float)0.39269908169872414 * ((angle+1) - 5);
	circle.x += (int)(cos(rad) * speed);
	circle.y += (int)(sin(rad) * speed); 
	if (get_distance(temp, circle) < 70) destroy();
}
void Pod::show( SDL_Surface *screen)
{
	if (angle < 0 || angle > 15) angle = 0;
	apply_surface(circle.x - camera.x ,circle.y - camera.y,pod[angle],screen);
}
void Pod::spawn(int x, int y)
{
	circle.x = x;
	circle.y = y;
	circle.r = 90;
}

void Pod::destroy()
{
	circle.r = 0;
}



/*********************************************** SHIP CLASS *******************************************/
/*
 Representation of a player's ship
*/

//The Ship that will move around on the screen
class Ship
{
    private:

    //The Ship's collision box
	SDL_Rect box;
	Circle circle;
    
	// The projectile that will be fired from the ship
	Projectile proj;
	Explosion explosion;
	Pod pod;
	
    //The movement components of the Ship
    int rotate, moving, angle, lastX, lastY, breaking;

	
	// Signals the state of the ship
	bool render;
	
	// The dimensions of the ship's collision box.
	int shipSize, speed, damage;
	
	// Signals the state of the shield
	int showShield;
	int recharge;
	
	bool shoot;
	
	// Ship stats (type = light, heavy or mothership = 1-3)
	int shield, health, type, max_shield, reveal;

	// Drift calculations
    double xDrift, yDrift, accelerating;
	
	// Variables dealing with AI
	bool ai, ai_shoot;
	int personality, desired_angle, ai_think;
	Circle target;

	
	public:

	Ship();
	
	int xVel, yVel;
	
	// Sets the type of the ship
	void set_type(int type);
	
	// Spawns the ship with re-initialised settings
	void spawn();
	
	//Sets the camera over the Ship
    void set_camera();
	
	// Returns true if render = 1
	bool do_render();
	
	// Returns the ship's current type
	int get_type();
	
	//Handles the input from the keyboard
    void handle_input( SDL_Event &event );
    
    //Moves the Ship after each loop repetition
    void move(Planet planets[], Ship players[]);
	
    
    //Shows the Ship on the screen
    void show( SDL_Surface *screen );
	
	//Displays the health and shield respresentation
	void show_health( SDL_Surface *screen);
		
	int get_damage();
	
	//Called when a ship takes damage
    void hurt(int damage);
	
	//Called to destroy ship
	void destroy();
	
	//Called to show the shield if it has power
	void show_shield();
	
	// Recharges the shield if i=1
	void recharge_shield(int i);
	
	// Returns the ship's box dimensions and coordinates
	SDL_Rect get_coords();
	Circle get_circle();
	Circle get_pod();
	// Returns the dimensions and coords of the projectile
	Projectile get_proj();
	
	// Destroys the projectile when called.
	void destroy_proj();
	void reveal_ship();
	void ai_move(Planet planets[], Ship players[]);
	int get_nearest(Planet planets[]);
	int get_nearest(Ship players[]);
	int get_team();
	bool enemyVisible(Ship players[]);
	int enemyInRange(Ship players[]);
	bool revealed();
	
	int get_distance(Circle one, Circle two);
	void get_heading();
	void set_angle(Circle one, Circle two);
	bool isOverPlanet(Planet planets[]);
	void ai_on();
	
};

Ship::Ship()
{

	render = false;
	recharge = 0;
	breaking = 0;
	ai = false;
	reveal = 0;
	ai_shoot = false;
	shoot = false;
	ai_think = 1;
	desired_angle = 0;
}
void Ship::ai_on()
{
	ai = true;
	
	
	if (type == 2 || type == 5)
	{
		personality = 3;
	}
	else
	{
		personality = rand()%3;
	}
}
void Ship::set_type(int t)
{
	type = t;
	if (type == 0)
	{
		shipSize = 110;
		max_shield = 100;
		shield = 200;
		health = 100;
		speed = 40;
		type = 0;
		damage = 5;
		render = true;
		spawn();
		explosion.reset();
	}
	else if (type == 1)
	{

		shipSize = 150;
		type = 1;
		max_shield = 200;
		shield = 200;
		health = 200;
		damage = 10;
		speed = 30;
		render = true;
		spawn();
		explosion.reset();
	}
	else if (type == 2)
	{

		shipSize = 470;
		type = 2;
		max_shield = 2000 *(MAX_PLAYERS/10); // Mothership's health is determined by number of potential enemies
		shield = 2000 *(MAX_PLAYERS/10);
		speed = 20;
		damage = 30;
		health = 2000 *(MAX_PLAYERS/10);
		render = true;
		spawn();
		explosion.reset();
	}
	else if (type == 3)
	{

		shipSize = 110;
		type = 3;
		max_shield = 100;
		shield = 200;
		health = 100;
		speed = 40;
		damage = 5;
		render = true;
		spawn();
		explosion.reset();
	}
	else if (type == 4)
	{

		shipSize = 150;
		type = 4;
		max_shield = 200;
		shield = 200;
		health = 200;
		speed =30;
		damage = 10;
		render = true;
		spawn();
		explosion.reset();
	}
	else if (type == 5)
	{

		shipSize = 470;
		type = 5;
		max_shield = 2000 *(MAX_PLAYERS/10);
		shield = 2000 *(MAX_PLAYERS/10);
		speed = 20;
		health = 2000 *(MAX_PLAYERS/10);
		damage = 30;
		render = true;
		spawn();
		explosion.reset();
	}

}

// Re-initialises variables to starting values
void Ship::spawn()
{
	angle = 0;
	rotate = 0;
	moving = 0;
	xVel = 0;
	yVel = 0;
	xDrift = 0;
	yDrift = 0;
	shoot = false;

	box.w = shipSize;
	box.h = shipSize; 
	// both motherships have been placed
	if (mothership[0].x > 0 && mothership[1].x > 0)
	{
		// if ship = 0 or 1
		if (type < 2)
		{
			// Spawn near mothership
			box.x = mothership[blue_mother].x + rand()%mothership[blue_mother].w/2;
			box.y = mothership[blue_mother].y + rand()%mothership[blue_mother].w/2;
		}
		// if ship == 3 or 4
		else if (type > 2 && type < 5)
		{
			box.x = mothership[red_mother].x + rand()%mothership[red_mother].w/2;
			box.y = mothership[red_mother].y + rand()%mothership[red_mother].w/2;
		}
		// If ship = 2 or 5
		else
		{
			// Spawn at rand
			box.x =  rand()%15000;
			while (box.x + box.w > LEVEL_WIDTH) 
			{
				box.x = rand()%15000;
			}
			
			box.y =  rand()%15000;
			while (box.y + box.h > LEVEL_HEIGHT)
			{
				box.y = rand()%15000;
			}
		}
	}	
	else
	{
		// Spawn motherships at rand
		box.x =  rand()%15000;
		while (box.x + box.w > LEVEL_WIDTH) 
		{
			box.x = rand()%15000;
		}
		
		box.y =  rand()%15000;
		while (box.y + box.h > LEVEL_HEIGHT)
		{
			box.y = rand()%15000;
		}
	}
	circle.x = box.x + shipSize/2;
	circle.y = box.y + shipSize/2;
	circle.r = shipSize/2;
	showShield = 0;
	
	accelerating = 0;	
	pod.set_team(type);	

}
int Ship::get_damage()
{
	return damage;
}
void Ship::set_camera()
{

	camera.x = ( box.x + box.w / 2 ) - SCREEN_WIDTH / 2;
	camera.y = ( box.y + box.h / 2 ) - SCREEN_HEIGHT / 2;
    
    //Keep the camera in bounds.
    if( camera.x < 0 )
    {
        camera.x = 0;    
    }
    if( camera.y < 0 )
    {
        camera.y = 0;    
    }
    if( camera.x > LEVEL_WIDTH - camera.w )
    {
        camera.x = LEVEL_WIDTH - camera.w;    
    }
    if( camera.y > LEVEL_HEIGHT - camera.h )
    {
        camera.y = LEVEL_HEIGHT - camera.h;    
    }    
}

bool Ship::do_render()
{
	if (render) return true;
	else return false;
}

int Ship::get_type()
{
	return type;
}

void Ship::handle_input( SDL_Event &event )
{
	
    //If a key was pressed
    if( event.type == SDL_KEYDOWN && render)
    {
        switch( event.key.keysym.sym )
        {
			// If up, move in current direction
            case SDLK_UP: 
			moving = 1;
			break;
			
			// If left, begin rotating anti-clockwise
            case SDLK_LEFT:
			rotate = -1;
			break;

			// If right, begin rotating clockwise
            case SDLK_RIGHT:
			rotate = 1;
			break;    
			
			// If down, decelerate ship
			case SDLK_DOWN:
			breaking = 1;
			break;

			// If spacebar is pressed, fire a shot in the direction the ship is currently facing
			case SDLK_SPACE:
			shoot = true;
			break;

			default: break;
        }
    }
    //If a key was released
    else if( event.type == SDL_KEYUP && render)
    {
        switch( event.key.keysym.sym )
        {
			// If up, stop moving and begin drifting
			case SDLK_UP: 
			moving = 0;
			yDrift += yVel;
			xDrift += xVel;
			yVel = 0;
			xVel = 0;
			accelerating = 0;
			break;
			
			// If left, stop rotating
			case SDLK_LEFT:
			rotate = 0;
			break;
			
			// If right, stop rotating
			case SDLK_RIGHT:
			rotate = 0;
			break;
			
			// If down, stop breaking
			case SDLK_DOWN:
			breaking = 0;
			break;
			
			// If space, stop shooting
			case SDLK_SPACE:
			shoot = false;
			break;
			
			default: break;
        }
    }
}

void Ship::move(Planet planets[], Ship players[])
{
	// If ai is active, switch to AI move
	if(ai){ai_move(planets, players);}
	else
	{
		// If ship is revealed, decrement time to be revealed
		if (reveal > 0)
		{
			reveal --;
		}
		// Recharge shield if necessary
		if (type == 2 || type == 5) recharge = 1;
		if (recharge == 1)
		{
			if (shield < max_shield) shield ++;
			recharge = 0;
		}
		
		// Update angle
		if (rotate == -1)
		{
			if (angle-1 > -1) angle -= 1;
			else angle = 15;
		}
		if (rotate == 1)
		{
			if (angle+1 < 16) angle += 1;
			else angle = 0;
		}	
		
		// Record current heading of ship and velocities
		float rad = (float)0.39269908169872414 * ((angle+1) - 5);
		if (accelerating < speed && moving == 1) accelerating ++;
		lastX = (int)(cos(rad) * accelerating);
		lastY = (int)(sin(rad) * accelerating); 
		
		// Shoot if pressed
		if (shoot)
		{
			int projX = (int)(cos(rad) * 30);
			int projY = (int)(sin(rad) * 30); 
			if (!proj.active()) proj.shoot(box.x + (shipSize/2), box.y + (shipSize/2), projX, projY); 
		}
		// Slow ship if breaking
		if (breaking == 1)
		{
			xVel = 0;
			yVel = 0;
			xDrift -= xDrift / 10;
			yDrift -= yDrift / 10;
		}
		// Move forward if player is pressing 'up'
		if (moving == 1)
		{
			xVel = lastX;
			yVel = lastY;
			// Cancel drift as quickly as acceleration is being applied to prevent 'boost'
			if (xDrift == 0.0){}
			else if (xDrift < 0.0) xDrift ++;
			else xDrift --;
			if (yDrift == 0.0){}
			else if (yDrift < 0.0) yDrift ++;
			else yDrift --;
		}
		else
		{
			// Calculate and apply drift
			if (xDrift == 0.0){}
			else if (xDrift < 0.0) xDrift += 0.2;
			else xDrift -= 0.2;
			if (yDrift == 0.0){}
			else if (yDrift < 0.0) yDrift += 0.2;
			else yDrift -= 0.2;
		}
		//If the Ship will go off the universe by moving in any direction, dont move in that direction
		if ((box.x + xVel + xDrift) < 0 || (box.x + xVel + xDrift + box.w) > LEVEL_WIDTH){}
		else 
		{	
			box.x += xVel + (Sint16)xDrift;
			circle.x = box.x + shipSize/2;

		}
		if ((box.y + yVel + yDrift) < 0 || (box.y + yVel + yDrift + box.h) > LEVEL_HEIGHT){}
		else 
		{
			box.y += yVel + (Sint16)yDrift;
			circle.y = box.y + shipSize/2;

		}
	}
	
}



void Ship::show( SDL_Surface *screen )
{    
	// Display projectile if it has been fired
	if (proj.active() && pod.get_coords().r == 0 && render)
	{
		proj.move();
		proj.show();
		
	}
	// Display pod if mothership exists and pod exists
	if (pod.get_coords().r == 0 || mothership[get_team()-1].w == 0){}
	else
	{
		pod.move();
		box.x = pod.get_coords().x;
		box.y = pod.get_coords().y;
		pod.show(screen);
	}
	//Show the Ship if it still exists on screen
	if (render && health > 0)
	{
		apply_surface( box.x - camera.x, box.y - camera.y, ships[type][angle][moving], screen);
	}
	else
	{
		// If type is fighter not mothership and mothership exists, spawn escape pod
		if (type != 2 && type != 5)
		{
			explosion.show(box.x, box.y, type);
		
			box.w = 0;
			box.h = 0;
			xDrift = 0;
			yDrift = 0;
			shoot = false;
			render = false;
			if (mothership[get_team()-1].w == 0){}
			else
			{
				if (pod.get_coords().x == 0 && pod.get_coords().y == 0)
				{
					pod.spawn(box.x,box.y);
				}
				if (pod.get_coords().r == 0 && !render && mothership[get_team()-1].w != 0)
				{
					if (ai)
					{
						set_type(type);
					}
					
				}
			}
			
		}
		else
		{
			explosion.show(box.x, box.y, type);
			box.w = 0;
			box.h = 0;
			xDrift = 0;
			yDrift = 0;
			render = false;
		}
		
	}
	// Display shield if it needs to be displayed and has energy left
	if (shield > 0 && showShield > 0)
	{
		apply_surface( box.x - camera.x, box.y - camera.y, shieldTexture[type], screen);
		showShield --;
	}
	
	if (reveal > 0) reveal --;
}
Circle Ship::get_pod()
{
	return pod.get_coords();
}
// Display health bar on screen
void Ship::show_health( SDL_Surface *screen)
{
	int display_width = SCREEN_WIDTH - 170;
	if (type < 3) apply_surface( display_width, 0, displaybar[0], screen);
	else apply_surface(display_width, 0, displaybar[1], screen);

	if (health < 100)
	{
		SDL_SetAlpha(healthbar[type], SDL_SRCALPHA, health*2);
	}
	else
	{
		SDL_SetAlpha(healthbar[type], SDL_SRCALPHA, 200);
	}
	apply_surface( display_width+10, 0, healthbar[type], screen);
	if (shield < 100)
	{
		SDL_SetAlpha(shieldbar, SDL_SRCALPHA, shield*2);
	}
	else
	{
		SDL_SetAlpha(shieldbar, SDL_SRCALPHA, 200);
	}
	apply_surface( display_width+10, 0, shieldbar, screen);
}
void Ship::hurt(int damage)
{
	show_shield();
	// If the shield is stronger than the damage, subtract damage from shield
	if (shield > damage)
	{
		shield -= damage;
	}
	// If shield has power left, subtract remainder from health and remove shield
	else if (shield > 0)
	{
		int newDamage = damage - shield;
		shield = 0;
		health -= newDamage;
	}
	// Otherwise, remove damage from health and destroy health <= 0
	else
	{
		if (health > damage) {health -= damage;}
		else destroy();
	}
	reveal_ship();
}

void Ship::destroy()
{
	render = false;
	health = 0;
	moving = 0;

	xVel = 0;
	yVel = 0;
	xDrift = 0;
	yDrift = 0;
	shield = 0;
}

void Ship::show_shield()
{
	showShield = 3;
}

void Ship::recharge_shield(int i)
{
	if (i == 0)
	{
		// If ship is not mothership, stop recharging
		if (type != 2 || type != 5)
		{
			recharge = 0;
		}
	}
	else recharge = 1;
}

SDL_Rect Ship::get_coords()
{
	return box;
}
Circle Ship::get_circle()
{
	return circle;
}
Projectile Ship::get_proj()
{
	return proj;
}
void Ship::destroy_proj()
{
	proj.destroy();
}

//// --- CODE RELATING TO ARTIFICIAL INTELLIGENCE FUNCTIONS --- ///
// AI-Move, called if ai is active.
void Ship::ai_move(Planet planets[], Ship players[])
{	
	if (ai_think >= 1 && render)
	{
		// If ship is a hunter
		if (personality == HUNTER)
		{
			// If there are any enemies on the radar, set target as nearest visible enemy
			if (enemyVisible(players))
			{
				int t =  get_nearest(players);
				target = (Circle&)(Circle const&)players[t].get_circle();
			}
			// Otherwise, set the target as the nearest planet that needs to be captured
			else
			{

				int p = get_nearest(planets);
				target = (Circle&)(Circle const&)planets[p].get_circle();
				target.x += planets[p].get_circle().r + rand()%planets[p].get_circle().r - rand()%planets[p].get_circle().r;
				target.y += planets[p].get_circle().r + rand()%planets[p].get_circle().r - rand()%planets[p].get_circle().r;				
			}
			// If an enemy is in-range, shoot at enemy
			if (enemyInRange(players) > -1)
			{
				int t = enemyInRange(players);
				target = (Circle&)(Circle const&)players[t].get_circle();
				ai_shoot = true;

			}
			else ai_shoot = false;
		}
		else if (personality == MOTHERSHIP)
		{
			// Target is nearest uncaptured planet
			int p = get_nearest(planets);
			target = (Circle&)(Circle const&)planets[p].get_circle();
			target.x += planets[p].get_circle().r;
			target.y += planets[p].get_circle().r;
		}
		else if (personality == GUARDIAN)
		{
			// Target is mothership
			target = (Circle&)(Circle const&)players[get_team()].get_circle();
			if (enemyInRange(players) > -1)
			{
				int t = enemyInRange(players);
				target = (Circle&)(Circle const&)players[t].get_circle();
				ai_shoot = true;
			}
			else ai_shoot = false;
		}
		else if (personality == SCOUT)
		{
			// Target is nearest uncaptured planet
			int p = get_nearest(planets);
			target = (Circle&)(Circle const&)planets[p].get_circle();
			target.x += planets[p].get_circle().r;
			target.y += planets[p].get_circle().r;
		
			if (enemyInRange(players) > -1 && isOverPlanet(planets))
			{
				int t = enemyInRange(players);
				target = (Circle&)(Circle const&)players[t].get_circle();
				ai_shoot = true;
			}
			else ai_shoot = false;
		}
	// Recreate delay on thinking
	ai_think = -3;
	set_angle(circle, target);
	}
	else ai_think++;
	
	if (angle != desired_angle)
	{
		if (angle < desired_angle) rotate = 1;
		else rotate = -1;
		
	}
	else rotate = 0;
	
	// Rotate to meet desired angle
	if (rotate == -1)
	{
		if (angle-1 > -1) angle -= 1;
		else angle = 15;
	}
	if (rotate == 1)
	{
		if (angle+1 < 16) angle += 1;
		else angle = 0;
	}
	
	// Set movement variables
	if (get_distance(circle, target) > target.r)
	{
		breaking = 0;
		moving = 1; 
	}
	else 
	{
		breaking = 1;
	}
	if (breaking == 1 && moving == 1)
	{
		xDrift = xVel;
		yDrift = yVel;
		moving = 0;
		accelerating = 0;
		xVel = 0;
		yVel = 0;
	}
	if (breaking == 1)
	{
		xDrift -= xDrift / 2;
		yDrift -= yDrift / 2;
	}
	// Move forward
	if (moving == 1)
	{
		// Calculate velocities
		float rad = (float)0.392697 * (angle-4);
		if (accelerating < speed) accelerating ++;
		lastX = (int)(cos(rad) * accelerating);
		lastY = (int)(sin(rad) * accelerating); 
		xVel = lastX;
		yVel = lastY;
		// Calculate and apply drift
		if (xDrift == 0.0){}
		else if (xDrift < 0.0) xDrift ++;
		else xDrift --;
		if (yDrift == 0.0){}
		else if (yDrift < 0.0) yDrift ++;
		else yDrift --;
	}
	else
	{
		// Calculate normal drift
		if (xDrift == 0.0){}
		else if (xDrift < 0.0) xDrift += 0.2;
		else xDrift -= 0.2;
		if (yDrift == 0.0){}
		else if (yDrift < 0.0) yDrift += 0.2;
		else yDrift -= 0.2;
	}
	if (!proj.active() && ai_shoot) 
	{
		float rad = (float)0.39269908169872414 * (angle-4);
		int projX = (int)(cos(rad) * 30);
		int projY = (int)(sin(rad) * 30); 
		proj.shoot(box.x + (shipSize/2), box.y + (shipSize/2), projX, projY);
	}
	if ((box.x + xVel + xDrift) < 0 || (box.x + xVel + xDrift + box.w) > LEVEL_WIDTH){}
	else 
	{	
		box.x +=xVel + (Sint16)xDrift;
		circle.x = box.x + shipSize/2;

	}
	if ((box.y + yVel + yDrift) < 0 || (box.y + yVel + yDrift + box.h) > LEVEL_HEIGHT){}
	else 
	{
		box.y +=yVel + (Sint16)yDrift;
		circle.y = box.y + shipSize/2;

	}
}
// Calculate the angle the ship should be facing given the target location and the ship's current location
// A Circle holds an x,y location and a radius (r).
// Circle 1 = ships location. Circle 2 = target location
void Ship::set_angle(Circle one, Circle two)
{
	int x;
	int y;
	if (one.x < two.x) x = two.x - one.x;
	else x = one.x - two.x;
	if (one.y < two.y) y = two.y - one.y;
	else y = one.y - two.y;
	
	if(one.x == two.x && one.y == two.y) desired_angle = angle;
	else if(one.x == two.x && one.y > two.y) desired_angle = 0;
	else if (one.x < two.x && one.y == two.y) desired_angle = 4;
	else if (one.x == two.x && one.y < two.y) desired_angle = 8;
	else if (one.x > two.x && one.y == two.y) desired_angle = 12;
	else
	{
		float radians = atan((float)x/(float)y);		
		if (one.x < two.x && one.y > two.y){} //  add 0
		else if (one.x < two.x && one.y < two.y) radians += (float)1.57079633; // add 90
		else if (one.x > two.x && one.y < two.y) radians += (float)3.14159265; // add 180
		else if (one.x > two.x && one.y > two.y) radians = (float)6.28318531 - radians;// 4.71238898; // add 270
		
		desired_angle = (int)((float)radians / 0.392697);
	}	
}
void Ship::get_heading()
{
	set_angle(circle, target);
	float rad = (float)0.392697 * (angle-4);
	if (accelerating < speed && moving == 1) accelerating ++;
	lastX = (int)(cos(rad) * accelerating);
	lastY = (int)(sin(rad) * accelerating); 
}
bool Ship::isOverPlanet(Planet planets[])
{
	for (int i = 0; i < MAX_PLANETS; i++)
	{
		if (get_distance((Circle&)(Circle const&)planets[i].get_circle(), (Circle&)(Circle const&)circle) <= planets[i].get_circle().r) return true;
	}
	return false;
}
bool Ship::enemyVisible(Ship players[])
{
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		if (players[i].get_team() != get_team())
		{
			if (players[i].revealed())
			{
				return true;
			}
		}
	}
	return false;
}
int Ship::enemyInRange(Ship players[])
{
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		if (players[i].get_team() != get_team() && get_distance(players[i].get_circle(), circle) < 750 && players[i].do_render())
		{
			return i;
		}
	}
	return -1;
}
bool Ship::revealed()
{
	if (reveal > 0) return true;
	else return false;
}
void Ship::reveal_ship()
{
	reveal = 200;
}
int Ship::get_nearest(Planet planets[])
{
	
	int nearest = -1;
	int count = 0;
	// Set nearest planet not owned by team
	while (nearest == -1)
	{
		if (planets[count].get_owner() != get_team()) 
		{
			nearest = count;
		}
		else count ++;
	}
	
	// Find planet closer than nearest until all planets are searched
	// For the cases where the owner is not team and i is not = nearest
	for (int i = 0; i < MAX_PLANETS; i++)
	{
		if(planets[i].get_owner() != get_team() && nearest != i)
		{
			if( get_distance((Circle&)(Circle const&)planets[i].get_circle(), (Circle&)(Circle const&)circle) < get_distance((Circle&)(Circle const&)planets[nearest].get_circle(),(Circle&)(Circle const&)circle))
			{
				nearest = i;
			}
		}
	}
	return nearest;
}
int Ship::get_nearest(Ship players[])
{
	
	int nearest = -1;
	int count = 0;
	// Set nearest player not on same team
	while (nearest == -1)
	{
		if (players[count].get_team() != get_team()) 
		{
			nearest = count;
		}
		else count ++;
	}
	
	// Find player closer than nearest until all planets are searched
	// For the cases where the team is not same and i is not = nearest
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		if(players[i].get_team() != get_team() && nearest != i)
		{
			if( get_distance((Circle&)(Circle const&)players[i].get_circle(), (Circle&)(Circle const&)circle) < get_distance((Circle&)(Circle const&)players[nearest].get_circle(),(Circle&)(Circle const&)circle))
			{
				nearest = i;
			}
		}
	}
	return nearest;
}

int Ship::get_team()
{
	if (type < 3) return 1;
	else return 2;
}
int Ship::get_distance(Circle one, Circle two)
{
	int x = 0;
	int y = 0;
	Circle a = one;
	Circle b = two;
	
	// if a < b, subtract it from b, else vice versa
	if (a.x < b.x) x = b.x - a.x;
	else x = a.x - b.x;
	
	if (a.y < b.y) y = b.y - a.y;
	else y = a.y - b.y;
	
	return x + y;
}


/******************************************** TIMER CLASS *******************************************/
//The timer class
class Timer
{
    private:
    //The clock time when the timer started
    int startTicks;
    
    //The ticks stored when the timer was paused
    int pausedTicks;
    
    //The timer status
    bool paused;
    bool started;
    
    public:
    //Initializes variables
    Timer();
    
    //The various clock actions
    void start();
    void stop();
    void pause();
    void unpause();
    
    //Get the number of ticks since the timer started
    //Or gets the number of ticks when the timer was paused
    int get_ticks();
    
    //Checks the status of the timer
    bool is_started();
    bool is_paused();    
};

Timer::Timer()
{
    //Initialize the variables
    startTicks = 0;
    pausedTicks = 0;
    paused = false;
    started = false;    
}

void Timer::start()
{
    //Start the timer
    started = true;
    
    //Unpause the timer
    paused = false;
    
    //Get the current clock time
    startTicks = SDL_GetTicks();    
}

void Timer::stop()
{
    //Stop the timer
    started = false;
    
    //Unpause the timer
    paused = false;    
}

void Timer::pause()
{
    //If the timer is running and isn't already paused
    if( ( started == true ) && ( paused == false ) )
    {
        //Pause the timer
        paused = true;
    
        //Calculate the paused ticks
        pausedTicks = SDL_GetTicks() - startTicks;
    }
}

void Timer::unpause()
{
    //If the timer is paused
    if( paused == true )
    {
        //Unpause the timer
        paused = false;
    
        //Reset the starting ticks
        startTicks = SDL_GetTicks() - pausedTicks;
        
        //Reset the paused ticks
        pausedTicks = 0;
    }
}

int Timer::get_ticks()
{
    //If the timer is running
    if( started == true )
    {
        //If the timer is paused
        if( paused == true )
        {
            //Return the number of ticks when the the timer was paused
            return pausedTicks;
        }
        else
        {
            //Return the current time minus the start time
            return SDL_GetTicks() - startTicks;
        }    
    }
    
    //If the timer isn't running return 0
    return 0;    
}

bool Timer::is_started()
{
    return started;    
}

bool Timer::is_paused()
{
    return paused;    
}

/*************************************************** LOAD IMAGE METHOD ***********************************/
SDL_Surface *load_image( std::string filename ) 
{
    //Temporary storage for the image that's loaded
    SDL_Surface* loadedImage = NULL;
    
    //The optimized image that will be used
    SDL_Surface* optimizedImage = NULL;
    
    //Load the image
    loadedImage = IMG_Load( filename.c_str() );
    
    //If nothing went wrong in loading the image
    if( loadedImage != NULL )
    {
        //Create an optimized image
        optimizedImage = SDL_DisplayFormat( loadedImage );
        
        //Free the old image
        SDL_FreeSurface( loadedImage );
        
        //If the image was optimized just fine
        if( optimizedImage != NULL )
        {
            //Set all pixels of color 0x00FFFF to be transparent
	    Uint32 colorkey = SDL_MapRGB( optimizedImage->format, 255, 0, 255 );
            SDL_SetColorKey( optimizedImage, SDL_RLEACCEL | SDL_SRCCOLORKEY, colorkey );
        }
    }
    
    //Return the optimized image
    return optimizedImage;
}


bool init()
{
    //Initialize all SDL sub systems
    if( SDL_Init( SDL_INIT_EVERYTHING ) == -1 )
    {
        return false;    
    }
    
    //Set up the screen
	if (FULL_SCREEN_MODE == 1) screen = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_FULLSCREEN );
    else screen = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE );
    //If there was in error in setting up the screen
    if( screen == NULL )
    {
        return false;    
    }
    
    //Set the window caption
    SDL_WM_SetCaption( "Dan Nicholson's Colonies", NULL );
    
    //If everything initialized fine
    return true;
}

bool load_files()
{
    //Load the ship image
	// Light 1 Textures
	ships[0][0][1] = load_image( "light1on/1.gif");
	ships[0][1][1] = load_image( "light1on/2.gif");
	ships[0][2][1] = load_image( "light1on/3.gif");
	ships[0][3][1] = load_image( "light1on/4.gif");
	ships[0][4][1] = load_image( "light1on/5.gif");
	ships[0][5][1] = load_image( "light1on/6.gif");
	ships[0][6][1] = load_image( "light1on/7.gif");
	ships[0][7][1] = load_image( "light1on/8.gif");
	ships[0][8][1] = load_image( "light1on/9.gif");
	ships[0][9][1] = load_image( "light1on/10.gif");
	ships[0][10][1] = load_image( "light1on/11.gif");
	ships[0][11][1] = load_image( "light1on/12.gif");
	ships[0][12][1] = load_image( "light1on/13.gif");
	ships[0][13][1] = load_image( "light1on/14.gif");
	ships[0][14][1] = load_image( "light1on/15.gif");
	ships[0][15][1] = load_image( "light1on/16.gif");
	
    ships[0][0][0] = load_image( "light1off/1.gif");
	ships[0][1][0] = load_image( "light1off/2.gif");
	ships[0][2][0] = load_image( "light1off/3.gif");
	ships[0][3][0] = load_image( "light1off/4.gif");
	ships[0][4][0] = load_image( "light1off/5.gif");
	ships[0][5][0] = load_image( "light1off/6.gif");
	ships[0][6][0] = load_image( "light1off/7.gif");
	ships[0][7][0] = load_image( "light1off/8.gif");
	ships[0][8][0] = load_image( "light1off/9.gif");
	ships[0][9][0] = load_image( "light1off/10.gif");
	ships[0][10][0] = load_image( "light1off/11.gif");
	ships[0][11][0] = load_image( "light1off/12.gif");
	ships[0][12][0] = load_image( "light1off/13.gif");
	ships[0][13][0] = load_image( "light1off/14.gif");
	ships[0][14][0] = load_image( "light1off/15.gif");
	ships[0][15][0] = load_image( "light1off/16.gif");
	
	// Heavy 1 textures
	ships[1][0][0] = load_image( "heavy1off/1.gif");
	ships[1][1][0] = load_image( "heavy1off/2.gif");
	ships[1][2][0] = load_image( "heavy1off/3.gif");
	ships[1][3][0] = load_image( "heavy1off/4.gif");
	ships[1][4][0] = load_image( "heavy1off/5.gif");
	ships[1][5][0] = load_image( "heavy1off/6.gif");
	ships[1][6][0] = load_image( "heavy1off/7.gif");
	ships[1][7][0] = load_image( "heavy1off/8.gif");
	ships[1][8][0] = load_image( "heavy1off/9.gif");
	ships[1][9][0] = load_image( "heavy1off/10.gif");
	ships[1][10][0] = load_image( "heavy1off/11.gif");
	ships[1][11][0] = load_image( "heavy1off/12.gif");
	ships[1][12][0] = load_image( "heavy1off/13.gif");
	ships[1][13][0] = load_image( "heavy1off/14.gif");
	ships[1][14][0] = load_image( "heavy1off/15.gif");
	ships[1][15][0] = load_image( "heavy1off/16.gif");
	
	ships[1][0][1] = load_image( "heavy1on/1.gif");
	ships[1][1][1] = load_image( "heavy1on/2.gif");
	ships[1][2][1] = load_image( "heavy1on/3.gif");
	ships[1][3][1] = load_image( "heavy1on/4.gif");
	ships[1][4][1] = load_image( "heavy1on/5.gif");
	ships[1][5][1] = load_image( "heavy1on/6.gif");
	ships[1][6][1] = load_image( "heavy1on/7.gif");
	ships[1][7][1] = load_image( "heavy1on/8.gif");
	ships[1][8][1] = load_image( "heavy1on/9.gif");
	ships[1][9][1] = load_image( "heavy1on/10.gif");
	ships[1][10][1] = load_image( "heavy1on/11.gif");
	ships[1][11][1] = load_image( "heavy1on/12.gif");
	ships[1][12][1] = load_image( "heavy1on/13.gif");
	ships[1][13][1] = load_image( "heavy1on/14.gif");
	ships[1][14][1] = load_image( "heavy1on/15.gif");
	ships[1][15][1] = load_image( "heavy1on/16.gif");

	// Mothership 1 textures
	ships[2][0][0] = load_image( "mother1off/1.gif");
	ships[2][1][0] = load_image( "mother1off/2.gif");
	ships[2][2][0] = load_image( "mother1off/3.gif");
	ships[2][3][0] = load_image( "mother1off/4.gif");
	ships[2][4][0] = load_image( "mother1off/5.gif");
	ships[2][5][0] = load_image( "mother1off/6.gif");
	ships[2][6][0] = load_image( "mother1off/7.gif");
	ships[2][7][0] = load_image( "mother1off/8.gif");
	ships[2][8][0] = load_image( "mother1off/9.gif");
	ships[2][9][0] = load_image( "mother1off/10.gif");
	ships[2][10][0] = load_image( "mother1off/11.gif");
	ships[2][11][0] = load_image( "mother1off/12.gif");
	ships[2][12][0] = load_image( "mother1off/13.gif");
	ships[2][13][0] = load_image( "mother1off/14.gif");
	ships[2][14][0] = load_image( "mother1off/15.gif");
	ships[2][15][0] = load_image( "mother1off/16.gif");
	
	ships[2][0][1] = load_image( "mother1on/1.gif");
	ships[2][1][1] = load_image( "mother1on/2.gif");
	ships[2][2][1] = load_image( "mother1on/3.gif");
	ships[2][3][1] = load_image( "mother1on/4.gif");
	ships[2][4][1] = load_image( "mother1on/5.gif");
	ships[2][5][1] = load_image( "mother1on/6.gif");
	ships[2][6][1] = load_image( "mother1on/7.gif");
	ships[2][7][1] = load_image( "mother1on/8.gif");
	ships[2][8][1] = load_image( "mother1on/9.gif");
	ships[2][9][1] = load_image( "mother1on/10.gif");
	ships[2][10][1] = load_image( "mother1on/11.gif");
	ships[2][11][1] = load_image( "mother1on/12.gif");
	ships[2][12][1] = load_image( "mother1on/13.gif");
	ships[2][13][1] = load_image( "mother1on/14.gif");
	ships[2][14][1] = load_image( "mother1on/15.gif");
	ships[2][15][1] = load_image( "mother1on/16.gif");

	// Light 2 Textures
	ships[3][0][1] = load_image( "light2on/1.gif");
	ships[3][1][1] = load_image( "light2on/2.gif");
	ships[3][2][1] = load_image( "light2on/3.gif");
	ships[3][3][1] = load_image( "light2on/4.gif");
	ships[3][4][1] = load_image( "light2on/5.gif");
	ships[3][5][1] = load_image( "light2on/6.gif");
	ships[3][6][1] = load_image( "light2on/7.gif");
	ships[3][7][1] = load_image( "light2on/8.gif");
	ships[3][8][1] = load_image( "light2on/9.gif");
	ships[3][9][1] = load_image( "light2on/10.gif");
	ships[3][10][1] = load_image( "light2on/11.gif");
	ships[3][11][1] = load_image( "light2on/12.gif");
	ships[3][12][1] = load_image( "light2on/13.gif");
	ships[3][13][1] = load_image( "light2on/14.gif");
	ships[3][14][1] = load_image( "light2on/15.gif");
	ships[3][15][1] = load_image( "light2on/16.gif");
	

    ships[3][0][0] = load_image( "light2off/1.gif");
	ships[3][1][0] = load_image( "light2off/2.gif");
	ships[3][2][0] = load_image( "light2off/3.gif");
	ships[3][3][0] = load_image( "light2off/4.gif");
	ships[3][4][0] = load_image( "light2off/5.gif");
	ships[3][5][0] = load_image( "light2off/6.gif");
	ships[3][6][0] = load_image( "light2off/7.gif");
	ships[3][7][0] = load_image( "light2off/8.gif");
	ships[3][8][0] = load_image( "light2off/9.gif");
	ships[3][9][0] = load_image( "light2off/10.gif");
	ships[3][10][0] = load_image( "light2off/11.gif");
	ships[3][11][0] = load_image( "light2off/12.gif");
	ships[3][12][0] = load_image( "light2off/13.gif");
	ships[3][13][0] = load_image( "light2off/14.gif");
	ships[3][14][0] = load_image( "light2off/15.gif");
	ships[3][15][0] = load_image( "light2off/16.gif");
	
	// Heavy 2 Textures
	ships[4][0][0] = load_image( "heavy2off/1.gif");
	ships[4][1][0] = load_image( "heavy2off/2.gif");
	ships[4][2][0] = load_image( "heavy2off/3.gif");
	ships[4][3][0] = load_image( "heavy2off/4.gif");
	ships[4][4][0] = load_image( "heavy2off/5.gif");
	ships[4][5][0] = load_image( "heavy2off/6.gif");
	ships[4][6][0] = load_image( "heavy2off/7.gif");
	ships[4][7][0] = load_image( "heavy2off/8.gif");
	ships[4][8][0] = load_image( "heavy2off/9.gif");
	ships[4][9][0] = load_image( "heavy2off/10.gif");
	ships[4][10][0] = load_image( "heavy2off/11.gif");
	ships[4][11][0] = load_image( "heavy2off/12.gif");
	ships[4][12][0] = load_image( "heavy2off/13.gif");
	ships[4][13][0] = load_image( "heavy2off/14.gif");
	ships[4][14][0] = load_image( "heavy2off/15.gif");
	ships[4][15][0] = load_image( "heavy2off/16.gif");
	
	ships[4][0][1] = load_image( "heavy2on/1.gif");
	ships[4][1][1] = load_image( "heavy2on/2.gif");
	ships[4][2][1] = load_image( "heavy2on/3.gif");
	ships[4][3][1] = load_image( "heavy2on/4.gif");
	ships[4][4][1] = load_image( "heavy2on/5.gif");
	ships[4][5][1] = load_image( "heavy2on/6.gif");
	ships[4][6][1] = load_image( "heavy2on/7.gif");
	ships[4][7][1] = load_image( "heavy2on/8.gif");
	ships[4][8][1] = load_image( "heavy2on/9.gif");
	ships[4][9][1] = load_image( "heavy2on/10.gif");
	ships[4][10][1] = load_image( "heavy2on/11.gif");
	ships[4][11][1] = load_image( "heavy2on/12.gif");
	ships[4][12][1] = load_image( "heavy2on/13.gif");
	ships[4][13][1] = load_image( "heavy2on/14.gif");
	ships[4][14][1] = load_image( "heavy2on/15.gif");
	ships[4][15][1] = load_image( "heavy2on/16.gif");
	
	// Mothership 2 Textures
	
	ships[5][0][0] = load_image( "mother2off/1.gif");
	ships[5][1][0] = load_image( "mother2off/2.gif");
	ships[5][2][0] = load_image( "mother2off/3.gif");
	ships[5][3][0] = load_image( "mother2off/4.gif");
	ships[5][4][0] = load_image( "mother2off/5.gif");
	ships[5][5][0] = load_image( "mother2off/6.gif");
	ships[5][6][0] = load_image( "mother2off/7.gif");
	ships[5][7][0] = load_image( "mother2off/8.gif");
	ships[5][8][0] = load_image( "mother2off/9.gif");
	ships[5][9][0] = load_image( "mother2off/10.gif");
	ships[5][10][0] = load_image( "mother2off/11.gif");
	ships[5][11][0] = load_image( "mother2off/12.gif");
	ships[5][12][0] = load_image( "mother2off/13.gif");
	ships[5][13][0] = load_image( "mother2off/14.gif");
	ships[5][14][0] = load_image( "mother2off/15.gif");
	ships[5][15][0] = load_image( "mother2off/16.gif");
	
	ships[5][0][1] = load_image( "mother2on/1.gif");
	ships[5][1][1] = load_image( "mother2on/2.gif");
	ships[5][2][1] = load_image( "mother2on/3.gif");
	ships[5][3][1] = load_image( "mother2on/4.gif");
	ships[5][4][1] = load_image( "mother2on/5.gif");
	ships[5][5][1] = load_image( "mother2on/6.gif");
	ships[5][6][1] = load_image( "mother2on/7.gif");
	ships[5][7][1] = load_image( "mother2on/8.gif");
	ships[5][8][1] = load_image( "mother2on/9.gif");
	ships[5][9][1] = load_image( "mother2on/10.gif");
	ships[5][10][1] = load_image( "mother2on/11.gif");
	ships[5][11][1] = load_image( "mother2on/12.gif");
	ships[5][12][1] = load_image( "mother2on/13.gif");
	ships[5][13][1] = load_image( "mother2on/14.gif");
	ships[5][14][1] = load_image( "mother2on/15.gif");
	ships[5][15][1] = load_image( "mother2on/16.gif");

	// Button textures
	button[0][0] = load_image("menu/new_game_button0.gif");
	button[0][1] = load_image("menu/new_game_button1.gif");
	button[1][0] = load_image("menu/how_to_play_button0.gif");
	button[1][1] = load_image("menu/how_to_play_button1.gif");
	button[2][0] = load_image("menu/quit_game_button0.gif");
	button[3][0] = load_image("planets/blue.gif");
	button[4][0] = load_image("planets/red.gif");
	button[5][0] = load_image("menu/light1.gif");
	button[6][0] = load_image("menu/light2.gif");
	button[7][0] = load_image("menu/heavy1.gif");
	button[8][0] = load_image("menu/heavy2.gif");

	// Planet Textures
	planetTex[0] = load_image("planets/1.gif");
	planetTex[1] = load_image("planets/2.gif");
	planetTex[2] = load_image("planets/3.gif");
	planetTex[3] = load_image("planets/4.gif");

	// Health / Shield bar textures
	healthbar[0] = load_image("healthbars/light1health.gif");
	healthbar[1] = load_image("healthbars/heavy1health.gif");
	healthbar[2] = load_image("healthbars/mother1health.gif");
	healthbar[3] = load_image("healthbars/light2health.gif");
	healthbar[4] = load_image("healthbars/heavy2health.gif");
	healthbar[5] = load_image("healthbars/mother2health.gif");
	shieldbar = load_image("healthbars/shieldbar.gif");
	
	// Explosion Textures
	explosion[0][0] = load_image("explosions/0/1.gif");
	explosion[0][1] = load_image("explosions/0/2.gif");
	explosion[0][2] = load_image("explosions/0/3.gif");
	explosion[0][3] = load_image("explosions/0/4.gif");
	explosion[0][4] = load_image("explosions/0/5.gif");
	explosion[0][5] = load_image("explosions/0/6.gif");
	explosion[0][6] = load_image("explosions/0/7.gif");
	explosion[0][7] = load_image("explosions/0/8.gif");
	explosion[0][8] = load_image("explosions/0/9.gif");
	explosion[0][9] = load_image("explosions/0/10.gif");
	explosion[0][10] = load_image("explosions/0/11.gif");
	
	explosion[1][0] = load_image("explosions/1/1.gif");
	explosion[1][1] = load_image("explosions/1/2.gif");
	explosion[1][2] = load_image("explosions/1/3.gif");
	explosion[1][3] = load_image("explosions/1/4.gif");
	explosion[1][4] = load_image("explosions/1/5.gif");
	explosion[1][5] = load_image("explosions/1/6.gif");
	explosion[1][6] = load_image("explosions/1/7.gif");
	explosion[1][7] = load_image("explosions/1/8.gif");
	explosion[1][8] = load_image("explosions/1/9.gif");
	explosion[1][9] = load_image("explosions/1/10.gif");
	explosion[1][10] = load_image("explosions/1/11.gif");
	
	explosion[2][0] = load_image("explosions/2/1.gif");
	explosion[2][1] = load_image("explosions/2/2.gif");
	explosion[2][2] = load_image("explosions/2/3.gif");
	explosion[2][3] = load_image("explosions/2/4.gif");
	explosion[2][4] = load_image("explosions/2/5.gif");
	explosion[2][5] = load_image("explosions/2/6.gif");
	explosion[2][6] = load_image("explosions/2/7.gif");
	explosion[2][7] = load_image("explosions/2/8.gif");
	explosion[2][8] = load_image("explosions/2/9.gif");
	explosion[2][9] = load_image("explosions/2/10.gif");
	explosion[2][10] = load_image("explosions/2/11.gif");

	// Radar Textures
	radar_screen[0] = load_image("radar/blue_screen.gif");
	radar_screen[1] = load_image("radar/red_screen.gif");
	
	radar_dot[0] = load_image("radar/blue.gif");
	radar_dot[1] = load_image("radar/red.gif");
	radar_dot[2] = load_image("radar/yellow.gif");
	radar_dot[3] = load_image("radar/lrg_blue.gif");
	radar_dot[4] = load_image("radar/lrg_red.gif");
	radar_dot[5] = load_image("radar/planet_g.gif");
	radar_dot[6] = load_image("radar/planet_b.gif");
	radar_dot[7] = load_image("radar/planet_r.gif");
	
	displaybar[0] = load_image("healthbars/bluebar.gif");
	displaybar[1] = load_image("healthbars/redbar.gif");
	SDL_SetAlpha(radar_screen[0], SDL_SRCALPHA, 200);
	SDL_SetAlpha(radar_screen[1], SDL_SRCALPHA, 200);

	SDL_SetAlpha(displaybar[0], SDL_SRCALPHA, 200);
	SDL_SetAlpha(displaybar[1], SDL_SRCALPHA, 200);
	shooting_star = load_image("shooting_star.gif");

	// Planet owner logos
	planet_owner[0] = load_image("planets/blue.gif");
	planet_owner[1] = load_image("planets/red.gif");
	SDL_SetAlpha(planet_owner[0], SDL_SRCALPHA, 100);
	SDL_SetAlpha(planet_owner[1], SDL_SRCALPHA, 100);

	// pod textures
	pod[0] = load_image("pod/1.gif");
	pod[1] = load_image("pod/2.gif");
	pod[2] = load_image("pod/3.gif");
	pod[3] = load_image("pod/4.gif");
	pod[4] = load_image("pod/5.gif");
	pod[5] = load_image("pod/6.gif");
	pod[6] = load_image("pod/7.gif");
	pod[7] = load_image("pod/8.gif");
	pod[8] = load_image("pod/9.gif");
	pod[9] = load_image("pod/10.gif");
	pod[10] = load_image("pod/11.gif");
	pod[11] = load_image("pod/12.gif");
	pod[12] = load_image("pod/13.gif");
	pod[13] = load_image("pod/14.gif");
	pod[14] = load_image("pod/15.gif");
	pod[15] = load_image("pod/16.gif");

	// Menu
	menu[0] = load_image("menu/home.gif");
	SDL_SetAlpha(menu[0], SDL_SRCALPHA, 200);
	menu[1] = load_image("menu/how_to_play.gif");
	SDL_SetAlpha(menu[1], SDL_SRCALPHA, 200);
	menu[2] = load_image("menu/you_loose.gif");
	menu[3] = load_image("menu/you_win.gif");
    
    //Load the tile image
    tileSheet = load_image( "background.gif" );
	// Laser
    laser = load_image("laser/1.gif");
	
	// Shield Textures
	shieldTexture[0] = load_image("shield_light.gif");
	SDL_SetAlpha(shieldTexture[0], SDL_SRCALPHA, 100);
	shieldTexture[1] = load_image("shield_heavy.gif");
	SDL_SetAlpha(shieldTexture[1], SDL_SRCALPHA, 100);
	shieldTexture[2] = load_image("shield_mother.gif");
	SDL_SetAlpha(shieldTexture[2], SDL_SRCALPHA, 100);
	shieldTexture[3] = load_image("shield_light.gif");
	SDL_SetAlpha(shieldTexture[3], SDL_SRCALPHA, 100);
	shieldTexture[4] = load_image("shield_heavy.gif");
	SDL_SetAlpha(shieldTexture[4], SDL_SRCALPHA, 100);
	shieldTexture[5] = load_image("shield_mother.gif");
	SDL_SetAlpha(shieldTexture[5], SDL_SRCALPHA, 100);

    //If there was a problem in loading the textures (assuming all textures are present if one is)
    if( tileSheet == NULL )
    {
        return false;    
    }
    
    //If everything loaded fine
    return true;
}

/*************************************** BUTTON CLASS *********************************************/
//The button
class Button
{
    private:
    //The attributes of the button
    SDL_Rect box;
    int state, type;
	
    public:
    //Initialize the variables
    Button( int x, int y, int w, int h, int t );
    
    //Handles events and set the button's sprite region
    void handle_events();
    
    //Shows the button on the screen
    void show();
};



Button::Button( int x, int y, int w, int h, int t )
{
    //Set the button's attributes
    box.x = x;
    box.y = y;
    box.w = w;
    box.h = h;
    
    //Set the default sprite
    state = 0;
	type = t;
}
    
void Button::handle_events()
{
    //The mouse offsets
    int x = 0, y = 0;
    
    //If a mouse button was pressed
    if( event.type == SDL_MOUSEBUTTONDOWN )
    {
        //If the left mouse button was pressed
        if( event.button.button == SDL_BUTTON_LEFT )
        {
            //Get the mouse offsets
            x = event.button.x;
            y = event.button.y;
        
            //If the mouse is over the button
            if( ( x > box.x ) && ( x < box.x + box.w ) && ( y > box.y ) && ( y < box.y + box.h ) )
            {
				if (type == 0) 
				{
					menu_position = 0;
					active_button = type;
				}
				else if (type == 1)
				{
					menu_position = 3;
					active_button = type;
				}
				else if (type == 2) exit(0);
				else if (type == 3) 
				{
					menu_position = 1;
				}
				else if (type == 4) menu_position = 2;
				
				else if (type == 5)
				{
					player_type = 0;
					menu_position = 3;
					paused = false;
				}
				else if (type == 6)
				{
					player_type = 3;
					menu_position = 3;
					paused = false;
				}
				else if (type == 7)
				{
					player_type = 1;
					menu_position = 3;
					paused = false;
				}
				else if (type == 8)
				{
					player_type = 4;
					menu_position = 3;
					paused = false;
				}
				
            }
        }
		
    }
	if (active_button != type) state = 0;
	else state = 1;
}
    
void Button::show()
{
    //Show the button
    apply_surface( box.x, box.y, button[type][state], screen);
}

/******************************************* CLEAN UP METHOD ******************************************/
void clean_up( Tile *tiles[] )
{
    //Free the surfaces
	for (int i = 0; i <= 5; i++)
	{
		int count = 0;
		while (count < 16)
		{	
			SDL_FreeSurface( ships[i][count][0]);
			SDL_FreeSurface( ships[i][count][1]);
			count++;
		}
		
	}
	for (int i = 0; i < 9; i++)
	{
		SDL_FreeSurface(button[i][0]);
		SDL_FreeSurface(button[i][1]);
	}

	SDL_FreeSurface( tileSheet );
	SDL_FreeSurface( planetTex[0]);
	SDL_FreeSurface( planetTex[1]);
	SDL_FreeSurface( planetTex[2]);
	SDL_FreeSurface( planetTex[3]);

	SDL_FreeSurface(healthbar[0]);
	SDL_FreeSurface(healthbar[1]);
	SDL_FreeSurface(healthbar[2]);
	SDL_FreeSurface(healthbar[3]);
	SDL_FreeSurface(healthbar[4]);
	SDL_FreeSurface(healthbar[5]);
	
	SDL_FreeSurface(shieldbar);
	
	
	for (int i = 0; i < 3; i++)
	{
		for (int count = 0; count < 11; count++)
		{	
			SDL_FreeSurface( explosion[i][count]);
			SDL_FreeSurface( explosion[i][count]);
		}
		
	}
	SDL_FreeSurface(radar_screen[0]);
	SDL_FreeSurface(radar_screen[1]);
	
	for (int i = 0; i < 8; i++)
	{
		SDL_FreeSurface(radar_dot[i]);
	}
	SDL_FreeSurface(displaybar[0]);
	SDL_FreeSurface(displaybar[1]);
	SDL_FreeSurface(shooting_star);
	SDL_FreeSurface(planet_owner[0]);
	SDL_FreeSurface(planet_owner[1]);
	
	for (int i = 0; i < 16; i++)
	{
		SDL_FreeSurface(pod[i]);
	}
	SDL_FreeSurface(menu[0]);
	SDL_FreeSurface(menu[1]);
	
	SDL_FreeSurface( laser);
	SDL_FreeSurface( tileSheet);
	
	for (int i = 0; i < 6; i++)
	{
		SDL_FreeSurface(shieldTexture[i]);
	}
	
	//Free the tiles
    for( int t = 0; t < TOTAL_TILES; t++ )
    {
        delete tiles[ t ];    
    }
    
    //Free the pointer array
    delete []tiles;
    
    //Quit SDL
    SDL_Quit();
}
bool set_tiles( Tile *tiles[] )
{
    //The tile offset
    int x = 0, y = 0;
	
    //Initialize the tiles
    for( int t = 0; t < TOTAL_TILES; t++ )
    {

		tiles[t] = new Tile(x,y,tileSheet);
		
		//Move to next tile spot
        x += TILE_WIDTH;
        
        //If we've gone too far
        if( x >= LEVEL_WIDTH )
        {
            //Move back
            x = 0;
            
            //Move to the next row
            y += TILE_HEIGHT;    
        }
    }
    
    //If the map was loaded fine
    return true;
}

/******************************************** SHOOTING STAR CLASS *********************************/
class Shooting_Star
{
	private:
	//Holds coordinates and dimensions of the collision box
	SDL_Rect box;
	// X and Y Velocity
	int xVel, yVel;
	
	public:
	Shooting_Star();
	// Displays Shooting_Star on screen
	void show(SDL_Surface *screen);
	// Moves Shooting_Star
	void move();
	SDL_Rect get_coords();
};

Shooting_Star::Shooting_Star()
{
	box.x = rand()%LEVEL_WIDTH;
	box.y = rand()%LEVEL_HEIGHT;
	box.w = 2;
	box.h = 2;
	int tempX = rand()%40;
	int tempY = 0;
	if (tempX < 25)
	{
		while(tempY < 25)
		{
			tempY = rand()%40;
		}
	}
	else tempY = rand()%40;
	xVel = tempX;
	yVel = tempY;
}

SDL_Rect Shooting_Star::get_coords()
{
	return box;
}
void Shooting_Star::show( SDL_Surface *screen )
{    
    //Show the Shooting Star
	SDL_SetAlpha(shooting_star, SDL_SRCALPHA, 255);
	apply_surface( box.x - camera.x, box.y - camera.y, shooting_star, screen);
}

void Shooting_Star::move()
{
	//If the Shooting_Star hits the left or right edges of the screen, reverse velocity (bounce off)
    if ((box.x + xVel) < 0 || (box.x + xVel + box.w) > LEVEL_WIDTH)
	{
		xVel = -xVel;
		box.x += xVel;
	}
	else
	{
		box.x +=xVel;
	}

	//If the Shooting_Star hits the top or bottom of the screen, bounce off
    if ((box.y + yVel) < 0 || (box.y + yVel + box.h) > LEVEL_HEIGHT)
	{
		yVel = -yVel;
		box.y += yVel;
	}
	else
	{
		box.y +=yVel;
	}
	
}

int get_distance(Circle a, Circle b)
{
	//return sqrt( double pow( b.x - a.x, 2 ) + double pow( b.y - a.y, 2 ) );
	double g = pow( (double)b.x - (double)a.x, 2 );
	double h =  pow( (double)b.y - (double)a.y, 2 );
    return (int) sqrt(g + h);
}
/********************************************* MAIN METHOD ******************************************/
int main( int argc, char* args[] )
{
	// Set all buttons to react to screen size change
	Button newGame( (SCREEN_WIDTH / 2) - (150/2) - 200, (SCREEN_HEIGHT / 2) - (50/2) - 120, 150, 50, 0);
	Button howToPlay( (SCREEN_WIDTH / 2) - (150/2), (SCREEN_HEIGHT / 2) - (50/2) - 120, 150, 50, 1);
	Button exitGame( (SCREEN_WIDTH / 2) - (150/2) + 200, (SCREEN_HEIGHT / 2) - (50/2) - 120, 150, 50, 2);
	Button team1 ((SCREEN_WIDTH / 2) - (125/2) - 100, (SCREEN_HEIGHT / 2) - (125/2), 125, 125, 3);
	Button team2 ((SCREEN_WIDTH / 2) - (125/2) + 100, (SCREEN_HEIGHT / 2) - (125/2), 125, 125, 4);
	Button light1 ((SCREEN_WIDTH / 2) - (200/2) - 150, (SCREEN_HEIGHT / 2) - (120/2)+ 50, 200, 120, 5);
	Button light2 ((SCREEN_WIDTH / 2) - (200/2) - 150, (SCREEN_HEIGHT / 2) - (87/2)+ 50, 200, 87, 6);
	Button heavy1 ((SCREEN_WIDTH / 2) - (203/2) + 150, (SCREEN_HEIGHT / 2) - (180/2)+ 50, 203, 180, 7);
	Button heavy2 ((SCREEN_WIDTH / 2) - (259/2) + 150, (SCREEN_HEIGHT / 2) - (180/2)+ 50, 259, 180, 8);
	
	bool quit = false;
	// Create stars
	Shooting_Star star[SHOOTING_STARS];
	
	// Create planets
	Planet planets[MAX_PLANETS];
	int count = 0;
	srand((unsigned)time(0));
	while (count < MAX_PLANETS)
	{
		int type = rand()%4;
		Circle temp;
		temp.x = rand()%14200;
		temp.y = rand()%14200;
		for (int i = 0; i <= count; i++)
		{
			if (get_distance(temp, planets[count].get_circle()) < 1200)
			{
				temp.x = rand()%14200;
				temp.y = rand()%14200;
				i = 0;
			}
		}
		planets[count].create(type, temp.x, temp.y);
		count++;
	}
	// Create players
	Ship players[MAX_PLAYERS];
	int this_player = 0;
	// Init game for 'quick start'
	this_mothership = 1;
	enemy_mothership = 2;
	// Set motherships
	players[1].set_type(2);
	players[2].set_type(5);
	// Make motherships distance further apart if necessary
	while (get_distance(players[1].get_circle(), players[2].get_circle()) < 10000)
	{
		players[2].set_type(5);
	}
	// Set reference boxes
	mothership[0] = players[this_mothership].get_coords();
	mothership[1] = players[enemy_mothership].get_coords();
	players[0].set_type(player_type);
	this_mothership = players[this_player].get_team();
	if (this_mothership == 1) 
	{
		blue_mother = 0;
		red_mother = 1;
		enemy_mothership = 2;
	}
	else 
	{
		blue_mother = 1;
		red_mother = 0;
		enemy_mothership = 1;
	}
	// init AI
	players[1].ai_on();
	players[2].ai_on();
	srand((unsigned)time(0));

	// init other players and distribute equally between both teams
	for (int i = 3; i < (MAX_PLAYERS/2); i++)
	{
		int type = rand()%2;
		players[i].set_type(type);
		players[i].ai_on();
	}
	srand((unsigned)time(0));

	for (int i = (MAX_PLAYERS/2); i < MAX_PLAYERS; i++)
	{
		int type = rand()%5;
		while (type < 3)
		{
			type = rand()%5;
		}
		players[i].set_type(type);
		players[i].ai_on();
	}
    //The tiles that will be used
    Tile *tiles[ TOTAL_TILES ];
    
    //The frames rate regulator
    Timer fps;
    
    //Do the initialization
    if( init() == false )
    {
        return 1;
    }
    //Load the files
    if( load_files() == false )
    {
        return 1;
    }
    //Set the tiles
    if( set_tiles( tiles ) == false )
    {
        return 1;    
    }
	
    //Make sure the program waits for a quit
    while( quit == false )
    {
        //Start the frame timer
        fps.start();
        
		//While there's an event to handle
        while( SDL_PollEvent( &event ) )
        {
            //Handle events for the Ship
			if (paused)
			{
				int reset = 0;
				// Handle menu events
				newGame.handle_events();
				howToPlay.handle_events();
				exitGame.handle_events();
				
				if (menu_position == 0)
				{
					team1.handle_events();
					team2.handle_events();
				}
				else if (menu_position == 1)
				{
					light1.handle_events();
					heavy1.handle_events();
					reset = 1;
				}
				else if (menu_position == 2)
				{
					light2.handle_events();
					heavy2.handle_events();
					reset = 1;
				}
				
				// Reset game (as above)
				if (reset == 1)
				{
					count = 0;
					srand((unsigned)time(0));
					while (count < MAX_PLANETS)
					{
						int type = rand()%4;
						Circle temp;
						temp.x = rand()%14200;
						temp.y = rand()%14200;
						for (int i = 0; i <= count; i++)
						{
							if (get_distance(temp, planets[count].get_circle()) < 1200)
							{
								temp.x = rand()%14200;
								temp.y = rand()%14200;
								i = 0;
							}
						}
						planets[count].create(type, temp.x, temp.y);
						count++;
					}
					players[1].set_type(2);
					players[2].set_type(5);
					while (get_distance(players[1].get_circle(), players[2].get_circle()) < 10000)
					{
						players[2].set_type(5);
					}
					
					
					players[0].set_type(player_type);
					this_mothership = players[this_player].get_team();
					if (this_mothership == 1) 
					{
						blue_mother = 0;
						red_mother = 1;
						enemy_mothership = 2;
					}
					else 
					{
						blue_mother = 1;
						red_mother = 0;
						enemy_mothership = 1;
					}
					mothership[0] = players[this_mothership].get_coords();
					mothership[1] = players[enemy_mothership].get_coords();
					players[0].set_type(player_type);
					players[1].ai_on();
					players[2].ai_on();
					srand((unsigned)time(0));

					for (int i = 3; i < (MAX_PLAYERS/2)+1; i++)
					{
						int type = rand()%2;
						players[i].set_type(type);
						players[i].ai_on(); 
					}
					srand((unsigned)time(0));

					for (int i = (MAX_PLAYERS/2)+1; i < MAX_PLAYERS; i++)
					{
						int type = rand()%5;
						while (type < 3)
						{
							type = rand()%5;
						}
						players[i].set_type(type);
						players[i].ai_on();
					}
				}

				
			}
            players[0].handle_input( event );
            
            //If the user has Xed out the window
            if( event.type == SDL_QUIT )
            {
                //Quit the program
                quit = true;
            }
        }

        //Set the camera
        players[0].set_camera();
        
        //Show the tiles
         for( int t = 0; t < TOTAL_TILES; t++ )
         {
             tiles[ t ]->show( screen );
         }

		if( event.type == SDL_KEYDOWN )
			{
				switch( event.key.keysym.sym )
				{
					// HANDLE RESPAWN KEYS
					case SDLK_1:
					if(players[0].do_render() || players[0].get_pod().r != 0 || mothership[players[0].get_team()-1].w == 0){}
					else if (players[0].get_team() == 1) 
					{
						players[0].set_type(0);
					}
					else players[0].set_type(3);
					break;
					case SDLK_2:
					if(players[0].do_render() || players[0].get_pod().r != 0 || mothership[players[0].get_team()-1].w == 0){}
					else if (players[0].get_team() == 1) 
					{
						players[0].set_type(1);
					}
					else players[0].set_type(4);
					break;
					
					// Handle resume key
					case SDLK_ESCAPE:
					active_button = 1;
					menu_position = 3;
					if (!paused) paused = true;
					break;

					case SDLK_r:
					paused = false;
					break;
					default: break;
				}
			}
		// Show menu
		if (paused)
		{
			SDL_ShowCursor(SDL_ENABLE);
			// Resize to fit window
			int xW = (SCREEN_WIDTH / 2) - (800/2);
			int yH = (SCREEN_HEIGHT / 2) - (600/2); 
			int position = 0;
			if (menu_position == 3) position = 1;
			apply_surface(xW,yH, menu[position], screen);
			if (menu_position == 0)
			{
				team1.show();
				team2.show();
			}
			else if (menu_position == 1)
			{
				light1.show();
				heavy1.show();
				

			}
			else if (menu_position == 2)
			{
				light2.show();
				heavy2.show();

			}
			newGame.show();
			howToPlay.show();
			exitGame.show();
			
		}
		// Show game
		else
		{
			SDL_ShowCursor(SDL_DISABLE);
			// Check collision between ships 
			// Check all players for collisions
			for(int i= 0 ; i < MAX_PLAYERS; i++)
			{
				for (int c = 0; c < MAX_PLAYERS; c++)
				{
					// Check that the two players are not the same and are both alive
					if( i!= c &&  players[i].do_render() && players[c].do_render())
					{	
						// Check each player for a colision
						if (check_collision((Circle&)(Circle const&)players[i].get_circle(), (Circle&)(Circle const&)players[c].get_circle()) == true)			
						{
							// If player is on team one and is colliding with mothership, recharge shield
							if (players[i].get_team() == 1 && players[c].get_type() == 2)
							{
								players[i].recharge_shield(1);
							}
							// Above for team 2
							else if (players[i].get_team() == 2 && players[c].get_type() == 5)
							{
								players[i].recharge_shield(1);
							}
							// NOTE: Collision detection between ships was removed to enhance gameplay, as the 'bounce' function made the gameplay frustrating
						}
						// Check that player 'i's projectile has not impacted on player 'c's ship
						if (collision((SDL_Rect&)(SDL_Rect const&)players[i].get_proj().get_coords(), (SDL_Rect&)(SDL_Rect const&)players[c].get_coords()) == true && i != c)
						{
							// Do not allow friendly fire
							// If player 1 is 0,1,2 and player 2 is 0,1,2 then do nothing
							if(players[i].get_team() == 1 && players[c].get_team() == 1){}// && players[c].get_type() < 3){}
							// Else if player 1 is 3,4,5 and player 2 is 3,4,5 then do nothing
							else if(players[i].get_team() == 2 && players[c].get_team() == 2){}// && players[i].get_type() > 2 && players[c].get_type() < 6 && players[c].get_type() > 2){}
							// Otherwise, hurt player
							else
							{
								players[i].destroy_proj();
								players[c].hurt(players[i].get_damage());
							}
						}
						
					}
					
				}
			}
			for (int i = 0; i< MAX_PLANETS; i++)
			{
				for (int c = 0; c < MAX_PLAYERS; c++)
				{	
					// if player collides with planet - begin capture
					if (collision((SDL_Rect&)(SDL_Rect const&)planets[i].get_coords(), (SDL_Rect&)(SDL_Rect const&)players[c].get_coords()) == true && players[c].do_render())
					{
						if (players[c].get_type() < 3)
						{
							planets[i].capture(players[c].get_damage()/2, 1);
						}
						else planets[i].capture(players[c].get_damage()/2, 2);
					}
				}
			
			}

			for (int i = 0; i < MAX_PLAYERS; i++)
			{
				players[i].move(planets, players);
			}
			/* SHOW GRAPHICS ON SCREEN */
			for (int i = 0; i < SHOOTING_STARS; i++)
			{
				star[i].move();
				star[i].show(screen);
			}
			int count = 0;
			while (count < MAX_PLANETS)
			{
				planets[count].show(screen);
				count++;
			}
			players[this_mothership].show(screen);
			players[enemy_mothership].show(screen);

			count = 0;
			while (count < MAX_PLAYERS)
			{
				if (count == this_mothership || count == enemy_mothership) {}
				else players[count].show( screen );
				
				count ++;
			}
			// PLANET OWNER LOGOS
			for (int i = 0; i < MAX_PLANETS; i++)
			{
				if (planets[i].get_owner() > 0)
				{
					int tempX = planets[i].get_coords().x - camera.x + (planets[i].get_coords().w/2)-62;
					int tempY = planets[i].get_coords().y - camera.y + (planets[i].get_coords().h/2)-62;
					apply_surface( tempX, tempY, planet_owner[planets[i].get_owner()-1], screen);
				}
			}
			
			// HEALTH BAR
			players[0].show_health( screen );
			
			// RADAR SCREEN
			if (players[this_player].get_type() < 3)apply_surface( 0, 0, radar_screen[0], screen);
			else apply_surface(0, 0, radar_screen[1], screen);
			for (int i = 0; i < MAX_PLANETS; i++)
			{
				if (planets[i].get_owner() == 1)
				{
					apply_surface( (planets[i].get_coords().x / 100)+7, (planets[i].get_coords().y / 100)+7, radar_dot[6], screen);
				}
				else if (planets[i].get_owner() == 2)
				{
					apply_surface( (planets[i].get_coords().x / 100)+7, (planets[i].get_coords().y / 100)+7, radar_dot[7], screen);
				}
				else apply_surface( (planets[i].get_coords().x / 100)+7, (planets[i].get_coords().y / 100)+7, radar_dot[5], screen);
			}
			for (int i = 0; i < MAX_PLAYERS; i++)
			{
				if (players[i].do_render())
				{
					if (i == this_player) {apply_surface( (players[i].get_coords().x / 100)+7, (players[i].get_coords().y / 100)+7, radar_dot[2], screen);}
					else
					{
						switch (players[i].get_type())
						{
							case 0:
							if (players[this_player].get_team() != players[i].get_team())
							{
								if (players[i].revealed()) apply_surface( (players[i].get_coords().x / 100)+7, (players[i].get_coords().y / 100)+7, radar_dot[0], screen);
							}
							else apply_surface( (players[i].get_coords().x / 100)+7, (players[i].get_coords().y / 100)+7, radar_dot[0], screen);
							break;
							case 1:
							if (players[this_player].get_team() != players[i].get_team())
							{
								if (players[i].revealed()) apply_surface( (players[i].get_coords().x / 100)+7, (players[i].get_coords().y / 100)+7, radar_dot[0], screen);
							}
							else apply_surface( (players[i].get_coords().x / 100)+7, (players[i].get_coords().y / 100)+7, radar_dot[0], screen);
							break;
							case 2:
							if (players[this_player].get_team() != players[i].get_team())
							{
								if (players[i].revealed()) apply_surface( (players[i].get_coords().x / 100)+7, (players[i].get_coords().y / 100)+7, radar_dot[3], screen);
							}
							else apply_surface( (players[i].get_coords().x / 100)+7, (players[i].get_coords().y / 100)+7, radar_dot[3], screen);
							break;
							case 3:
							if (players[this_player].get_team() != players[i].get_team())
							{
								if (players[i].revealed()) apply_surface( (players[i].get_coords().x / 100)+7, (players[i].get_coords().y / 100)+7, radar_dot[1], screen);
							}
							else apply_surface( (players[i].get_coords().x / 100)+7, (players[i].get_coords().y / 100)+7, radar_dot[1], screen);break;
							case 4:
							if (players[this_player].get_team() != players[i].get_team())
							{
								if (players[i].revealed()) apply_surface( (players[i].get_coords().x / 100)+7, (players[i].get_coords().y / 100)+7, radar_dot[1], screen);
							}
							else apply_surface( (players[i].get_coords().x / 100)+7, (players[i].get_coords().y / 100)+7, radar_dot[1], screen);break;
							case 5:
							if (players[this_player].get_team() != players[i].get_team())
							{
								if (players[i].revealed()) apply_surface( (players[i].get_coords().x / 100)+7, (players[i].get_coords().y / 100)+7, radar_dot[4], screen);
							}
							else apply_surface( (players[i].get_coords().x / 100)+7, (players[i].get_coords().y / 100)+7, radar_dot[4], screen);break;
							default: break;
						}
					}
				}
			}
			mothership[0] = players[this_mothership].get_coords();
			mothership[1] = players[enemy_mothership].get_coords();
			if (!players[this_player].do_render() && !players[this_mothership].do_render() && !paused)
			{
				int xW = (SCREEN_WIDTH / 2) - (741/2);
				int yH = (SCREEN_HEIGHT / 2) - (51/2); 
				apply_surface( xW, yH, menu[2], screen);
			}
		}
		int red_captured = 0;
		int blue_captured = 0;
		// Tally planet owners
		for (int i = 0; i < MAX_PLANETS; i++)
		{
			if (planets[i].get_owner() == 1) blue_captured ++;
			else if (planets[i].get_owner() == 2) red_captured ++;
		}
		// If red team has captured all the planets, display win
		if (red_captured >= MAX_PLANETS && !paused)
		{
			if (players[this_player].get_team() == 1)
			{
				int xW = (SCREEN_WIDTH / 2) - (741/2);
				int yH = (SCREEN_HEIGHT / 2) - (51/2); 
				apply_surface( xW, yH, menu[2], screen);
				players[this_player].destroy();
			}
			else
			{
				int xW = (SCREEN_WIDTH / 2) - (741/2);
				int yH = (SCREEN_HEIGHT / 2) - (51/2); 
				apply_surface( xW, yH, menu[3], screen);
			}
		}
		// Same for blue win
		if (blue_captured >= MAX_PLANETS && !paused) 
		{	
			if (players[this_player].get_team() == 1)
			{
				int xW = (SCREEN_WIDTH / 2) - (741/2);
				int yH = (SCREEN_HEIGHT / 2) - (51/2); 
				apply_surface( xW, yH, menu[3], screen);
			}
			else
			{
				int xW = (SCREEN_WIDTH / 2) - (741/2);
				int yH = (SCREEN_HEIGHT / 2) - (51/2); 
				apply_surface( xW, yH, menu[2], screen);
				players[this_player].destroy();
			}
		}
		//Update the screen
		if( SDL_Flip( screen ) == -1 )
		{
			return 1;    
		}
        //Cap the frame rate - remove if needed
        while( fps.get_ticks() < 1000 / FPS )
        {
            //wait    
        }
    }
    //Clean up any uneeded data
    clean_up( tiles );
    return 0;    
}
