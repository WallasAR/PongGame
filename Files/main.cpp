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
#define WIDTH 960
#define HEIGHT 500
#define PADDLE_HEIGHT 80
#define PADDLE_WIDTH 15
#define PADDLE_SPEED 10
#define BALL_RADIUS 10
#define BALL_SPEED 3
#define CIRCLE_RADIUS 50

// Variaveis alocadas com escopo global:
float left_boundary = WIDTH;
float right_boundary = WIDTH * 2;
int score1 = 0, score2 = 0;
float paddle1_y = HEIGHT / 2 - PADDLE_HEIGHT / 2;
float paddle2_y = HEIGHT / 2 - PADDLE_HEIGHT / 2;
float paddle1_x = 20.0f;
float paddle2_x = WIDTH - 20.0f - PADDLE_WIDTH;
float ball_x = WIDTH / 2;
float ball_y = HEIGHT / 2;
float ball_dx = BALL_SPEED;
float ball_dy = BALL_SPEED;
bool Gamepaused = false;
float Ball_speed_x_pause = BALL_SPEED;
float Ball_speed_y_pause = BALL_SPEED;
float Ball_speed_increment = 20;

enum GameState { MENU, PLAYING };
enum GameMode { PVP, PVE_EASY, PVE_MED, PVE_HARD };
enum GameRules { CLASSIC, DEAD_ZONE };

GameState currentState = MENU;
GameMode currentMode = PVP;
GameRules currentRules = CLASSIC;

int match_timer = 0;
int dead_zone_timer = 7200; // 120 seconds
int dead_zone_width = 0;
float special_charge_1 = 0;
float special_charge_2 = 0;
// Skill System
enum SkillType {
    SKILL_NONE = 0, SKILL_SMASH = 1, SKILL_SHIELD = 2, SKILL_GHOST = 3,
    SKILL_BLINK = 4, SKILL_FREEZE = 5, SKILL_EMP = 6
};
int p1_loadout[3] = {SKILL_SMASH, SKILL_SHIELD, SKILL_GHOST}; // Forward, Backward, Neutral
int p2_loadout[3] = {SKILL_SMASH, SKILL_SHIELD, SKILL_GHOST};

// Skill States
bool p1_fireball_active = false;
bool p2_fireball_active = false;
int p1_shield_frames = 0;
int p2_shield_frames = 0;
int ghost_frames = 0;
int p1_freeze_frames = 0;
int p2_freeze_frames = 0;
int p1_emp_frames = 0;
int p2_emp_frames = 0;

float paddle1_height = PADDLE_HEIGHT;
float paddle2_height = PADDLE_HEIGHT;

// Controle de teclado
bool keys[256] = {false};
bool specialKeys[256] = {false};

// Configurable Keys
int key_p1_up = 'w'; bool p1_up_spec = false;
int key_p1_down = 's'; bool p1_down_spec = false;
int key_p1_forward = 'd'; bool p1_forward_spec = false;
int key_p1_backward = 'a'; bool p1_backward_spec = false;
int key_p1_spec = ' '; bool p1_spec_spec = false;

int key_p2_up = GLUT_KEY_UP; bool p2_up_spec = true;
int key_p2_down = GLUT_KEY_DOWN; bool p2_down_spec = true;
int key_p2_forward = GLUT_KEY_LEFT; bool p2_forward_spec = true;
int key_p2_backward = GLUT_KEY_RIGHT; bool p2_backward_spec = true;
int key_p2_spec = '0'; bool p2_spec_spec = false;

int key_pause = 13; bool pause_spec = false;

extern "C" {
    EMSCRIPTEN_KEEPALIVE void set_key_bind(int action_id, int new_key, bool is_special) {
        if (action_id == 0) { key_p1_up = new_key; p1_up_spec = is_special; }
        else if (action_id == 1) { key_p1_down = new_key; p1_down_spec = is_special; }
        else if (action_id == 2) { key_p1_spec = new_key; p1_spec_spec = is_special; }
        else if (action_id == 3) { key_p2_up = new_key; p2_up_spec = is_special; }
        else if (action_id == 4) { key_p2_down = new_key; p2_down_spec = is_special; }
        else if (action_id == 5) { key_p2_spec = new_key; p2_spec_spec = is_special; }
        else if (action_id == 6) { key_pause = new_key; pause_spec = is_special; }
        else if (action_id == 7) { key_p1_forward = new_key; p1_forward_spec = is_special; }
        else if (action_id == 8) { key_p1_backward = new_key; p1_backward_spec = is_special; }
        else if (action_id == 9) { key_p2_forward = new_key; p2_forward_spec = is_special; }
        else if (action_id == 10) { key_p2_backward = new_key; p2_backward_spec = is_special; }
    }
    
    EMSCRIPTEN_KEEPALIVE void set_loadout(int player, int forward, int backward, int neutral) {
        if (player == 1) {
            p1_loadout[0] = forward; p1_loadout[1] = backward; p1_loadout[2] = neutral;
        } else if (player == 2) {
            p2_loadout[0] = forward; p2_loadout[1] = backward; p2_loadout[2] = neutral;
        }
    }
}


// Efeitos Arcade
int shake_frames = 0;
int respawn_delay_frames = 0;
struct Particle {
    float x, y, dx, dy;
    int life;
    float r, g, b;
};
#define NUM_PARTICLES 200
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
        float gray = (rand() % 100) / 100.0f;
        particles[i].r = gray + 0.2f;
        particles[i].g = gray + 0.2f;
        particles[i].b = gray + 0.2f;
    }
}

void initHitExplosion(float x, float y, float dir_x) {
    int spawned = 0;
    for (int i = 0; i < NUM_PARTICLES; i++) {
        if (particles[i].life <= 0) {
            particles[i].x = x;
            particles[i].y = y;
            float angle = ((rand() % 120) - 60) * 3.14159f / 180.0f; // Cone angle
            if (dir_x < 0) angle += 3.14159f; 
            float speed = (rand() % 50) / 10.0f + 2.0f;
            particles[i].dx = cos(angle) * speed;
            particles[i].dy = sin(angle) * speed;
            particles[i].life = 10 + rand() % 10;
            particles[i].r = 1.0f; particles[i].g = 1.0f; particles[i].b = 1.0f;
            spawned++;
            if (spawned >= 30) break; // Only spawn 30 particles per hit
        }
    }
}

struct TrailPoint {
    float x, y;
    int life;
};
#define MAX_TRAILS 40
TrailPoint paddle1_trail[MAX_TRAILS];
TrailPoint paddle2_trail[MAX_TRAILS];
TrailPoint ball_trail[MAX_TRAILS];
int p1_trail_idx = 0;
int p2_trail_idx = 0;
int ball_trail_idx = 0;

void add_trail(TrailPoint trail[], int& idx, float x, float y) {
    trail[idx].x = x;
    trail[idx].y = y;
    trail[idx].life = 12;
    idx = (idx + 1) % MAX_TRAILS;
}

void initDoubleWallExplosion(float left_x, float right_x, float width) {
    for (int i = 0; i < NUM_PARTICLES; i++) {
        if (i % 2 == 0) {
            particles[i].x = left_x + (rand() % (int)width);
        } else {
            particles[i].x = right_x + (rand() % (int)width);
        }
        particles[i].y = (rand() % HEIGHT);
        
        float angle = (rand() % 360) * 3.14159f / 180.0f;
        float speed = (rand() % 100) / 10.0f + 3.0f;
        
        particles[i].dx = cos(angle) * speed;
        particles[i].dy = sin(angle) * speed;
        particles[i].life = 30 + rand() % 30; // longer life
        float gray = (rand() % 100) / 100.0f;
        particles[i].r = gray + 0.2f;
        particles[i].g = gray + 0.2f;
        particles[i].b = gray + 0.2f;
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
		Ball_speed_x_pause = ball_dx;
		Ball_speed_y_pause = ball_dy;
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
		if (Ball_speed_x_pause != 0) {
		    ball_dx = Ball_speed_x_pause;
		    ball_dy = Ball_speed_y_pause;
		}
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
    bool is_pause = pause_spec ? false : (key == key_pause || (key_pause >= 'a' && key_pause <= 'z' && key == key_pause - 32));
    if (is_pause) {
        if (Gamepaused == false) {
            pauseGame();
        } else {
            resumeGame();
        }
    }
	switch(key)
	{
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
        if(window.updateUI) window.updateUI($0, $1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12);
    }, score1, score2, (currentState == MENU) ? 1 : 0, Gamepaused, ball_x, ball_y, (shake_frames > 0 || ghost_frames > 0) ? 0 : 1, ball_dx, ball_dy, (int)special_charge_1, (int)special_charge_2, (currentRules == DEAD_ZONE) ? dead_zone_timer : -1, dead_zone_width);
#endif

	// Desenhando a mesa
    glClear(GL_COLOR_BUFFER_BIT);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    if (shake_frames > 0) {
        glTranslatef((rand() % 10) - 5, (rand() % 10) - 5, 0.0f);
    }
    
    // Draw ball if not ghosted
    if (ghost_frames == 0) {
        glPushMatrix();
        glTranslatef(ball_x, ball_y, 0);
        glBegin(GL_QUADS);
        glColor3f(1.0, 1.0, 1.0);
        glVertex2f(-BALL_SIZE, -BALL_SIZE);
        glVertex2f(BALL_SIZE, -BALL_SIZE);
        glVertex2f(BALL_SIZE, BALL_SIZE);
        glVertex2f(-BALL_SIZE, BALL_SIZE);
        glEnd();
        glPopMatrix();
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
	
    // Draw Warning Zones if about to shrink - Removed as per user request to replace with ASCII skulls

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
    glVertex2f((currentRules == DEAD_ZONE) ? dead_zone_width : 0.0, HEIGHT - 2.0);
    glVertex2f((currentRules == DEAD_ZONE) ? WIDTH - dead_zone_width : WIDTH, HEIGHT - 2.0);
    
    // Linha debaixo
    glVertex2f((currentRules == DEAD_ZONE) ? dead_zone_width : 0.0, 2.0);
    glVertex2f((currentRules == DEAD_ZONE) ? WIDTH - dead_zone_width : WIDTH, 2.0);
    glEnd();
		
 	// Desenha as paletas usando GL_TRIANGLES para compatibilidade WebGL
 	glColor3f(1.0, 1.0, 1.0);
 	
 	// Player 01:
 	glBegin(GL_TRIANGLES);
 	glVertex2f(paddle1_x, paddle1_y);
 	glVertex2f(paddle1_x + PADDLE_WIDTH, paddle1_y);
 	glVertex2f(paddle1_x + PADDLE_WIDTH, paddle1_y + paddle1_height);
 	
 	glVertex2f(paddle1_x, paddle1_y);
 	glVertex2f(paddle1_x + PADDLE_WIDTH, paddle1_y + paddle1_height);
 	glVertex2f(paddle1_x, paddle1_y + paddle1_height);
 	glEnd();
    
 	// Player 02:
 	glBegin(GL_TRIANGLES);
 	glVertex2f(paddle2_x, paddle2_y);
 	glVertex2f(paddle2_x + PADDLE_WIDTH, paddle2_y);
 	glVertex2f(paddle2_x + PADDLE_WIDTH, paddle2_y + paddle2_height);
 	
 	glVertex2f(paddle2_x, paddle2_y);
 	glVertex2f(paddle2_x + PADDLE_WIDTH, paddle2_y + paddle2_height);
        float dx = (rand() % 20 - 10) / 2.0f;
        float dy = (rand() % 20 - 10) / 2.0f;
        glTranslatef(dx, dy, 0.0f);
    }
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    // Draw Trails
    for (int i=0; i<MAX_TRAILS; i++) {
        if (paddle1_trail[i].life > 0) {
            float alpha = paddle1_trail[i].life / 12.0f * 0.4f;
            glColor4f(1.0f, 1.0f, 1.0f, alpha);
            glRectf(paddle1_trail[i].x, paddle1_trail[i].y, paddle1_trail[i].x + PADDLE_WIDTH, paddle1_trail[i].y + paddle1_height);
        }
        if (paddle2_trail[i].life > 0) {
            float alpha = paddle2_trail[i].life / 12.0f * 0.4f;
            glColor4f(1.0f, 1.0f, 1.0f, alpha);
            glRectf(paddle2_trail[i].x, paddle2_trail[i].y, paddle2_trail[i].x + PADDLE_WIDTH, paddle2_trail[i].y + paddle2_height);
        }
        if (ball_trail[i].life > 0 && ghost_frames <= 0) {
            float alpha = ball_trail[i].life / 12.0f * 0.6f;
            glColor4f(1.0f, 1.0f, 1.0f, alpha);
            glBegin(GL_QUADS);
            glVertex2f(ball_trail[i].x - BALL_RADIUS, ball_trail[i].y - BALL_RADIUS);
            glVertex2f(ball_trail[i].x + BALL_RADIUS, ball_trail[i].y - BALL_RADIUS);
            glVertex2f(ball_trail[i].x + BALL_RADIUS, ball_trail[i].y + BALL_RADIUS);
            glVertex2f(ball_trail[i].x - BALL_RADIUS, ball_trail[i].y + BALL_RADIUS);
            glEnd();
        }
    }
    
    // Draw Particles
    for (int i = 0; i < NUM_PARTICLES; i++) {
        if (particles[i].life > 0) {
            float alpha = particles[i].life / 40.0f;
            if (alpha > 1.0f) alpha = 1.0f;
            glColor4f(particles[i].r, particles[i].g, particles[i].b, alpha);
            glBegin(GL_QUADS);
            glVertex2f(particles[i].x - 2, particles[i].y - 2);
            glVertex2f(particles[i].x + 2, particles[i].y - 2);
            glVertex2f(particles[i].x + 2, particles[i].y + 2);
            glVertex2f(particles[i].x - 2, particles[i].y + 2);
            glEnd();
        }
    }
    
    glDisable(GL_BLEND);

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

void trigger_skill(int player, int skill_type) {
    if (player == 1) {
        if (skill_type == SKILL_SMASH) p1_fireball_active = true;
        else if (skill_type == SKILL_SHIELD) p1_shield_frames = 300;
        else if (skill_type == SKILL_GHOST) ghost_frames = 72; // 1.2s
        else if (skill_type == SKILL_BLINK) { ball_x += 200; if (ball_x > WIDTH) ball_x = WIDTH; }
        else if (skill_type == SKILL_FREEZE) p2_freeze_frames = 240; // 4s
        else if (skill_type == SKILL_EMP) p2_emp_frames = 240; // 4s
    } else {
        if (skill_type == SKILL_SMASH) p2_fireball_active = true;
        else if (skill_type == SKILL_SHIELD) p2_shield_frames = 300;
        else if (skill_type == SKILL_GHOST) ghost_frames = 72;
        else if (skill_type == SKILL_BLINK) { ball_x -= 200; if (ball_x < 0) ball_x = 0; }
        else if (skill_type == SKILL_FREEZE) p1_freeze_frames = 240;
        else if (skill_type == SKILL_EMP) p1_emp_frames = 240;
    }
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
	        
	        left_boundary = 0;
	        right_boundary = WIDTH;
	        
	        if (currentRules == DEAD_ZONE) {
	            if (dead_zone_timer > 0) {
	                dead_zone_timer--;
	                if (dead_zone_timer > 0 && dead_zone_timer % 1200 == 0) {
	                    // Just shrunk! Trigger explosion in both zones
	                    if (dead_zone_width < 240) {
	                        initDoubleWallExplosion(dead_zone_width, WIDTH - dead_zone_width - 60, 60);
#ifdef __EMSCRIPTEN__
                            EM_ASM({ if(window.playExplosion) window.playExplosion(); });
#endif
	                    }
	                }
	                dead_zone_width = ((7200 - dead_zone_timer) / 1200) * 60; // Every 1200 frames (20s) grows 60px
	                if (dead_zone_width > 240) dead_zone_width = 240; // Max 4 shrinks
	            }
	            left_boundary = dead_zone_width;
	            right_boundary = WIDTH - dead_zone_width;
	            paddle1_x = 20.0f + dead_zone_width;
	            paddle2_x = WIDTH - 20.0f - PADDLE_WIDTH - dead_zone_width;
	        } else {
	            paddle1_x = 20.0f;
	            paddle2_x = WIDTH - 20.0f - PADDLE_WIDTH;
	            dead_zone_width = 0;
	        }
	        
	        // Process Abilities
            bool p1_forward_pressed = p1_forward_spec ? specialKeys[key_p1_forward] : (keys[key_p1_forward] || (key_p1_forward >= 'a' && key_p1_forward <= 'z' && keys[key_p1_forward - 32]));
            bool p1_backward_pressed = p1_backward_spec ? specialKeys[key_p1_backward] : (keys[key_p1_backward] || (key_p1_backward >= 'a' && key_p1_backward <= 'z' && keys[key_p1_backward - 32]));
            bool p1_spec_pressed = p1_spec_spec ? specialKeys[key_p1_spec] : (keys[key_p1_spec] || (key_p1_spec >= 'a' && key_p1_spec <= 'z' && keys[key_p1_spec - 32]));
            
            if (special_charge_1 >= 100) {
                if (p1_forward_pressed && p1_loadout[0] != SKILL_NONE) { trigger_skill(1, p1_loadout[0]); special_charge_1 = 0; }
                else if (p1_backward_pressed && p1_loadout[1] != SKILL_NONE) { trigger_skill(1, p1_loadout[1]); special_charge_1 = 0; }
                else if (p1_spec_pressed && p1_loadout[2] != SKILL_NONE) { trigger_skill(1, p1_loadout[2]); special_charge_1 = 0; }
            }
            
            bool p2_forward_pressed = p2_forward_spec ? specialKeys[key_p2_forward] : (keys[key_p2_forward] || (key_p2_forward >= 'a' && key_p2_forward <= 'z' && keys[key_p2_forward - 32]));
            bool p2_backward_pressed = p2_backward_spec ? specialKeys[key_p2_backward] : (keys[key_p2_backward] || (key_p2_backward >= 'a' && key_p2_backward <= 'z' && keys[key_p2_backward - 32]));
            bool p2_spec_pressed = p2_spec_spec ? specialKeys[key_p2_spec] : (keys[key_p2_spec] || (key_p2_spec >= 'a' && key_p2_spec <= 'z' && keys[key_p2_spec - 32]));
            
            if (special_charge_2 >= 100 && currentMode == PVP) {
                if (p2_forward_pressed && p2_loadout[0] != SKILL_NONE) { trigger_skill(2, p2_loadout[0]); special_charge_2 = 0; }
                else if (p2_backward_pressed && p2_loadout[1] != SKILL_NONE) { trigger_skill(2, p2_loadout[1]); special_charge_2 = 0; }
                else if (p2_spec_pressed && p2_loadout[2] != SKILL_NONE) { trigger_skill(2, p2_loadout[2]); special_charge_2 = 0; }
            }
	        
	        paddle1_height = (p1_shield_frames > 0) ? PADDLE_HEIGHT * 2 : PADDLE_HEIGHT;
	        if (p1_shield_frames > 0) p1_shield_frames--;
	        paddle2_height = (p2_shield_frames > 0) ? PADDLE_HEIGHT * 2 : PADDLE_HEIGHT;
	        if (p2_shield_frames > 0) p2_shield_frames--;
	        if (ghost_frames > 0) ghost_frames--;
	        if (p1_freeze_frames > 0) p1_freeze_frames--;
	        if (p2_freeze_frames > 0) p2_freeze_frames--;
	        if (p1_emp_frames > 0) p1_emp_frames--;
	        if (p2_emp_frames > 0) p2_emp_frames--;
	    }
	    
    	// Mover as paletas (sempre ativas, mesmo durante delays)
	        // Track previous positions for trails
	        float old_p1_y = paddle1_y;
	        float old_p2_y = paddle2_y;
	        float old_ball_x = ball_x;
	        float old_ball_y = ball_y;

	        // Player 01
	        bool p1_down_pressed = p1_down_spec ? specialKeys[key_p1_down] : (keys[key_p1_down] || (key_p1_down >= 'a' && key_p1_down <= 'z' && keys[key_p1_down - 32]));
        bool p1_up_pressed = p1_up_spec ? specialKeys[key_p1_up] : (keys[key_p1_up] || (key_p1_up >= 'a' && key_p1_up <= 'z' && keys[key_p1_up - 32]));
        
        if (p1_emp_frames > 0) { bool temp = p1_down_pressed; p1_down_pressed = p1_up_pressed; p1_up_pressed = temp; }
        float p1_speed = (p1_freeze_frames > 0) ? PADDLE_SPEED * 0.5f : PADDLE_SPEED;

    	if (p1_down_pressed && paddle1_y > 0) {
    	    paddle1_y -= p1_speed;
    	}
    	if (p1_up_pressed && paddle1_y < HEIGHT - paddle1_height) {
    	    paddle1_y += p1_speed;
    	}
    
    	// Player 02
    	if (currentMode == PVP) {
            bool p2_down_pressed = p2_down_spec ? specialKeys[key_p2_down] : (keys[key_p2_down] || (key_p2_down >= 'a' && key_p2_down <= 'z' && keys[key_p2_down - 32]));
            bool p2_up_pressed = p2_up_spec ? specialKeys[key_p2_up] : (keys[key_p2_up] || (key_p2_up >= 'a' && key_p2_up <= 'z' && keys[key_p2_up - 32]));
            
            if (p2_emp_frames > 0) { bool temp = p2_down_pressed; p2_down_pressed = p2_up_pressed; p2_up_pressed = temp; }
            float p2_speed = (p2_freeze_frames > 0) ? PADDLE_SPEED * 0.5f : PADDLE_SPEED;

        	if (p2_down_pressed && paddle2_y > 0) {
        	    paddle2_y -= p2_speed;
        	}
        	if (p2_up_pressed && paddle2_y < HEIGHT - paddle2_height) {
        	    paddle2_y += p2_speed;
        	}
    	} else {
    	    // AI Logic
    	    float ai_speed = (p2_freeze_frames > 0) ? PADDLE_SPEED * 0.5f : PADDLE_SPEED;
    	    float react_x = 0;
    	    if (currentMode == PVE_EASY) { ai_speed = ai_speed * 0.4f; react_x = WIDTH / 2.0f; }
    	    else if (currentMode == PVE_MED) { ai_speed = ai_speed * 0.7f; react_x = WIDTH / 4.0f; }
    	    else if (currentMode == PVE_HARD) { ai_speed = ai_speed * 1.1f; react_x = 0.0f; }
    	    
    	    if (ball_x > react_x) {
        	    float paddle2_center = paddle2_y + paddle2_height / 2;
        	    bool ai_wants_up = (ball_y > paddle2_center + 10 && paddle2_y < HEIGHT - paddle2_height);
        	    bool ai_wants_down = (ball_y < paddle2_center - 10 && paddle2_y > 0);
        	    
        	    if (p2_emp_frames > 0) { bool temp = ai_wants_down; ai_wants_down = ai_wants_up; ai_wants_up = temp; }
        	    
        	    if (ai_wants_up) {
        	        paddle2_y += ai_speed;
        	    } else if (ai_wants_down) {
        	        paddle2_y -= ai_speed;
        	    }
    	    }
    	    
    	    // AI Special Usage (Medium & Hard)
    	    if (special_charge_2 >= 100 && ball_dx > 0) {
    	        if (currentMode == PVE_MED && ball_x > WIDTH * 0.6f && (rand() % 100) < 2) {
    	            trigger_skill(2, p2_loadout[rand() % 3]);
    	            special_charge_2 = 0;
    	        } else if (currentMode == PVE_HARD && ball_x > WIDTH * 0.4f && (rand() % 100) < 5) {
    	            trigger_skill(2, p2_loadout[rand() % 3]);
    	            special_charge_2 = 0;
    	        }
    	    }
    	}
    	
    	if (abs(paddle1_y - old_p1_y) > 0.5f) add_trail(paddle1_trail, p1_trail_idx, paddle1_x, paddle1_y);
    	if (abs(paddle2_y - old_p2_y) > 0.5f) add_trail(paddle2_trail, p2_trail_idx, paddle2_x, paddle2_y);
    	add_trail(ball_trail, ball_trail_idx, ball_x, ball_y);

        // Atualizar Particulas e Trails sempre
        for (int i = 0; i < NUM_PARTICLES; i++) {
            if (particles[i].life > 0) {
                particles[i].x += particles[i].dx;
                particles[i].y += particles[i].dy;
                particles[i].life--;
            }
        }
        for (int i=0; i<MAX_TRAILS; i++) {
            if (paddle1_trail[i].life > 0) paddle1_trail[i].life--;
            if (paddle2_trail[i].life > 0) paddle2_trail[i].life--;
            if (ball_trail[i].life > 0) ball_trail[i].life--;
        }

	    if (shake_frames > 0) {
	        shake_frames--;
            if (shake_frames == 0) {
                respawn_delay_frames = 60 - (dead_zone_width / 6); // Max 60, Min 20
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
		if ((ball_x >= paddle1_x && ball_x <= paddle1_x + PADDLE_WIDTH + 10) && (ball_y + BALL_RADIUS >= paddle1_y && ball_y - BALL_RADIUS <= paddle1_y + paddle1_height))
		{
			ball_dx = -ball_dx;
			ball_dx *= 1.10f; // 10% speed increase per hit
			ball_dy *= 1.10f;
            if (ball_dx > 18.0f) ball_dx = 18.0f;
            if (ball_dx < -18.0f) ball_dx = -18.0f;
            if (ball_dy > 18.0f) ball_dy = 18.0f;
            if (ball_dy < -18.0f) ball_dy = -18.0f;
            
            initHitExplosion(ball_x, ball_y, ball_dx);
			
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
		else if ((ball_x >= paddle2_x - 10 && ball_x <= paddle2_x + PADDLE_WIDTH) && (ball_y + BALL_RADIUS >= paddle2_y && ball_y - BALL_RADIUS <= paddle2_y + paddle2_height))
		{
		   	ball_dx = -ball_dx;
			ball_dx *= 1.10f; // 10% speed increase per hit
			ball_dy *= 1.10f;
            if (ball_dx > 18.0f) ball_dx = 18.0f;
            if (ball_dx < -18.0f) ball_dx = -18.0f;
            if (ball_dy > 18.0f) ball_dy = 18.0f;
            if (ball_dy < -18.0f) ball_dy = -18.0f;
		   	
		   	initHitExplosion(ball_x, ball_y, ball_dx);
		   	
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
        dead_zone_timer = 7200;
        dead_zone_width = 0;
        special_charge_1 = 0;
        special_charge_2 = 0;
        p1_fireball_active = false;
        p2_shield_frames = 0;
        
        ball_x = WIDTH/2; ball_y = HEIGHT/2;
    }
    
    EMSCRIPTEN_KEEPALIVE
    void trigger_action(int action) {
        if (action == 0) { // Resume
            resumeGame();
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
        printf("Frame %d: ball(%.1f, %.1f), paddle1(%.1f, %.1f), paddle2(%.1f, %.1f)\n", frameCount, ball_x, ball_y, paddle1_x, paddle1_y, paddle2_x, paddle2_y);
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
