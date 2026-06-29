#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

// parametros pre-definidos:
#define WIDTH 640
#define HEIGHT 500
#define PADDLE_HEIGHT 80
#define PADDLE_WIDTH 35
#define PADDLE_SPEED 10
#define BALL_RADIUS 10
#define BALL_SPEED 3
#define CIRCLE_RADIUS 50

// Variaveis alocadas com escopo global:
float left_boundary = WIDTH;
float right_boundary = WIDTH * 2;
int score1 = 0, score2 = 0;
float paddle1_x = HEIGHT / 2 - PADDLE_HEIGHT / 2;
float paddle2_x = HEIGHT / 2 - PADDLE_HEIGHT / 2;
float ball_x = WIDTH / 2;
float ball_y = HEIGHT / 2;
float ball_dx = BALL_SPEED;
float ball_dy = BALL_SPEED;
bool Gamepaused = false;
float Ball_speed_pause = BALL_SPEED;
float Ball_speed_increment = 20;

enum GameState { MENU, PLAYING };
enum GameMode { PVP, PVE_EASY, PVE_MED, PVE_HARD };
enum GameRules { CLASSIC, DEAD_ZONE };

GameState currentState = MENU;
GameMode currentMode = PVP;
GameRules currentRules = CLASSIC;

int match_timer = 0;
int dead_zone_timer = 3600; // 60 seconds
float special_charge_1 = 0;
float special_charge_2 = 0;
bool p1_fireball_active = false;
int p2_shield_frames = 0;

float paddle1_height = PADDLE_HEIGHT;
float paddle2_height = PADDLE_HEIGHT;

// Controle de teclado
bool keys[256] = {false};
bool specialKeys[256] = {false};

// Efeitos Arcade
int shake_frames = 0;
int respawn_delay_frames = 0;
struct Particle {
    float x, y, dx, dy;
    int life;
    float r, g, b;
};
#define NUM_PARTICLES 60
Particle particles[NUM_PARTICLES];

void initExplosion(float x, float y) {
    for (int i = 0; i < NUM_PARTICLES; i++) {
        particles[i].x = x;
        particles[i].y = y;
        float angle = (rand() % 360) * 3.14159f / 180.0f;
        float speed = (rand() % 100) / 10.0f + 2.0f;
        particles[i].dx = cos(angle) * speed;
        particles[i].dy = sin(angle) * speed;
        particles[i].life = 20 + rand() % 20; // 20 to 40 frames
        particles[i].r = (rand() % 100) / 100.0f + 0.5f;
        particles[i].g = (rand() % 100) / 100.0f;
        particles[i].b = (rand() % 100) / 100.0f + 0.5f; // Purple/Pink neon bias
    }
}

// Cria a janela:
void init()
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glDisable(GL_DEPTH_TEST); // Ensure depth test doesn't hide 2D objects
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WIDTH, 0, HEIGHT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

#ifndef __EMSCRIPTEN__
// Funcao para desenhar texto usando bitmap (apenas nativo, emscripten usara HTML)
void drawText(const char *text, float x, float y) {
    glRasterPos2f(x, y);
    while (*text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *text);
        text++;
    }
}
#endif

// Funcao de Pause 
void pauseGame()
{
	if (Gamepaused == false)
	{
		Gamepaused = true;
		Ball_speed_pause = ball_dx;
		ball_dx = 0;
		ball_dy = 0;	 	
	}
}
// Funcao de Resume
void resumeGame()
{
	if (Gamepaused == true)
	{
		Gamepaused = false;
		ball_dx = Ball_speed_pause;
		ball_dy = Ball_speed_pause;
	}
}

// Funcoes de teclado
void keyboard(unsigned char key, int x, int y) 
{
    keys[key] = true;
    
    if (currentState == MENU) {
        if (key == '1') { currentMode = PVP; currentState = PLAYING; score1 = 0; score2 = 0; ball_x = WIDTH/2; ball_y = HEIGHT/2; }
        else if (key == '2') { currentMode = PVE_EASY; currentState = PLAYING; score1 = 0; score2 = 0; ball_x = WIDTH/2; ball_y = HEIGHT/2; }
        else if (key == '3') { currentMode = PVE_MED; currentState = PLAYING; score1 = 0; score2 = 0; ball_x = WIDTH/2; ball_y = HEIGHT/2; }
        else if (key == '4') { currentMode = PVE_HARD; currentState = PLAYING; score1 = 0; score2 = 0; ball_x = WIDTH/2; ball_y = HEIGHT/2; }
        return;
    }
    
	switch(key)
	{
	case 13: // 13 eh o ENTER
		if(Gamepaused == false)
		{
			pauseGame();
		}	
		else 
		{
			resumeGame();
		}
		break;
	case 'm':
	case 'M':
	    currentState = MENU; // Voltar ao menu
	    break;
	case 27: // ESC
#ifndef __EMSCRIPTEN__
		exit(0);
#endif
		break;
	}
}

void keyboardUp(unsigned char key, int x, int y) {
    keys[key] = false;
}

void special(int key, int x, int y) {
    if(key >= 0 && key < 256) specialKeys[key] = true;
}

void specialUp(int key, int x, int y) {
    if(key >= 0 && key < 256) specialKeys[key] = false;
}

// Funcao de Desenho
void display()
{
    // Atualizar UI no Javascript independentemente do estado do jogo
#ifdef __EMSCRIPTEN__
    EM_ASM({
        if(window.updateUI) window.updateUI($0, $1, $2, $3, $4, $5, $6, $7, $8, $9, $10);
    }, score1, score2, (currentState == MENU) ? 1 : 0, Gamepaused, ball_x, ball_y, shake_frames > 0 ? 0 : 1, ball_dx, ball_dy, (int)special_charge_1, (int)special_charge_2);
#endif

	// Desenhando a mesa
    glClear(GL_COLOR_BUFFER_BIT);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    if (shake_frames > 0) {
        glTranslatef((rand() % 10) - 5, (rand() % 10) - 5, 0.0f);
    }
	
	if (currentState == MENU) {
#ifndef __EMSCRIPTEN__
        glColor3f(1.0, 1.0, 1.0);
        drawText("PONG", WIDTH / 2 - 25, HEIGHT / 2 + 100);
        drawText("1. Player vs Player", WIDTH / 2 - 80, HEIGHT / 2 + 20);
        drawText("2. Player vs AI (Easy)", WIDTH / 2 - 80, HEIGHT / 2 - 10);
        drawText("3. Player vs AI (Medium)", WIDTH / 2 - 80, HEIGHT / 2 - 40);
        drawText("4. Player vs AI (Hard)", WIDTH / 2 - 80, HEIGHT / 2 - 70);
#endif
        glutSwapBuffers();
        return;
    }
	
    // Draw Dead Zones if active
    if (currentRules == DEAD_ZONE && dead_zone_timer <= 0) {
        glColor3f(0.5, 0.0, 0.0);
        glBegin(GL_TRIANGLES);
        // Left dead zone
        glVertex2f(0, 0); glVertex2f(80, 0); glVertex2f(80, HEIGHT);
        glVertex2f(0, 0); glVertex2f(80, HEIGHT); glVertex2f(0, HEIGHT);
        // Right dead zone
        glVertex2f(WIDTH - 80, 0); glVertex2f(WIDTH, 0); glVertex2f(WIDTH, HEIGHT);
        glVertex2f(WIDTH - 80, 0); glVertex2f(WIDTH, HEIGHT); glVertex2f(WIDTH - 80, HEIGHT);
        glEnd();
    }

	// Linha central vertical tracejada (estilo Pong clássico)
	glLineWidth(2.0);
    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_LINES);
    for(int i = 0; i < HEIGHT; i += 20) {
        glVertex2f(WIDTH / 2.0, i);
        glVertex2f(WIDTH / 2.0, i + 10);
    }
    glEnd();
    
	// Bordas superior e inferior brancas
    glLineWidth(4.0);
    glBegin(GL_LINES);
	// Linha topo
    glVertex2f(0.0, HEIGHT - 2.0);
    glVertex2f(WIDTH, HEIGHT - 2.0);
    
    // Linha debaixo
    glVertex2f(0.0, 2.0);
    glVertex2f(WIDTH, 2.0);
    glEnd();
		
 	// Desenha as paletas usando GL_TRIANGLES para compatibilidade WebGL
 	glColor3f(1.0, 1.0, 1.0);
 	
 	// Player 01:
 	glBegin(GL_TRIANGLES);
 	glVertex2f(20.0, paddle1_x);
 	glVertex2f(PADDLE_WIDTH, paddle1_x);
 	glVertex2f(PADDLE_WIDTH, paddle1_x + paddle1_height);
 	
 	glVertex2f(20.0, paddle1_x);
 	glVertex2f(PADDLE_WIDTH, paddle1_x + paddle1_height);
 	glVertex2f(20.0, paddle1_x + paddle1_height);
 	glEnd();
    
 	// Player 02:
 	glBegin(GL_TRIANGLES);
 	glVertex2f(WIDTH - PADDLE_WIDTH, paddle2_x);
 	glVertex2f(WIDTH - 20.0, paddle2_x);
 	glVertex2f(WIDTH - 20.0, paddle2_x + paddle2_height);
 	
 	glVertex2f(WIDTH - PADDLE_WIDTH, paddle2_x);
 	glVertex2f(WIDTH - 20.0, paddle2_x + paddle2_height);
 	glVertex2f(WIDTH - PADDLE_WIDTH, paddle2_x + paddle2_height);
 	glEnd();

    // Particulas
    glPointSize(3.0f);
    glBegin(GL_POINTS);
    for (int i = 0; i < NUM_PARTICLES; i++) {
        if (particles[i].life > 0) {
            glColor3f(particles[i].r, particles[i].g, particles[i].b);
            glVertex2f(particles[i].x, particles[i].y);
        }
    }
    glEnd();

#ifndef __EMSCRIPTEN__
    if (shake_frames == 0) {
     	// Desenha a bola usando GL_TRIANGLES
     	glColor3f(1.0, 1.0, 1.0);
     	glBegin(GL_TRIANGLES);
     	glVertex2f(ball_x - BALL_RADIUS, ball_y - BALL_RADIUS);
     	glVertex2f(ball_x + BALL_RADIUS, ball_y - BALL_RADIUS);
     	glVertex2f(ball_x + BALL_RADIUS, ball_y + BALL_RADIUS);
     	
     	glVertex2f(ball_x - BALL_RADIUS, ball_y - BALL_RADIUS);
     	glVertex2f(ball_x + BALL_RADIUS, ball_y + BALL_RADIUS);
     	glVertex2f(ball_x - BALL_RADIUS, ball_y + BALL_RADIUS);
     	glEnd();
 	}
#endif

    glutSwapBuffers();
}

void reshape(int w, int h) 
{
    // w = WIDTH; h = HEIGHT;
    // Define a area da janela de renderizacao
    glViewport(0, 0, WIDTH, HEIGHT);
}

void update_physics()
{
	if (Gamepaused == false)
	{	
	    if (currentState == PLAYING) {
	        match_timer++;
	        
	        // Check if Point limit reached (Classic)
	        if (score1 >= 10 || score2 >= 10) {
	            currentState = MENU;
	        }
	        
	        if (currentRules == DEAD_ZONE) {
	            if (dead_zone_timer > 0) dead_zone_timer--;
	            else {
	                left_boundary = 80;
	                right_boundary = WIDTH - 80;
	            }
	        } else {
	            left_boundary = 0;
	            right_boundary = WIDTH;
	        }
	        
	        // Process Abilities
	        if (keys[' '] && special_charge_1 >= 100) {
	            special_charge_1 = 0;
	            p1_fireball_active = true;
	        }
	        if (keys['0'] && special_charge_2 >= 100) {
	            special_charge_2 = 0;
	            p2_shield_frames = 300; // 5 seconds
	        }
	        
	        paddle2_height = (p2_shield_frames > 0) ? PADDLE_HEIGHT * 2 : PADDLE_HEIGHT;
	        if (p2_shield_frames > 0) p2_shield_frames--;
	    }
	    
    	// Mover as paletas (sempre ativas, mesmo durante delays)
    	// Player 01
    	if ((keys['s'] || keys['S']) && paddle1_x > 0) {
    	    paddle1_x -= PADDLE_SPEED;
    	}
    	if ((keys['w'] || keys['W']) && paddle1_x < HEIGHT - paddle1_height) {
    	    paddle1_x += PADDLE_SPEED;
    	}
    
    	// Player 02
    	if (currentMode == PVP) {
        	if (specialKeys[GLUT_KEY_DOWN] && paddle2_x > 0) {
        	    paddle2_x -= PADDLE_SPEED;
        	}
        	if (specialKeys[GLUT_KEY_UP] && paddle2_x < HEIGHT - paddle2_height) {
        	    paddle2_x += PADDLE_SPEED;
        	}
    	} else {
    	    // AI Logic
    	    float ai_speed = PADDLE_SPEED;
    	    float react_x = 0;
    	    if (currentMode == PVE_EASY) { ai_speed = PADDLE_SPEED * 0.4f; react_x = WIDTH / 2.0f; }
    	    else if (currentMode == PVE_MED) { ai_speed = PADDLE_SPEED * 0.7f; react_x = WIDTH / 4.0f; }
    	    else if (currentMode == PVE_HARD) { ai_speed = PADDLE_SPEED * 1.1f; react_x = 0.0f; }
    	    
    	    if (ball_x > react_x) {
        	    float paddle2_center = paddle2_x + paddle2_height / 2;
        	    if (ball_y > paddle2_center + 10 && paddle2_x < HEIGHT - paddle2_height) {
        	        paddle2_x += ai_speed;
        	    } else if (ball_y < paddle2_center - 10 && paddle2_x > 0) {
        	        paddle2_x -= ai_speed;
        	    }
    	    }
    	}

	    if (shake_frames > 0) {
	        shake_frames--;
            for (int i = 0; i < NUM_PARTICLES; i++) {
                if (particles[i].life > 0) {
                    particles[i].x += particles[i].dx;
                    particles[i].y += particles[i].dy;
                    particles[i].life--;
                }
            }
            if (shake_frames == 0) {
                respawn_delay_frames = 60; // 1 second delay at 60fps
            }
            return; // Pause ball physics while exploding
	    }
	    
	    if (respawn_delay_frames > 0) {
	        respawn_delay_frames--;
	        return; // Pause ball physics while giving players time to prepare
	    }
	
    	// Mover a bola
    	ball_x += ball_dx;
    	ball_y += ball_dy;

    	// Verificar colisoes com as paredes
		if (ball_x + BALL_RADIUS >= WIDTH || ball_x - BALL_RADIUS <= 0)
		{
    	   	ball_dx = -ball_dx;
#ifdef __EMSCRIPTEN__
            EM_ASM({ if(window.playBeep) window.playBeep(400, 100); });
#endif
		}
		if (ball_y + BALL_RADIUS >= HEIGHT - 60 || ball_y - BALL_RADIUS <= 0)
		{
    	   	ball_dy = -ball_dy;
#ifdef __EMSCRIPTEN__
            EM_ASM({ if(window.playBeep) window.playBeep(400, 100); });
#endif
		}
	
		// Marcador de pontuacao:
		if (ball_x + BALL_RADIUS >= right_boundary || ball_x - BALL_RADIUS <= left_boundary)
		{
		    if (ball_x > WIDTH/2) score1++;
		    else score2++;
		    
            initExplosion(ball_x, ball_y);
            shake_frames = 30;
#ifdef __EMSCRIPTEN__
            EM_ASM({ if(window.playExplosion) window.playExplosion(); });
#endif
    		// reset para a bola voltar ao centro
    		ball_x = WIDTH/2;
    		ball_y = HEIGHT/2;
    		ball_dx = BALL_SPEED * (ball_dx > 0 ? -1 : 1);
			ball_dy = BALL_SPEED * (ball_dy > 0 ? 1 : -1);
			p1_fireball_active = false;
		}

		// Verifica colisao da bola com as paletas
		if ((ball_x >= 20 && ball_x <= PADDLE_WIDTH + 10) && (ball_y + BALL_RADIUS >= paddle1_x && ball_y - BALL_RADIUS <= paddle1_x + paddle1_height))
		{
			ball_dx = -ball_dx;
			ball_dx += (ball_dx > 0 ? 0.5f : -0.5f); // Pequeno incremento de velocidade
			ball_dy += (ball_dy > 0 ? 0.5f : -0.5f);
			
			if (p1_fireball_active) {
			    ball_dx *= 1.5f;
			    ball_dy *= 1.5f;
			    p1_fireball_active = false;
			}
			
			special_charge_1 += 20;
			if (special_charge_1 > 100) special_charge_1 = 100;
			
#ifdef __EMSCRIPTEN__
            EM_ASM({ if(window.playBeep) window.playBeep(600, 100); });
#endif
		}
		else if ((ball_x >= WIDTH - PADDLE_WIDTH - 10 && ball_x <= WIDTH - 20) && (ball_y + BALL_RADIUS >= paddle2_x && ball_y - BALL_RADIUS <= paddle2_x + paddle2_height))
		{
		   	ball_dx = -ball_dx;
		   	
		   	special_charge_2 += 20;
		   	if (special_charge_2 > 100) special_charge_2 = 100;
#ifdef __EMSCRIPTEN__
            EM_ASM({ if(window.playBeep) window.playBeep(600, 100); });
#endif
		}
	}
}

#ifdef __EMSCRIPTEN__
extern "C" {
    EMSCRIPTEN_KEEPALIVE
    void start_game(int rules, int opponent) {
        if (rules == 0) currentRules = CLASSIC;
        else if (rules == 1) currentRules = DEAD_ZONE;
        
        if (opponent == 1) currentMode = PVP;
        else if (opponent == 2) currentMode = PVE_EASY;
        else if (opponent == 3) currentMode = PVE_MED;
        else if (opponent == 4) currentMode = PVE_HARD;
        
        currentState = PLAYING;
        Gamepaused = false;
        score1 = 0; score2 = 0; 
        match_timer = 0;
        dead_zone_timer = 3600;
        special_charge_1 = 0;
        special_charge_2 = 0;
        p1_fireball_active = false;
        p2_shield_frames = 0;
        
        ball_x = WIDTH/2; ball_y = HEIGHT/2;
    }
    
    EMSCRIPTEN_KEEPALIVE
    void trigger_action(int action) {
        if (action == 0) { // Resume
            Gamepaused = false;
        } else if (action == 1) { // Restart
            start_game((int)currentRules, (int)currentMode);
        } else if (action == 2) { // Quit to Menu
            currentState = MENU;
        }
    }
}

void emscripten_loop() {
    update_physics();
    display();
    static int frameCount = 0;
    if (frameCount++ % 60 == 0) {
        printf("Frame %d: ball(%.1f, %.1f), paddle1(%.1f), paddle2(%.1f)\n", frameCount, ball_x, ball_y, paddle1_x, paddle2_x);
    }
}
#endif

void update(int value)
{
    update_physics();
    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Pong Game - WebAssembly");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(special);
    glutSpecialUpFunc(specialUp);
    
    init();

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(emscripten_loop, 0, 1);
#else
    glutTimerFunc(0, update, 0);
    glutMainLoop();
#endif

    return 0;
}
