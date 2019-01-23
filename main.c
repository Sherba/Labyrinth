#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>
#include <stdio.h>
#include <time.h>

#define TIMER0 (1)
#define TIMER0_INTERVAL (16)

#define MAX_LAB_W 50
#define MAX_LAB_H 50

#define CUBE_SIZE 7
#define EPS 0.00
#define PI 3.14159265359
#define DPH 3.14159265359 / 90
#define DF 0.25

/* height and width of screen (used for clairvoyance animation) */
int height;
int width;

/* parameters used for animations */
float animation_param = 0;
float initial_rise_param = 0;
float clairvoyance_param = 0;
int toggle_clairvoyance = 0;
float fall_param = 1;
float rise_param = 0;

/* light parameters */
float light_ambient[] = {.3, .3, .3, 1};
float light_diffuse[] = {.7, .1, .1, 1};
float light_specular[] = {1, 1, 0, 1};

/* material parameters */
float material_ambient[] = {.3, .3, .3, 1};
float material_diffuse[] = {1, 1, 0, 1};
float material_specular[] = {.6, .6, 0, 1};
float shininess = 20;

/* camera (and light) position */
float eye_x = 0;
float eye_y = CUBE_SIZE;
float eye_z = 0;

/* point to which camera is facing */
float to_x = 0;
float to_y = CUBE_SIZE;
float to_z = 0;

/* angle and movement speed parameters */
float phi = 0;
float delta_phi = 0;
float delta_forward = 0;
float run = 1;

/* field where player is currently at */
int field_x = 0;
int field_z = 0;
int last_field_x = 0;
int last_field_z = 0;

/* field where end of the labyrinth is */
int finish_field_x = 0;
int finish_field_z = 0;

/* wall which is supposed to be moving */
int wall_x = 0;
int wall_z = 0;

/* field position parameters */
int N = 0;
int S = 0;
int W = 0;
int E = 0;

/* labyrinth height and width */
int lab_h = 0;
int lab_w = 0;

/* array in which labyrinth is represented */
char labyrinth[MAX_LAB_H][MAX_LAB_W];

/* declarations of callback functions */
void on_timer(int timer_id);
void on_display();
void on_reshape(int w, int h);
void on_keypress(unsigned char key, int x, int y);
void on_keyrelease(unsigned char key, int x, int y);

void print_labyrinth();
void draw_labyrinth();
void refresh_eye();
void set_init();
void clear_labyrinith();
void set_new_wall();
int solve_labyrinth(int x, int z);

int main(int argc, char** argv) {

	/* initialize variables */
	set_init();

	solve_labyrinth(field_x, field_z);
	print_labyrinth();

	glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB   | 
    					GLUT_DEPTH | 
    					GLUT_DOUBLE);
    glutInitWindowSize(600, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow(argv[0]);

    glClearColor(0, 0, 0, 1);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    //glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);

    /* setting lighting */
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

	/* setting materials */
	glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse);
	glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular);
	glMaterialf(GL_FRONT, GL_SHININESS, shininess);

    glutTimerFunc(TIMER0_INTERVAL, on_timer, TIMER0);
    glutDisplayFunc(on_display);
    glutReshapeFunc(on_reshape);
    glutKeyboardFunc(on_keypress);
    glutKeyboardUpFunc(on_keyrelease);

    glutMainLoop();

    return 0;
}

void on_display() {

	glClear(GL_COLOR_BUFFER_BIT |
			GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	refresh_eye();
	
	gluLookAt(eye_x, eye_y, eye_z,
			 to_x,  to_y,  to_z,
			 0,     1,     0);

	/* set light position to where camera is at */
	float light_position[] = {eye_x, eye_y, eye_z, 1};

	/* setting light position */
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	/* labyrinth is drawn after translating  
	 * so it starts in the (0, 0)
	 */
	glPushMatrix();
		glTranslatef(CUBE_SIZE / 2, 0, CUBE_SIZE / 2);
		draw_labyrinth();
	glPopMatrix();

	/* if you stand on the finish tile you won */
	if (finish_field_x == field_x &&
		finish_field_z == field_z) {

		printf("YOU WON\n");
		exit(EXIT_SUCCESS);
	}

	glutSwapBuffers();
}

void on_reshape(int w, int h) {

	width = w;
	height = h;

	glViewport(0, 0, w, h);

	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();
	gluPerspective(60, (float)(w)/h, 1, 200);

	glMatrixMode(GL_MODELVIEW);
}

void on_timer(int timer_id) {

	animation_param += 0.05;
	if (initial_rise_param <= 1) {
		initial_rise_param += 0.03;
	}

	if (0 == (int)animation_param % 10) {
		animation_param += 1;
		srand(time(NULL));
		set_new_wall();
	}

	if (TIMER0 == timer_id) {
		glutPostRedisplay();
		glutTimerFunc(TIMER0_INTERVAL, on_timer, TIMER0);
	}
}

void on_keypress(unsigned char key, int x, int y) {

	(void)x;
	(void)y;

	switch(key) {

		case 27:
			exit(0);
		case 'w':
		case 'W':
			delta_forward = DF;
			break;
		case 's':
		case 'S':
			delta_forward = -DF;
			break;
		case 'a':
		case 'A':
			delta_phi = -DPH;
			break;
		case 'd':
		case 'D':
			delta_phi = DPH;
			break;
		case 'k':
		case 'K':
			toggle_clairvoyance = 1;
			clear_labyrinith();
			solve_labyrinth(field_x, field_z);
			break;
		case 'm':
			printf("X: %lf\tZ :%lf\n", eye_x, eye_z);
			printf("fX: %d\tfZ :%d\n", field_x, field_z);
			printf("N:%d\tW:%d\tE:%d\tS:%d\n",N, W, E, S);
			break;
		case 'n':
			print_labyrinth();
			break;
		case 32:
			run = 2.5;
			break;
	}
}

void on_keyrelease(unsigned char key, int x, int y) {

	(void)x;
	(void)y;

	switch(key) {
		case 'w':
		case 'W':
			delta_forward = 0;
			break;
		case 'a':
		case 'A':
			delta_phi = 0;
			break;
		case 's':
		case 'S':
			delta_forward = 0;
			break;
		case 'd':
		case 'D':
			delta_phi = 0;
			break;
		case 'k':
		case 'K':
			toggle_clairvoyance = 0;
			break;
		case 32:
			run = 1;
			break;
	}
}

/* function draws labyrinth as it is read from file
 * # - cube is up
 * _ - cube is down
 * @ - start
 * $ - finish
 */
void draw_labyrinth() {

	glPushMatrix();

		glTranslatef(-CUBE_SIZE / 2, 0, -CUBE_SIZE / 2);
		glBegin(GL_POLYGON);
			glVertex3f(0                , 1.5 * CUBE_SIZE, 0                );
			glVertex3f(0                , 1.5 * CUBE_SIZE, lab_h * CUBE_SIZE);
			glVertex3f(lab_w * CUBE_SIZE, 1.5 * CUBE_SIZE, lab_h * CUBE_SIZE);
			glVertex3f(lab_w * CUBE_SIZE, 1.5 * CUBE_SIZE, 0                );
		glEnd();
		glTranslatef(CUBE_SIZE / 2, 0, CUBE_SIZE / 2);

		for (int j = 0; j <= lab_h - 1; j++) {
			for (int i = 0; i <= lab_w - 1; i++) {

				int curr_y = 0;
				if (i == wall_x && j == wall_z) {

					/* if chosen wall is up it should move down (with fall parameter)
					 * when it falls to the 0 it sets the tile to '_' and changes wall_x and wall_z
					 * to -1 so no other tiles should be moved untill next time.
					 * this works similarly for '_' -> "#"
					 */
					if ('#' == labyrinth[i][j]) {

						curr_y = fall_param * CUBE_SIZE - EPS;
						fall_param -= 0.05;
						if (fall_param <= 0) {

							labyrinth[i][j] = '_';
							fall_param = 1;
							wall_x = -1;
							wall_z = -1;

							// clear_labyrinith();
							// solve_labyrinth(field_x, field_z);
						}
					} else if ('_' == labyrinth[i][j]) {

						if (i == field_x && j == field_z) {
							printf("YOU DIED\n");
							exit(EXIT_SUCCESS);
						}
						curr_y = rise_param * CUBE_SIZE - EPS;
						rise_param += 0.05;
						if (rise_param >= 1) {
							labyrinth[i][j] = '#';
							rise_param = 0;
							wall_x = -1;
							wall_z = -1;

							// clear_labyrinith();
							// solve_labyrinth(field_x, field_z);
						}
					} else {
						//ignore
					}

				} else {
					curr_y = (labyrinth[i][j] == '#' ? initial_rise_param * CUBE_SIZE - EPS : 0);
				}
				

				glPushMatrix();
					glTranslatef(i * (CUBE_SIZE+EPS), curr_y, j * (CUBE_SIZE+EPS));
					glutSolidCube(CUBE_SIZE);
				glPopMatrix();
				
			}
		}

		/* Draws square to show the path to exit */
		if (!toggle_clairvoyance) return;
		glDisable(GL_LIGHTING);
		glBegin(GL_QUADS);
		for (int j = 0; j <= lab_h - 1; j++) {
			for (int i = 0; i <= lab_w - 1; i++) {

				if ('.' == labyrinth[i][j]) {

					glColor3f(0, 1, 1);

					glVertex3f(i * (CUBE_SIZE + EPS) - 1, 4 + 0.1 * sin(3 * animation_param), j * (CUBE_SIZE + EPS) - 1);
					glVertex3f(i * (CUBE_SIZE + EPS) + 1, 4 + 0.1 * sin(3 * animation_param), j * (CUBE_SIZE + EPS) - 1);
					glVertex3f(i * (CUBE_SIZE + EPS) + 1, 4 + 0.1 * sin(3 * animation_param), j * (CUBE_SIZE + EPS) + 1);
					glVertex3f(i * (CUBE_SIZE + EPS) - 1, 4 + 0.1 * sin(3 * animation_param), j * (CUBE_SIZE + EPS) + 1);
				}
			}
		}
		glEnd();
		glEnable(GL_LIGHTING);

	glPopMatrix();
}

/* function is called if player is supposed to be moving
 * lock_x and lock_z flags are supposed to lock movement in
 * case of collision. So if upper wall is risen, player is moving
 * toward it, and is in N section of field, lock_z will be set to 0
 * and player wont be able to move thru the wall
 */
void refresh_eye() {

	/* toggles clairvoyance */
	if (toggle_clairvoyance) {

		glMatrixMode(GL_PROJECTION);

		glLoadIdentity();
		gluPerspective(60 + 10 * sin(clairvoyance_param), (float)(width)/height, 1, 200);

		glMatrixMode(GL_MODELVIEW);

		clairvoyance_param += 0.05;
	} else {

		clairvoyance_param = 0;
		glMatrixMode(GL_PROJECTION);

		glLoadIdentity();
		gluPerspective(60, (float)(width)/height, 1, 200);

		glMatrixMode(GL_MODELVIEW);
	}

	int lock_x = 1;
	int lock_z = 1;
	phi += delta_phi;

	if (labyrinth[field_x][field_z - 1] == '#' && N == 1 && delta_forward * sin(phi) < 0) {
		lock_z = 0;
	}

	if (labyrinth[field_x][field_z + 1] == '#' && S == 1 && delta_forward * sin(phi) > 0) {
		lock_z = 0;
	}

	if (labyrinth[field_x - 1][field_z] == '#' && W == 1 && delta_forward * cos(phi) < 0) {
		lock_x = 0;
	}

	if (labyrinth[field_x + 1][field_z] == '#' && E == 1 && delta_forward * cos(phi) > 0) {
		lock_x = 0;
	}

	/* called when player fallse of the labyrinth */
	if (field_x < 0 	 ||
		field_x >= lab_w ||
		field_z < 0 	 ||
		field_z >= lab_h ) {

		lock_x = 0;
		lock_z = 0;
		eye_y -= 0.6;
		to_y -= 0.6;
	}

	/* if player falls he dies */
	if (eye_y < -50) {
		printf("YOU DIED\n");
		exit(EXIT_SUCCESS);
	}


	eye_x += lock_x * run * delta_forward * cos(phi);
	eye_z += lock_z * run * delta_forward * sin(phi);

	to_x = eye_x + cos(phi);
	to_z = eye_z + sin(phi);

	/* update field where player is positioned 
	 * and update clairvoyance path
	 */
	if (field_x != eye_x / CUBE_SIZE &&
		field_z != eye_z / CUBE_SIZE) {

		last_field_x = field_x;
		last_field_z = field_z;

		field_x = eye_x / CUBE_SIZE;
		field_z = eye_z / CUBE_SIZE;

		if ('.' == labyrinth[last_field_x][last_field_z] &&
			'.' == labyrinth[field_x][field_z]			) {

			labyrinth[last_field_x][last_field_z] = '_';

		} else if ('.' == labyrinth[last_field_x][last_field_z] &&
	               '_' == labyrinth[field_x][field_z]			) {

			labyrinth[field_x][field_z] = '.';			
		} 
	}

	/* update where in the field is player currently at */
	N = W = S = E = 0;
	if ((int)eye_x % CUBE_SIZE <= CUBE_SIZE * 0.2) W = 1;
	if ((int)eye_x % CUBE_SIZE >= CUBE_SIZE * 0.8) E = 1;
	if ((int)eye_z % CUBE_SIZE <= CUBE_SIZE * 0.2) N = 1;
	if ((int)eye_z % CUBE_SIZE >= CUBE_SIZE * 0.8) S = 1;
}

/* function initializes starting variables 
 * (starting/ending position, initial angle etc.)
 */
void set_init() {

	/* reads labyrinth from file */
	FILE* data;
	data = fopen("lavirint.txt", "r");
	if (NULL == data) {
		fprintf(stderr, "File opening error\n");
		exit(EXIT_FAILURE);
	}

	fscanf(data, "%d %d\n", &lab_h, &lab_w);
	for (int j = 0; j <= lab_h - 1; j++) {
		for (int i = 0; i <= lab_w - 1; i++) {
			labyrinth[i][j] = fgetc(data);
		}
		fgetc(data);
	}

	fclose(data);

	int start_x = 0;
	int start_z = 0;
	int end_x = 0;
	int end_z = 0;

	for (int j = 0; j <= lab_h - 1; j++) {
		for (int i = 0; i <= lab_w - 1; i++) {

			if ('@' == labyrinth[i][j]) {
				start_x = i;
				start_z = j;
			}

			if ('$' == labyrinth[i][j]) {
				end_x = i;
				end_z = j;
			}

		}
	}

	finish_field_x = end_x;
	finish_field_z = end_z;

	eye_x = (start_x + 0.5) * CUBE_SIZE;
	eye_z = (start_z + 0.5) * CUBE_SIZE;

	field_x = eye_x / CUBE_SIZE;
	field_z = eye_z / CUBE_SIZE;
	
	labyrinth[field_x][field_z] = '_';

	if (0 == start_z) {
		phi = PI / 2;
	} 
	if (lab_h - 1 == start_z) {
		phi = -PI / 2;
	}

	if (0 == start_x) {
		phi = 0;
	}
	if (lab_w - 1 == start_x) {
		phi = PI;
	}

	to_x = eye_x + cos(phi);
	to_z = eye_z + sin(phi);

	printf("%d %d\n", start_x, start_z);
}

/* set labyrinth fields to '.'
 * to show path to exit 
 */
int solve_labyrinth(int x, int z) {

	if ('$' == labyrinth[x][z])	return 1;
	if ('_' != labyrinth[x][z]) return 0;

	labyrinth[x][z] = '.';

	if (solve_labyrinth(x + 1, z))	return 1;
	if (solve_labyrinth(x, z + 1))	return 1; 
	if (solve_labyrinth(x - 1, z))	return 1;
	if (solve_labyrinth(x, z - 1))	return 1;

	labyrinth[x][z] = '_';
	return 0;
}

/* prints labyrinth */
void print_labyrinth() {

	for (int j = 0; j <= lab_h - 1; j++) {
		for (int i = 0; i <= lab_w - 1; i++) {
			fprintf(stderr, "%c", labyrinth[i][j]);
		}
		fprintf(stderr, "\n");
	}
}

/* used when clairvoyance is called again to clear the labyrinth first */
void clear_labyrinith() {

	for (int j = 0; j <= lab_h - 1; j++) {
		for (int i = 0; i <= lab_w - 1; i++) {
			
			if ('.' == labyrinth[i][j])	labyrinth[i][j] = '_';
		}
	}
}

/* chooses the wall which is going to move next */
void set_new_wall() {
	
	wall_x = (int)((lab_w - 2) * (rand() / (float)RAND_MAX) + 1);
	wall_z = (int)((lab_h - 2) * (rand() / (float)RAND_MAX) + 1);
}
