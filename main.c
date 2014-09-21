#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <GL/freeglut.h>

// Types
typedef struct vec_t{
	double x;
	double y;
	double z;
} vector;

// Domain
double length;
vector d1, d2, d3, d4, d5, d6, d7, d8;
char running;

// Rendering
int camera_type;
char wireframe;

// Sphere
long drop_hit;
double radius;
vector velocity;
vector center;

// Domain
long ground_drop_hit;

// Rain
double volume;
int num_rain_drops;
vector rain_velocity;
vector *rain;

double time = 0.0;
double dt;
double limit;

// Frame
int width;
int height;

/**
*
**/
void finish(void){
	// Free memory and Exit application
	printf("Cleaning memory and exiting\n");
	free(rain);
	exit(EXIT_SUCCESS);
}

/**
*
**/
void draw_rain(void){
	glBegin(GL_POINTS);
    glColor3f(0.f, 0.9f, 1.f);

	for(int i=0; i<num_rain_drops; i++){
        glVertex3f(rain[i].x, rain[i].y, rain[i].z);
	}

    glEnd();
}

/**
*
**/
void reshape(GLint w, GLint h){
	width = w;
	height = h;
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(100.0, (float)width / height, 0.0001f, 10000.f);
	glMatrixMode(GL_MODELVIEW);
}

/**
*
**/
void keypress(unsigned char key, int x, int y){
	int number = key-48;
	if(number > 0 && number < 5){
		camera_type = number;
	}
}

/**
*
**/
void display(void){
	// Clear frame buffer and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// Camera
	glLoadIdentity();
	if(camera_type == 1){	
		gluLookAt(0, 0, 2*length, 0, 0, 0, 0, 1, 0);
	}else if(camera_type == 2){
		gluLookAt(center.x+(radius*3.0), center.y, center.z, center.x+1.0, center.y, center.z, 0, 1, 0);
	}else if(camera_type == 3){
		gluLookAt(center.x-(radius*3.0), center.y, center.z, center.x+1.0, center.y, center.z, 0, 1, 0);
	}else if(camera_type == 4){
		gluLookAt(center.x, center.y, center.z-(radius*2.0), center.x, center.y, center.z+1.0, 0, 1, 0);
	}

	// Sphere speed
	char buff[50];
	sprintf(buff, "Sphere speed {%f, %f, %f}", velocity.x, velocity.y, velocity.z);
	glRasterPos2i(-30, 22);
    glColor3f(1.f, 1.f, 1.f);
	glutBitmapString(GLUT_BITMAP_HELVETICA_18, buff);

	// Rain speed
	sprintf(buff, "Rain speed {%f, %f, %f}", rain_velocity.x, rain_velocity.y, rain_velocity.z);
	glRasterPos2i(-30, 20);
    glColor3f(1.f, 1.f, 1.f);
	glutBitmapString(GLUT_BITMAP_HELVETICA_18, buff);

	// Ground hit count
	sprintf(buff, "Ground drops hit %lu", ground_drop_hit);
	glRasterPos2i(-30, 18);
    glColor3f(1.f, 1.f, 1.f);
	glutBitmapString(GLUT_BITMAP_HELVETICA_18, buff);

	// Hit count
	sprintf(buff, "Drops hit %lu", drop_hit);
	glRasterPos2i(-30, 16);
    glColor3f(1.f, 1.f, 1.f);
	glutBitmapString(GLUT_BITMAP_HELVETICA_18, buff);

	// Rain drops
	sprintf(buff, "Drops %d", num_rain_drops);
	glRasterPos2i(-30, 14);
    glColor3f(1.f, 1.f, 1.f);
	glutBitmapString(GLUT_BITMAP_HELVETICA_18, buff);

	// Delta time
	sprintf(buff, "Dt = %f", dt);
	glRasterPos2i(-30, 12);
    glColor3f(1.f, 1.f, 1.f);
	glutBitmapString(GLUT_BITMAP_HELVETICA_18, buff);

	// Time
	sprintf(buff, "Time %f", time);
	glRasterPos2i(-30, 10);
    glColor3f(1.f, 1.f, 1.f);
	glutBitmapString(GLUT_BITMAP_HELVETICA_18, buff);
	
	// mm per hour
	double mm_per_hour = (3600.0/time) * ((ground_drop_hit * volume)/(length*length)) * 1000;
	sprintf(buff, "mm/hour %f", mm_per_hour);
	glRasterPos2i(-30, 8);
    glColor3f(1.f, 1.f, 1.f);
	glutBitmapString(GLUT_BITMAP_HELVETICA_18, buff);

	// Draw
	draw_rain();
	
	// Draw sphere
    glColor3f(1.f, 1.f, 1.f);
	glTranslatef(center.x, center.y, center.z);
	if(wireframe){	
		glutWireSphere(radius, 30, 30);
	}else{
		glutSolidSphere(radius, 30, 30);
	}
	glTranslatef(-center.x, -center.y, -center.z);
	
	// Make sure changes appear onscreen
	glutSwapBuffers();
}

/**
*
**/
void relocate(int i){
	rain[i].x = (2.0*length*rand())/(RAND_MAX*1.0) - length;
	rain[i].y = length;
	rain[i].z = (2.0*length*rand())/(RAND_MAX*1.0) - length;
}

/**
*
**/
void idle(void){
	// Move rain drops
	#pragma omp parallel for
	for(int i=0; i<num_rain_drops; i++){
		rain[i].x += rain_velocity.x * dt;
		rain[i].y += rain_velocity.y * dt;
		rain[i].z += rain_velocity.z * dt;
		
		// Domain collision
		if(rain[i].x > length){
			rain[i].x -= 2.0*length;
		}
		if(rain[i].x < -length){
			rain[i].x += 2.0*length;
		}
		if(rain[i].y > length){
			rain[i].y -= 2.0*length;
		}
		if(rain[i].y < -length){
			relocate(i);
			#pragma omp critical
			{
			ground_drop_hit++;
			}
		}
		if(rain[i].z > length){
			rain[i].z -= 2.0*length;
		}
		if(rain[i].z < -length){
			rain[i].z += 2.0*length;
		}
		
		// Sphere collision check
		double dx = rain[i].x - center.x;
		double dy = rain[i].y - center.y;
		double dz = rain[i].z - center.z;
		double dist = sqrt(dx*dx + dy*dy + dz*dz);
		
		if(dist < radius){
			relocate(i);
			
			#pragma omp critical
			{
			drop_hit++;
			}
		}
	}
	
	// Moving body
	center.x += velocity.x * dt;
	if(center.x > limit){running = 0;}
	center.y += velocity.y * dt;
	if(center.y > limit){running = 0;}
	center.z += velocity.z * dt;
	if(center.z > limit){running = 0;}
	
	// Update time
	time += dt;
	
	// Exit check
	if(!running){
		printf("Hit %lu\n", drop_hit);
		finish();
	}else{
		glutPostRedisplay();
	}
}

/**
*
**/
int main(int argc, char **args){
	// Variables
	double vx = 5.f;
	double vy = 0.f;
	double vz = 0.f;
	double rx = 0.0;
	double ry = -25.0;
	double rz = 0.0;
	camera_type = 1;
	wireframe = 0;
	num_rain_drops = 10000;
	dt = 0.001;
	for(int i=0; i<argc; i++){
    	if(strcmp("--camera", args[i]) == 0){
    		if(*args[i+1]-48 > 0 && *args[i+1]-48 < 5){	
				camera_type = *args[i+1]-48;
    		}
       	}else if(strcmp("--drops", args[i]) == 0){
       		num_rain_drops = strtol(args[i+1], NULL, 10);
       	}else if(strcmp("--wireframe", args[i]) == 0){
			wireframe = 1;
       	}else if(strcmp("--dt", args[i]) == 0){
			dt = strtod(args[i+1], NULL);
       	}else if(strcmp("--vx", args[i]) == 0){
			vx = strtod(args[i+1], NULL);
       	}else if(strcmp("--vy", args[i]) == 0){
			vy = strtod(args[i+1], NULL);
       	}else if(strcmp("--vz", args[i]) == 0){
			vz = strtod(args[i+1], NULL);
       	}else if(strcmp("--rx", args[i]) == 0){
			rx = strtod(args[i+1], NULL);
       	}else if(strcmp("--ry", args[i]) == 0){
			ry = strtod(args[i+1], NULL);
       	}else if(strcmp("--rz", args[i]) == 0){
			rz = strtod(args[i+1], NULL);
       	}else if(strcmp("--help", args[i]) == 0){
       		printf("\t\"--camera 1\" to specify Domain camera\n");
       		printf("\t\"--camera 2\" to specify sphere front camera\n");
       		printf("\t\"--camera 3\" to specify sphere back camera\n");
       		printf("\t\"--camera 4\" to specify sphere side camera\n");
       		printf("\t\"--drops NUM\" to specify number of rain drops\n");
       		printf("\t\"--wireframe\" to activate Wireframe mode\n");
       		printf("\t\"--dt DOUBLE\" to specify simulation delta\n");
       		printf("\t\"--vx DOUBLE\" to specify sphere speed in X direction\n");
       		printf("\t\"--vy DOUBLE\" to specify sphere speed in Y direction\n");
       		printf("\t\"--vz DOUBLE\" to specify sphere speed in Z direction\n");
       		printf("\t\"--rx DOUBLE\" to specify rain speed in X direction\n");
       		printf("\t\"--ry DOUBLE\" to specify rain speed in Y direction\n");
       		printf("\t\"--rz DOUBLE\" to specify rain speed in Z direction\n");
       		exit(EXIT_SUCCESS);
       	}
    }
	
	// Print
	printf("Starting rain simulator\n");
	
	// Init domain vectors
	d1 = (vector){-length, -length, -length};
	d2 = (vector){-length, -length, length};
	d3 = (vector){length, -length, length};
	d4 = (vector){length, -length, -length};
	d5 = (vector){-length, length, -length};
	d6 = (vector){-length, length, length};
	d7 = (vector){length, length, length};
	d8 = (vector){length, length, -length};
	
	// Domain
	drop_hit = 0;
	ground_drop_hit = 0;
	length = 10.0;
	radius = 0.5;
	limit = length+radius;
	running = 1;
	
	// Sphere
	velocity = (vector){vx, vy, vz};
	center = (vector){-(length+radius), 0.0, 0.0};
	
	// rain
	rain_velocity = (vector){rx, ry, rz};
	rain = malloc(num_rain_drops * sizeof(vector));
	int i;
	for(i=0; i<num_rain_drops; i++){
		rain[i].x = (2.0*length*rand())/(RAND_MAX*1.0) - length;
		rain[i].y = (2.0*length*rand())/(RAND_MAX*1.0) - length;
		rain[i].z = (2.0*length*rand())/(RAND_MAX*1.0) - length;
	}
	
	// Rain volume
	double vel = sqrt(rx*rx + ry*ry + rz*rz);
	double r = (vel*vel)/272500;
	volume = (4.0 * 3.14159265359 * r * r * r)/3.0;
		
	// GLUT Window Initialization
	glutInit(&argc, args);
	glutInitWindowSize(900, 600);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow("Rain");
	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutKeyboardFunc(keypress);
	glutIdleFunc(idle);
	glutMainLoop();
	
	return EXIT_SUCCESS;
}
