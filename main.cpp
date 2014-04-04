/**********************************************************************

SLENDERBOY by Jason Guzon
Camera functions slightly modified from the Castle program by Philipp Crocoll

**********************************************************************
ESC: exit

CAMERA movement:
w : forwards
s : backwards
a : turn left
d : turn right
e : strafe right
q : strafe left

**********************************************************************/

   
#include <cmath>			//sine and cosine functions
#include <windows.h>		//exit(0)
#include "camera.h"			//By Philipp Crocoll, slightly modified by Jason Guzon
#include <GL/glut.h>		//includes gl.h and glu.h

#include "tga.h"			//For texture mapping

#include <stdlib.h>
#include <ctime>			//For random numbers
#include <algorithm>		//random_shuffle
#include <math.h>			//arctan2

#include <iostream>
using namespace std;

#define PI 3.1415265359

//For lighting
static GLfloat MatSpec[] = {1.0,1.0,1.0,1.0};
static GLfloat MatShininess[] = {45.0};
static GLfloat LightPos[] = {0.0,0.0,0.0,1.0};
static GLfloat ModelAmb[] = {0.5,0.5,0.5,0.0};

CCamera Camera; //Our camera

//For the trees
GLUquadricObj *obj;
int treeChooser[25];//For randomly choosing which trees get pages on them
GLfloat treeXPosition[25] = {0, -10, 2, 0, 7, -3, 10, 9, 8, 13, 9, 17, 15, 18, 19, 24, 32, 20, 22, 23, 29, 40, 36, 30, 45};//The x positions of each tree
GLfloat treeZPosition[25] = {0, -10, -20, -40, -34, -26, -18, -8, 5, -28, -45, -36, -14, 1, -22, -32, -40, -10, 10, -16, -8, 0, -32, -22, -14};//The y positions of each tree
//For example, tree i's coordinates are treeXPosition[i], treeZPosition[i]

//For lighting and material
GLfloat no_mat[] = { 0.0, 0.0, 0.0, 1.0 };
GLfloat mat_ambient[] = { 0.7, 0.7, 0.7, 1.0 };
GLfloat mat_low_ambience[] = { 0.5, 0.5, 0.5, 1.0 };
GLfloat mat_ambient_color[] = { 0.8, 0.8, 0.2, 1.0 };
GLfloat mat_diffuse[4] = { 0.0, 0.0, 0.0, 1.0 };
GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat no_shininess[] = { 0.0 };
GLfloat low_shininess[] = { 5.0 };
GLfloat high_shininess[] = { 100.0 };
GLfloat mat_emission[] = {0.3, 0.2, 0.2, 0.0};

//For texture
GLuint image[13];

//Variables for the pages. All 8 pages means you win
int pagesFound = 0;//Counts how many pages you have found
GLfloat glPagesFound = 0.0;//Same as pagesFound, just as a GLfloat for translating Slender at a speed according to how many pages have ben found
bool pageFound = true; //just in general
bool page1Found = false;
bool page2Found = false;
bool page3Found = false;
bool page4Found = false;
bool page5Found = false;
bool page6Found = false;
bool page7Found = false;
bool page8Found = false;

//For your character's movement
bool legalMove = true;

//Slenderboy's position
GLfloat slenderX = 50.0;
GLfloat slenderZ = -50.0;
bool slenderMoveX = true;//Checks if his movement is legal (or if he's trying to walk into a tree)
bool slenderMoveZ = true;
GLfloat moveSpeed = 1000.0;//The larger the number, the easier it is
GLfloat slenderAngle = 0;//For calculating his angle so he'll always be facing you

//Convert our positions to int to easily see if we lost
int myX;
int myZ;
int sX;
int sZ;
GLfloat rememberX;//To store our position so we can return from Bird's Eye View
GLfloat rememberZ;

bool youLost = false;

bool inBirdEyeView = false;

//Creature's variables
int nextTree = 0;//Which tree it will run to
bool treeReached = true;
GLfloat creatureX = 0.0;//Creature's current position
GLfloat creatureZ = 0.0;
int creatureCount = 0;//Only moves every 1000 steps/movements



//Load all the images into the program
void setup_All_Texture()
{
	CreateTexture(image,"tree.tga",0);
	CreateTexture(image,"grass.tga",1);
	CreateTexture(image,"suit.tga",2);
	CreateTexture(image,"black.tga",3);
	CreateTexture(image,"page1.tga",4);
	CreateTexture(image,"page2.tga",5);
	CreateTexture(image,"page3.tga",6);
	CreateTexture(image,"page4.tga",7);
	CreateTexture(image,"page5.tga",8);
	CreateTexture(image,"page6.tga",9);
	CreateTexture(image,"page7.tga",10);
	CreateTexture(image,"page8.tga",11);
	CreateTexture(image,"fence.tga",12);
}



//Function for drawing trees
void DrawTree(float xPosition, float zPosition)
{
	glEnable(GL_TEXTURE_2D);

	glPushMatrix();

	glBindTexture(GL_TEXTURE_2D, image[0]);//image[0] is tree.tga
	gluQuadricTexture(obj, GL_TRUE);
	gluQuadricDrawStyle(obj, GLU_FILL);
	glPolygonMode(GL_FRONT, GL_FILL);
	gluQuadricNormals(obj, GLU_SMOOTH);

	glMaterialfv(GL_FRONT, GL_AMBIENT, no_mat);
	mat_diffuse[0] = 1;
	mat_diffuse[1] = 1;
	mat_diffuse[2] = 1;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, no_mat);
	glMaterialfv(GL_FRONT, GL_SHININESS, no_shininess);
	glMaterialfv(GL_FRONT, GL_EMISSION, no_mat);

	glTranslatef(xPosition, 0, zPosition);//Translate to the desired position
	glRotatef(-90,1.0,0.0,0.0);//Rotate so it will be standing up instead of laying down
	gluCylinder(obj, 1.0, 1.0, 10.0, 12, 12);

	glPopMatrix();

	glDisable(GL_TEXTURE_2D);
}



//Function to draw Slenderboy
void DrawSlenderboy()
{
	if (youLost == true)
	{
		glDisable(GL_LIGHTING);//Makes him "glow" if you lost
	}

	glEnable(GL_TEXTURE_2D);

	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_low_ambience);
	mat_diffuse[0] = 0.5;
	mat_diffuse[1] = 0.5;
	mat_diffuse[2] = 0.5;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, no_mat);
	glMaterialfv(GL_FRONT, GL_SHININESS, no_shininess);
	glMaterialfv(GL_FRONT, GL_EMISSION, no_mat);


	//Draw his front torso with tux texture-mapped
	glBindTexture(GL_TEXTURE_2D, image[2]);

	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(-0.5, 3.0, 0.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(0.5, 3.0, 0.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(0.5, 5.0, 0.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(-0.5, 5.0, 0.0);
	glEnd();

	//Draw the rest of his torso (3 polygons, so it makes a box)
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, image[3]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(0.5, 3.0, -0.5);
	glTexCoord2f(0.01, 0.0); glVertex3f(0.5, 3.0, 0.0);
	glTexCoord2f(0.01, 0.01); glVertex3f(0.5, 5.0, 0.0);
	glTexCoord2f(0.0, 0.01); glVertex3f(0.5, 5.0, -0.5);
	glEnd();

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, image[3]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(-0.5, 3.0, -0.5);
	glTexCoord2f(0.01, 0.0); glVertex3f(-0.5, 3.0, 0.0);
	glTexCoord2f(0.01, 0.01); glVertex3f(-0.5, 5.0, 0.0);
	glTexCoord2f(0.0, 0.01); glVertex3f(-0.5, 5.0, -0.5);
	glEnd();

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, image[3]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(-0.5, 3.0, -0.5);
	glTexCoord2f(0.01, 0.0); glVertex3f(0.5, 3.0, -0.5);
	glTexCoord2f(0.01, 0.01); glVertex3f(0.5, 5.0, -0.5);
	glTexCoord2f(0.0, 0.01); glVertex3f(-0.5, 5.0, -0.5);
	glEnd();

	//Draw his legs (two cylinders)
	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, image[3]);
	gluQuadricTexture(obj, GL_TRUE);
	gluQuadricDrawStyle(obj, GLU_FILL);
	glPolygonMode(GL_FRONT, GL_FILL);
	gluQuadricNormals(obj, GLU_SMOOTH);

	glTranslatef(-0.25, 0.0, -0.25);
	glRotatef(-90,1.0,0.0,0.0);
	gluCylinder(obj, 0.225, 0.225, 3.0, 3, 3);

	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.25, 0.0, -0.25);
	glRotatef(-90,1.0,0.0,0.0);
	gluCylinder(obj, 0.225, 0.225, 3.0, 3, 3);
	glPopMatrix();

	//Draw his arms (also two cylinders)
	glPushMatrix();
	glTranslatef(0.9, 2.0, -0.25);
	glRotatef(5, 0.0, 0.0, 1.0);
	glRotatef(-90,1.0,0.0,0.0);
	gluCylinder(obj, 0.225, 0.225, 3.0, 3, 3);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-0.9, 2.0, -0.25);
	glRotatef(-5, 0.0, 0.0, 1.0);
	glRotatef(-90,1.0,0.0,0.0);
	gluCylinder(obj, 0.225, 0.225, 3.0, 3, 3);
	glPopMatrix();

	glDisable(GL_TEXTURE_2D);

	//Draw his hands (two spheres)
	glPushMatrix();
	glTranslatef(-0.9, 2.0, -0.25);
	glRotatef(-5, 0.0, 0.0, 1.0);
	glScalef(1, 1.5, 1);
	gluSphere(obj, 0.2, 10, 10); 
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.9, 2.0, -0.25);
	glRotatef(5, 0.0, 0.0, 1.0);
	glScalef(1, 1.5, 1);
	gluSphere(obj, 0.2, 10, 10); 
	glPopMatrix();

	//Draw his head (a sphere)
	glPushMatrix();
	glTranslatef(0.0, 5.5, -0.125);
	glRotatef(-5, 1.0, 0.0, 0.0);
	glScalef(1, 2, 1);
	gluSphere(obj, 0.325, 15, 15); 
	glPopMatrix();

	if (youLost == true)
	{
		glEnable(GL_LIGHTING);//So only Slenderboy "glows" if you lose
	}
}


//Function for moving Slenderboy
void moveSlender()
{
	if(slenderX > Camera.Position.x)//If Slender is more east of you
	{
		for (int i=0; i<25; i++)//Loops through each of the trees to make sure he is not going to walk there
		{
			if ((slenderX - ((glPagesFound*glPagesFound)/moveSpeed) >= (treeXPosition[i]-1.0)) && (slenderX - ((glPagesFound*glPagesFound)/moveSpeed) <= (treeXPosition[i]+1.0)) && slenderZ >= (treeZPosition[i]-1.0) && slenderZ <= (treeZPosition[i]+1.0))
			{
				slenderMoveX = false; //Invalid move if he is going to walk into any of the trees
			}
		}
		if (slenderMoveX == true)
		{
			slenderX = slenderX - ((glPagesFound*glPagesFound)/moveSpeed); //Moves along the X axis at a speed according to how many pages you've found
		}
		slenderMoveX = true; //Resets the bool
	}
	else if(slenderX < Camera.Position.x)//If Slender is more west of you
	{
		for (int i=0; i<25; i++)
		{
			if ((slenderX + ((glPagesFound*glPagesFound)/moveSpeed) >= (treeXPosition[i]-1.0)) && (slenderX + ((glPagesFound*glPagesFound)/moveSpeed) <= (treeXPosition[i]+1.0)) && slenderZ >= (treeZPosition[i]-1.0) && slenderZ <= (treeZPosition[i]+1.0))
			{
				slenderMoveX = false;
			}
		}
		if (slenderMoveX == true)
		{
			slenderX = slenderX + ((glPagesFound*glPagesFound)/moveSpeed);
		}
		slenderMoveX = true;
	}
	if(slenderZ > Camera.Position.z)//If Slender is more south of you
	{
		for (int i=0; i<25; i++)
		{
			if (slenderX >= (treeXPosition[i]-1.0) && slenderX <= (treeXPosition[i]+1.0) && (slenderZ - ((glPagesFound*glPagesFound)/moveSpeed) >= (treeZPosition[i]-1.0)) && (slenderZ - ((glPagesFound*glPagesFound)/moveSpeed) <= (treeZPosition[i]+1.0)))
			{
				slenderMoveZ = false;
			}
		}
		if (slenderMoveZ == true)
		{
			slenderZ = slenderZ - ((glPagesFound*glPagesFound)/moveSpeed);
		}
		slenderMoveZ = true;
	}
	else if(slenderZ < Camera.Position.z)//If Slender is more north of you
	{
		for (int i=0; i<25; i++)
		{
			if (slenderX >= (treeXPosition[i]-1.0) && slenderX <= (treeXPosition[i]+1.0) && (slenderZ + ((glPagesFound*glPagesFound)/moveSpeed) >= (treeZPosition[i]-1.0)) && (slenderZ + ((glPagesFound*glPagesFound)/moveSpeed) <= (treeZPosition[i]+1.0)))
			{
				slenderMoveZ = false;
			}
		}
		if (slenderMoveZ == true)
		{
			slenderZ = slenderZ + ((glPagesFound*glPagesFound)/moveSpeed);
		}
		slenderMoveZ = true;
	}
	slenderMoveX = true;//Two bools so he can walk diagonally in one step or horizontally/vertically
	slenderMoveZ = true;

	//Change Slender's angle so he's always looking at you
	GLfloat currSX = slenderX;
	GLfloat currSZ = slenderZ;
	GLfloat myCurrX = Camera.Position.x;
	GLfloat myCurrZ = Camera.Position.z;
	GLfloat deltaZ = currSZ - myCurrZ;
	GLfloat deltaX = myCurrX - currSX;
	slenderAngle = 90 + atan2(deltaZ,deltaX)*180/PI;
}


//Functions for drawing each page
void drawPage1()//Pages #1 and #2 go on south sides of trees
{
	glEnable(GL_TEXTURE_2D);

	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_low_ambience);
	mat_diffuse[0] = 1;
	mat_diffuse[1] = 1;
	mat_diffuse[2] = 1;
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, no_mat);
	glMaterialfv(GL_FRONT, GL_SHININESS, no_shininess);
	glMaterialfv(GL_FRONT, GL_EMISSION, no_mat);


	//Draw page #1
	glBindTexture(GL_TEXTURE_2D, image[4]);

	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(-0.32, 4.5, 1.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(0.32, 4.5, 1.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(0.32, 5.5, 1.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(-0.32, 5.5, 1.0);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void drawPage2()
{
	glEnable(GL_TEXTURE_2D);

	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_low_ambience);
	mat_diffuse[0] = 1;
	mat_diffuse[1] = 1;
	mat_diffuse[2] = 1;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, no_mat);
	glMaterialfv(GL_FRONT, GL_SHININESS, no_shininess);
	glMaterialfv(GL_FRONT, GL_EMISSION, no_mat);


	//Draw page #2
	glBindTexture(GL_TEXTURE_2D, image[5]);

	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(-0.32, 4.5, 1.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(0.32, 4.5, 1.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(0.32, 5.5, 1.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(-0.32, 5.5, 1.0);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void drawPage3()//Pages #3 and #4 go on west sides of trees
{
	glEnable(GL_TEXTURE_2D);

	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_low_ambience);
	mat_diffuse[0] = 1;
	mat_diffuse[1] = 1;
	mat_diffuse[2] = 1;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, no_mat);
	glMaterialfv(GL_FRONT, GL_SHININESS, no_shininess);
	glMaterialfv(GL_FRONT, GL_EMISSION, no_mat);


	//Draw page #3
	glBindTexture(GL_TEXTURE_2D, image[6]);

	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(-1.0, 4.5, -0.32);
	glTexCoord2f(1.0, 0.0); glVertex3f(-1.0, 4.5, 0.32);
	glTexCoord2f(1.0, 1.0); glVertex3f(-1.0, 5.5, 0.32);
	glTexCoord2f(0.0, 1.0); glVertex3f(-1.0, 5.5, -0.32);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void drawPage4()
{
	glEnable(GL_TEXTURE_2D);

	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_low_ambience);
	mat_diffuse[0] = 1;
	mat_diffuse[1] = 1;
	mat_diffuse[2] = 1;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, no_mat);
	glMaterialfv(GL_FRONT, GL_SHININESS, no_shininess);
	glMaterialfv(GL_FRONT, GL_EMISSION, no_mat);


	//Draw page #4
	glBindTexture(GL_TEXTURE_2D, image[7]);

	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(-1.0, 4.5, -0.32);
	glTexCoord2f(1.0, 0.0); glVertex3f(-1.0, 4.5, 0.32);
	glTexCoord2f(1.0, 1.0); glVertex3f(-1.0, 5.5, 0.32);
	glTexCoord2f(0.0, 1.0); glVertex3f(-1.0, 5.5, -0.32);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void drawPage5()//Pages #5 and #6 go on north sides of trees
{
	glEnable(GL_TEXTURE_2D);

	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_low_ambience);
	mat_diffuse[0] = 1;
	mat_diffuse[1] = 1;
	mat_diffuse[2] = 1;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, no_mat);
	glMaterialfv(GL_FRONT, GL_SHININESS, no_shininess);
	glMaterialfv(GL_FRONT, GL_EMISSION, no_mat);


	//Draw page #5
	glBindTexture(GL_TEXTURE_2D, image[8]);

	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);

	glBegin(GL_QUADS);
	glTexCoord2f(1.0, 0.0); glVertex3f(-0.32, 4.5, -1.0);
	glTexCoord2f(0.0, 0.0); glVertex3f(0.32, 4.5, -1.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(0.32, 5.5, -1.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(-0.32, 5.5, -1.0);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void drawPage6()
{
	glEnable(GL_TEXTURE_2D);

	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_low_ambience);
	mat_diffuse[0] = 1;
	mat_diffuse[1] = 1;
	mat_diffuse[2] = 1;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, no_mat);
	glMaterialfv(GL_FRONT, GL_SHININESS, no_shininess);
	glMaterialfv(GL_FRONT, GL_EMISSION, no_mat);


	//Draw page #6
	glBindTexture(GL_TEXTURE_2D, image[9]);

	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);

	glBegin(GL_QUADS);
	glTexCoord2f(1.0, 0.0); glVertex3f(-0.32, 4.5, -1.0);
	glTexCoord2f(0.0, 0.0); glVertex3f(0.32, 4.5, -1.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(0.32, 5.5, -1.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(-0.32, 5.5, -1.0);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void drawPage7()//Pages #7 and #8 go on east sides of trees
{
	glEnable(GL_TEXTURE_2D);

	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_low_ambience);
	mat_diffuse[0] = 1;
	mat_diffuse[1] = 1;
	mat_diffuse[2] = 1;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, no_mat);
	glMaterialfv(GL_FRONT, GL_SHININESS, no_shininess);
	glMaterialfv(GL_FRONT, GL_EMISSION, no_mat);


	//Draw page #7
	glBindTexture(GL_TEXTURE_2D, image[10]);

	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);

	glBegin(GL_QUADS);
	glTexCoord2f(1.0, 0.0); glVertex3f(1.0, 4.5, -0.32);
	glTexCoord2f(0.0, 0.0); glVertex3f(1.0, 4.5, 0.32);
	glTexCoord2f(0.0, 1.0); glVertex3f(1.0, 5.5, 0.32);
	glTexCoord2f(1.0, 1.0); glVertex3f(1.0, 5.5, -0.32);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void drawPage8()
{
	glEnable(GL_TEXTURE_2D);

	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_low_ambience);
	mat_diffuse[0] = 1;
	mat_diffuse[1] = 1;
	mat_diffuse[2] = 1;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, no_mat);
	glMaterialfv(GL_FRONT, GL_SHININESS, no_shininess);
	glMaterialfv(GL_FRONT, GL_EMISSION, no_mat);


	//Draw page #8
	glBindTexture(GL_TEXTURE_2D, image[11]);

	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);

	glBegin(GL_QUADS);
	glTexCoord2f(1.0, 0.0); glVertex3f(1.0, 4.5, -0.32);
	glTexCoord2f(0.0, 0.0); glVertex3f(1.0, 4.5, 0.32);
	glTexCoord2f(0.0, 1.0); glVertex3f(1.0, 5.5, 0.32);
	glTexCoord2f(1.0, 1.0); glVertex3f(1.0, 5.5, -0.32);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}


//Function for drawing all the pages (calls the above functions if they were not found yet)
void DrawPages()
{
	if (page1Found == false)
	{
		glPushMatrix();
		glTranslatef(treeXPosition[treeChooser[0]],0,treeZPosition[treeChooser[0]]);
		drawPage1();
		glPopMatrix();
	}
	if (page2Found == false)
	{
		glPushMatrix();
		glTranslatef(treeXPosition[treeChooser[1]],0,treeZPosition[treeChooser[1]]);
		drawPage2();
		glPopMatrix();
	}
	if (page3Found == false)
	{
		glPushMatrix();
		glTranslatef(treeXPosition[treeChooser[2]],0,treeZPosition[treeChooser[2]]);
		drawPage3();
		glPopMatrix();
	}
	if (page4Found == false)
	{
		glPushMatrix();
		glTranslatef(treeXPosition[treeChooser[3]],0,treeZPosition[treeChooser[3]]);
		drawPage4();
		glPopMatrix();
	}
	if (page5Found == false)
	{
		glPushMatrix();
		glTranslatef(treeXPosition[treeChooser[4]],0,treeZPosition[treeChooser[4]]);
		drawPage5();
		glPopMatrix();
	}
	if (page6Found == false)
	{
		glPushMatrix();
		glTranslatef(treeXPosition[treeChooser[5]],0,treeZPosition[treeChooser[5]]);
		drawPage6();
		glPopMatrix();
	}
	if (page7Found == false)
	{
		glPushMatrix();
		glTranslatef(treeXPosition[treeChooser[6]],0,treeZPosition[treeChooser[6]]);
		drawPage7();
		glPopMatrix();
	}
	if (page8Found == false)
	{
		glPushMatrix();
		glTranslatef(treeXPosition[treeChooser[7]],0,treeZPosition[treeChooser[7]]);
		drawPage8();
		glPopMatrix();
	}
}


//Function for drawing the creature
void drawCreature(void)
{
	//If the creature is at his destination, wait for creatureCount again
	if (creatureX == treeXPosition[nextTree] && creatureZ == treeZPosition[nextTree])
	{
		treeReached = true;
		creatureCount = 0;//Reset creatureCount
	}

	//If the creature is more west than its destination, move eastward
	if(creatureX < treeXPosition[nextTree])
	{
		creatureX = creatureX + 0.5;
	}
	else if (creatureX > treeXPosition[nextTree])//More east, move westward
	{
		creatureX = creatureX - 0.5;
	}
	else;

	if(creatureZ < treeZPosition[nextTree])//More north, move southward
	{
		creatureZ = creatureZ + 0.5;
	}
	else if (creatureZ > treeZPosition[nextTree])//More south, move northward
	{
		creatureZ = creatureZ - 0.5;
	}
	else;

	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_low_ambience);
	mat_diffuse[0] = 0.5;
	mat_diffuse[1] = 0.5;
	mat_diffuse[2] = 0.5;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, no_mat);
	glMaterialfv(GL_FRONT, GL_SHININESS, no_shininess);
	glMaterialfv(GL_FRONT, GL_EMISSION, no_mat);

	glDisable(GL_TEXTURE_2D);
	glPushMatrix();
	glTranslatef(creatureX, 0.0, creatureZ);//Translate creature at the desired position
	glScalef(1, 1, 1.5);
	gluSphere(obj, 0.25, 10, 10); //Draw creature (just a sphere)
	glPopMatrix();
}


void Init(void) //Determines which trees have the pages
{
	srand((unsigned)time(0));//Generates a random number
	for(int i=0;i<25;i++)//Loads numbers 0 - 24 into treeChooser
	{
		treeChooser[i] = i;
	}
	random_shuffle(treeChooser, treeChooser+25);//Shuffles the order, and the first 8 in this order will have the pages
	cout << "Pages Found = " << pagesFound << endl; //So it will say Pages Found = 0
}


//Main display function
void Display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();	
	Camera.Render();

	glColor3d(1, 1, 1);

	//Draw the floor
	glEnable(GL_TEXTURE_2D);

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_low_ambience);
	if (youLost == true)
	{
		glMaterialfv(GL_FRONT, GL_AMBIENT, no_mat);//Makes the ground black if you lose
	}
	mat_diffuse[0] = 1.0;
	mat_diffuse[1] = 1.0;
	mat_diffuse[2] = 1.0;
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, no_mat);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, no_shininess);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, no_mat);

	glBindTexture(GL_TEXTURE_2D, image[1]);

	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST); 

	glBegin(GL_QUADS);
	glTexCoord2f(-2, -2); glVertex3f(-20, 0, 20);
	glTexCoord2f(-2, 5); glVertex3f(-20, 0, -50);
	glTexCoord2f(5, 5); glVertex3f(50, 0, -50);
	glTexCoord2f(5, -2); glVertex3f(50, 0, 20);
	glEnd();

	//Draw the fences
	glBindTexture(GL_TEXTURE_2D, image[12]);

	//South fence (z=20)
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 3.5); glVertex3f(-20, 7, 20);
	glTexCoord2f(35.0, 3.5); glVertex3f(50, 7, 20);
	glTexCoord2f(0.0, 0.0); glVertex3f(-20, 0, 20);
	glTexCoord2f(35.0, 0.0); glVertex3f(50, 0, 20);
	glEnd();

	//North fence (z=-50)
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 3.5); glVertex3f(-20, 7, -50);
	glTexCoord2f(35.0, 3.5); glVertex3f(50, 7, -50);
	glTexCoord2f(0.0, 0.0); glVertex3f(-20, 0, -50);
	glTexCoord2f(35.0, 0.0); glVertex3f(50, 0, -50);
	glEnd();

	//West fence (x=-20)
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 3.5); glVertex3f(-20, 7, 20);
	glTexCoord2f(35.0, 3.5); glVertex3f(-20, 7, -50);
	glTexCoord2f(0.0, 0.0); glVertex3f(-20, 0, 20);
	glTexCoord2f(35.0, 0.0); glVertex3f(-20, 0, -50);
	glEnd();

	//East fence (x=50)
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 3.5); glVertex3f(50, 7, 20);
	glTexCoord2f(35.0, 3.5); glVertex3f(50, 7, -50);
	glTexCoord2f(0.0, 0.0); glVertex3f(50, 0, 20);
	glTexCoord2f(35.0, 0.0); glVertex3f(50, 0, -50);
	glEnd();

	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);


	//Draw all the trees
	for(int i=0; i<25; i++)
	{
		DrawTree(treeXPosition[i], treeZPosition[i]);
	}


	glPushMatrix();
	if(inBirdEyeView == false && pagesFound < 8)
	{
		moveSlender();//Slender only moves if you're not in bird eye view and you have not won yet
	}
	glTranslatef(slenderX,0,slenderZ);
	glRotatef(slenderAngle,0.0,1.0,0.0);
	DrawSlenderboy();
	glPopMatrix();

	glPushMatrix();
	DrawPages();//Draw the pages which have not yet been found
	glPopMatrix();
	
	//For drawing the creature
	creatureCount = creatureCount + 1;
	if (creatureCount == 1000)//After 1000 steps/movements, the creature will run from one tree to another
	{
		treeReached = false;
		int floor = 0, ceiling = 24, range = (ceiling - floor);
		int rnd = floor + int((range * rand()) / (RAND_MAX + 1.0)); //Picks a random number from 0-24 and runs to that tree
		nextTree = rnd;
	}
	if (treeReached == false)
	{
		drawCreature();
	}

	if(pagesFound == 8)
	{
		cout << "Congratulations! You won!" << endl;
		glDisable(GL_LIGHTING);//"Lights up" the entire place if you won
	}

	//Uses ints to see if you and Slender are in about the same area. If so, you have lost
	myX = Camera.Position.x;
	myZ = Camera.Position.z;
	sX = slenderX;
	sZ = slenderZ;
	if (myX == sX && myZ == sZ) //YOU HAVE LOST THE GAME
	{
		cout << "Sorry, you lost..." << endl;
		youLost = true;
		glLightfv(GL_LIGHT0,GL_DIFFUSE,no_mat);//Makes lighting black so all you can see is Slender
		glLightfv(GL_LIGHT0,GL_SPECULAR,no_mat);
	}

	glFlush();			//Finish rendering
	glutSwapBuffers();	//Swap the buffers ->make the result of rendering visible
}



void Reshape(int x, int y)
{
	if (y == 0 || x == 0) return;  //Nothing is visible then, so return
	//Set a new projection matrix

	glViewport(0,0,x,y);  

	glMatrixMode(GL_PROJECTION);  
	glLoadIdentity();
	//Angle of view:40 degrees
	//Near clipping plane distance: 0.5
	//Far clipping plane distance: 20.0
	gluPerspective(40.0,(GLdouble)x/(GLdouble)y,1.0,200.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glLightfv(GL_LIGHT0, GL_POSITION, LightPos);
}


//Function for clicking on the pages
void mouse(int btn, int state, int x, int y)
{
	switch (btn)
	{
    case GLUT_LEFT_BUTTON://Must be a left click
        if(state == GLUT_DOWN)
        {
			if (
				//Checks if you are near page #1 within a 1.5 x 4 box
				page1Found == false &&
				Camera.Position.x >= (treeXPosition[treeChooser[0]]-1.5) &&
				Camera.Position.x <= (treeXPosition[treeChooser[0]]+1.5) &&
				Camera.Position.z >= (treeZPosition[treeChooser[0]]) &&
				Camera.Position.z <= (treeZPosition[treeChooser[0]]+4)
				)
			{
				//If so, a left click will take down the page and add it to your "found"
				page1Found = true;
				pagesFound = pagesFound + 1;
				glPagesFound = glPagesFound + 1.0;
				cout << "Pages Found = " << pagesFound << endl;
				Display();
			}
			if (
				page2Found == false &&
				Camera.Position.x >= (treeXPosition[treeChooser[1]]-1.5) &&
				Camera.Position.x <= (treeXPosition[treeChooser[1]]+1.5) &&
				Camera.Position.z >= (treeZPosition[treeChooser[1]]) &&
				Camera.Position.z <= (treeZPosition[treeChooser[1]]+4)
				)
			{
				page2Found = true;
				pagesFound = pagesFound + 1;
				glPagesFound = glPagesFound + 1.0;
				cout << "Pages Found = " << pagesFound << endl;
				Display();
			}
			if (
				page3Found == false &&
				Camera.Position.x >= (treeXPosition[treeChooser[2]]-4) &&
				Camera.Position.x <= (treeXPosition[treeChooser[2]]) &&
				Camera.Position.z >= (treeZPosition[treeChooser[2]]-1.5) &&
				Camera.Position.z <= (treeZPosition[treeChooser[2]]+1.5)
				)
			{
				page3Found = true;
				pagesFound = pagesFound + 1;
				glPagesFound = glPagesFound + 1.0;
				cout << "Pages Found = " << pagesFound << endl;
				Display();
			}
			if (
				page4Found == false &&
				Camera.Position.x >= (treeXPosition[treeChooser[3]]-4) &&
				Camera.Position.x <= (treeXPosition[treeChooser[3]]) &&
				Camera.Position.z >= (treeZPosition[treeChooser[3]]-1.5) &&
				Camera.Position.z <= (treeZPosition[treeChooser[3]]+1.5)
				)
			{
				page4Found = true;
				pagesFound = pagesFound + 1;
				glPagesFound = glPagesFound + 1.0;
				cout << "Pages Found = " << pagesFound << endl;
				Display();
			}
			if (
				page5Found == false &&
				Camera.Position.x >= (treeXPosition[treeChooser[4]]-1.5) &&
				Camera.Position.x <= (treeXPosition[treeChooser[4]]+1.5) &&
				Camera.Position.z >= (treeZPosition[treeChooser[4]]-4) &&
				Camera.Position.z <= (treeZPosition[treeChooser[4]])
				)
			{
				page5Found = true;
				pagesFound = pagesFound + 1;
				glPagesFound = glPagesFound + 1.0;
				cout << "Pages Found = " << pagesFound << endl;
				Display();
			}
			if (
				page6Found == false &&
				Camera.Position.x >= (treeXPosition[treeChooser[5]]-1.5) &&
				Camera.Position.x <= (treeXPosition[treeChooser[5]]+1.5) &&
				Camera.Position.z >= (treeZPosition[treeChooser[5]]-4) &&
				Camera.Position.z <= (treeZPosition[treeChooser[5]])
				)
			{
				page6Found = true;
				pagesFound = pagesFound + 1;
				glPagesFound = glPagesFound + 1.0;
				cout << "Pages Found = " << pagesFound << endl;
				Display();
			}
			if (
				page7Found == false &&
				Camera.Position.x >= (treeXPosition[treeChooser[6]]) &&
				Camera.Position.x <= (treeXPosition[treeChooser[6]]+4) &&
				Camera.Position.z >= (treeZPosition[treeChooser[6]]-1.5) &&
				Camera.Position.z <= (treeZPosition[treeChooser[6]]+1.5)
				)
			{
				page7Found = true;
				pagesFound = pagesFound + 1;
				glPagesFound = glPagesFound + 1.0;
				cout << "Pages Found = " << pagesFound << endl;
				Display();
			}
			if (
				page8Found == false &&
				Camera.Position.x >= (treeXPosition[treeChooser[7]]) &&
				Camera.Position.x <= (treeXPosition[treeChooser[7]]+4) &&
				Camera.Position.z >= (treeZPosition[treeChooser[7]]-1.5) &&
				Camera.Position.z <= (treeZPosition[treeChooser[7]]+1.5)
				)
			{
				page8Found = true;
				pagesFound = pagesFound + 1;
				glPagesFound = glPagesFound + 1.0;
				cout << "Pages Found = " << pagesFound << endl;
				Display();
			}
        }
    break;
    }
}

//For keyboard movement
void KeyDown(unsigned char key, int x, int y)
{	
	switch(key)
	{
	case 27:	//ESC, exits the game
		exit(0);
		break;
	case 'a':		//Rotates left
		Camera.RotateY(5.0);
		Display();
		break;
	case 'd':		//Rotates right
		Camera.RotateY(-5.0);
		Display();
		break;
	case 'w':		//Moves forward
		if (Camera.ViewDirChanged) Camera.GetViewDir();//Gets the direction you are facing
		for (int i=0; i<25; i++) //Loops through each of the trees' positions. If moving forward makes you walk into a tree, legalMove is set to false
		{
			if (Camera.Position.x+(Camera.ViewDir.x*0.3) >= (treeXPosition[i]-1.0) && Camera.Position.x+(Camera.ViewDir.x*0.3) <= (treeXPosition[i]+1.0) && Camera.Position.z+(Camera.ViewDir.z*0.3) >= (treeZPosition[i]-1.0) && Camera.Position.z+(Camera.ViewDir.z*0.3) <= (treeZPosition[i]+1.0))
			{
				legalMove = false;
				break;
			}
		}
		if(
			//If you are trying to walk past the fence, legalMove is set to false
			(Camera.Position.x+(Camera.ViewDir.x*0.3) >= (50)) ||
			(Camera.Position.x+(Camera.ViewDir.x*0.3) <= (-20)) ||
			(Camera.Position.z+(Camera.ViewDir.z*0.3) >= (20)) ||
			(Camera.Position.z+(Camera.ViewDir.z*0.3) <= (-50))
		)
		{
			legalMove = false;
		}
		if(legalMove == true)
		{
			Camera.MoveForwards( -0.3 ) ;//Moves and displays if you are trying to take a legalMove
			Display();
		}
		legalMove = true;
		break;
	case 's':		//Moves backwards
		if (Camera.ViewDirChanged) Camera.GetViewDir();
		for (int i=0; i<25; i++)//Loops through the trees locations and sees if any of them are in the way (so that you cannot walk in that direction any longer)
		{
			if (Camera.Position.x+(Camera.ViewDir.x*(-0.3)) >= (treeXPosition[i]-1.0) && Camera.Position.x+(Camera.ViewDir.x*(-0.3)) <= (treeXPosition[i]+1.0) && Camera.Position.z+(Camera.ViewDir.z*(-0.3)) >= (treeZPosition[i]-1.0) && Camera.Position.z+(Camera.ViewDir.z*(-0.3)) <= (treeZPosition[i]+1.0))
			{
				legalMove = false;
				break;
			}
		}
		if( //Checks if you have reached the borders of the game
			(Camera.Position.x+(Camera.ViewDir.x*(-0.3)) >= (50)) ||
			(Camera.Position.x+(Camera.ViewDir.x*(-0.3)) <= (-20)) ||
			(Camera.Position.z+(Camera.ViewDir.z*(-0.3)) >= (20)) ||
			(Camera.Position.z+(Camera.ViewDir.z*(-0.3)) <= (-50))
		)
		{
			legalMove = false;
		}
		if(legalMove == true)
		{
			Camera.MoveForwards( 0.3 ) ;
			Display();
		}
		legalMove = true;
		break;
	case 'q':		//Move left
		if (Camera.ViewDirChanged) Camera.GetViewDir();
		for (int i=0; i<25; i++)
		{
			if (Camera.Position.x+(Camera.ViewDir.z*0.3) >= (treeXPosition[i]-1.0) && Camera.Position.x+(Camera.ViewDir.z*0.3) <= (treeXPosition[i]+1.0) && Camera.Position.z+(-Camera.ViewDir.x*0.3) >= (treeZPosition[i]-1.0) && Camera.Position.z+(-Camera.ViewDir.x*0.3) <= (treeZPosition[i]+1.0))
			{
				legalMove = false;
				break;
			}
		}
		if(
			(Camera.Position.x+(Camera.ViewDir.z*0.3) >= (50)) ||
			(Camera.Position.x+(Camera.ViewDir.z*0.3) <= (-20)) ||
			(Camera.Position.z+(-Camera.ViewDir.x*0.3) >= (20)) ||
			(Camera.Position.z+(-Camera.ViewDir.x*0.3) <= (-50))
		)
		{
			legalMove = false;
		}
		if(legalMove == true)
		{
			Camera.StrafeRight( -0.3 ) ;
			Display();
		}
		legalMove = true;
		break;
	case 'e':		//Move right
		if (Camera.ViewDirChanged) Camera.GetViewDir();
		for (int i=0; i<25; i++)
		{
			if (Camera.Position.x+(Camera.ViewDir.z*(-0.3)) >= (treeXPosition[i]-1.0) && Camera.Position.x+(Camera.ViewDir.z*(-0.3)) <= (treeXPosition[i]+1.0) && Camera.Position.z+(-Camera.ViewDir.x*(-0.3)) >= (treeZPosition[i]-1.0) && Camera.Position.z+(-Camera.ViewDir.x*(-0.3)) <= (treeZPosition[i]+1.0))
			{
				legalMove = false;
				break;
			}
		}
		if(
			(Camera.Position.x+(Camera.ViewDir.z*(-0.3)) >= (50)) ||
			(Camera.Position.x+(Camera.ViewDir.z*(-0.3)) <= (-20)) ||
			(Camera.Position.z+(-Camera.ViewDir.x*(-0.3)) >= (20)) ||
			(Camera.Position.z+(-Camera.ViewDir.x*(-0.3)) <= (-50))
		)
		{
			legalMove = false;
		}
		if(legalMove == true)
		{
			Camera.StrafeRight( 0.3 ) ;
			Display();
		}
		legalMove = true;
		break;

	
	}
}


//Function for exiting bird eye view. Sets things back to normal
void exitBirdEye()
{
	inBirdEyeView = false;
	glEnable(GL_LIGHTING);
	glEnable(GL_FOG);
	Camera.Move(F3dVector(0.0,-30,0.0));
	Camera.RotateX(90.0);
	Camera.Position.x = rememberX;
	Camera.Position.z = rememberZ;
	glutKeyboardFunc(KeyDown);
	glutIdleFunc(NULL);
	Display();
}


//Automatically moves the camera in a box while in Bird Eye View
void birdIdle(void)
{
	if(Camera.Position.x > 0 && Camera.Position.z == 0)
	{
		Camera.Position.x = Camera.Position.x - 0.0625;
		Display();
	}
	else if(Camera.Position.x == 0 && Camera.Position.z > -40)
	{
		Camera.Position.z = Camera.Position.z - 0.0625;
		Display();
	}
	else if(Camera.Position.x < 40 && Camera.Position.z == -40)
	{
		Camera.Position.x = Camera.Position.x + 0.0625;
		Display();
	}
	else if(Camera.Position.x == 40 && Camera.Position.z < 0)
	{
		Camera.Position.z = Camera.Position.z + 0.0625;
		Display();
	}
}

//Bird eye view (activated by right clicking and choosing it from the dropdown menu
void birdEyeView()
{
	inBirdEyeView = true;
	glDisable(GL_LIGHTING);//Turns off lighting so you can see
	glDisable(GL_FOG);//Turns off fog so you can see
	Camera.Move(F3dVector(0.0,30,0.0));//Moves you up to y 30 units
	Camera.RotateX(-90.0);//Rotates you to face downward
	rememberX = Camera.Position.x;//Remember the position you were at so you can return there
	rememberZ = Camera.Position.z;
	Camera.Position.x = 40.0;
	Camera.Position.z = 0.0;
	glutKeyboardFunc(NULL);//Deactivates the keyboard functions so you cannot move while in Bird Eye View Mode
	glutIdleFunc(birdIdle);//Calls birdIdle to automatically pan the camera
	
}


//Dropdown menu from right clicking
void myMenuFunction(int id)
{
	
   if(id < 6) 
   {	
	   if(id == 1) //Change to Bird Eye View
	   {
		   if (inBirdEyeView == false)
		   {
			   birdEyeView();
		   }
	   }
	   else if(id == 2)//Return from Bird Eye View
	   {
		   if (inBirdEyeView == true)
		   {
			   exitBirdEye();
		   }
	   }
	   else if(id == 3)//Easy Mode
	   {
		   moveSpeed = 2000.0;
	   }
	   else if(id == 4)//Medium mode
	   {
		   moveSpeed = 1000.0;
	   }
	   else if(id == 5)//Hard mode
	   {
		   moveSpeed = 500.0;
	   }
	   else
		   exit(0);
   }
   else
	   exit(0);
}

int main(int argc, char **argv)
{	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(300,300);
	glutCreateWindow("Slenderboy");

	setup_All_Texture();

	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	glClearColor(0.0,0.0,0.0,0.0);
	obj = gluNewQuadric();
	gluQuadricNormals(obj, GLU_SMOOTH);
	glutDisplayFunc(Display);
	glutReshapeFunc(Reshape);
	glutMouseFunc(mouse);

	//register the menu function
    glutCreateMenu(myMenuFunction);

	//set up the menu
    glutAddMenuEntry("Enter Bird-Eye View", 1);
	glutAddMenuEntry("Exit Bird-Eye View", 2);
	glutAddMenuEntry("Easy Mode", 3);
	glutAddMenuEntry("Medium Mode (Default)", 4);
	glutAddMenuEntry("Hard Mode", 5);

	//Attach one button to the menu
    glutAttachMenu(GLUT_RIGHT_BUTTON);

	glutKeyboardFunc(KeyDown);
	Camera.Move(F3dVector(0.0,5.0,19.0));

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 45.0);
	glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 128);
	glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.005);

	glFogi (GL_FOG_MODE, GL_LINEAR);
	glFogf(GL_FOG_START, 20.0);
	glFogf(GL_FOG_END, 30.0);
	glHint(GL_FOG_HINT, GL_DONT_CARE);
	glEnable(GL_FOG);

	Init();
	glutMainLoop();
	return 0;
}