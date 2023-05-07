#include <stdlib.h>
#include <stdio.h>
#include <glut.h>
#include <windows.h>
#include <math.h>

//parametros pré-definidos:
#define WIDTH 640
#define HEIGHT 500
#define PADDLE_HEIGHT 80
#define PADDLE_WIDTH 35
#define PADDLE_SPEED 10
#define BALL_RADIUS 10
#define BALL_SPEED 3
#define CIRCLE_RADIUS 50

// Variáveis alocadas com escopo global:
float left = WIDTH;
float right = WIDTH * 2;
int score1 = 0, score2 = 0;
float paddle1_x = HEIGHT / 2 - PADDLE_HEIGHT / 2;
float paddle2_x = HEIGHT / 2 - PADDLE_HEIGHT / 2;
float ball_x = WIDTH / 2;
float ball_y = HEIGHT / 2;
float ball_dx = BALL_SPEED;
float ball_dy = BALL_SPEED;
//float ajust_paddles = 30.0;
bool Gamepaused = false;
float Ball_speed_pause = BALL_SPEED;
float Ball_speed_increment = 20;

// Cria a janela:
void init()
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WIDTH, 0, HEIGHT);
}   

// Função de Pause 
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
// Função de Resume
void resumeGame()
{
	if (Gamepaused == true)
	{
		Gamepaused = false;
		ball_dx = Ball_speed_pause;
		ball_dy = Ball_speed_pause;
	}
}
// Função que detecta a tecla precionada para pausar e retomar o game
void keyboard(unsigned char key, int x, int y) 
{
	switch(key)
	{
	case 13: // 13 é o ENTER na tabela ASCII - Faz o L
		if(Gamepaused == false)
		{
			pauseGame();
			Beep(300, 100);
			Beep(200, 100);
		}	
		else 
		{
			resumeGame();
			Beep(200, 100);
			Beep(300, 100);
		}
		break;
	
	case 27:
		glutDestroyWindow(glutGetWindow()); // Destrói a janela e encerra o programa	
		break;
	
	default:
		break;
	}
}
// Função de Desenho
void display()
{
	// Desenhando a mesa
    glClear(GL_COLOR_BUFFER_BIT);
	
    glColor3f(0.4, 0.7, 1.0);
    glBegin(GL_QUADS);
    glVertex2f(0.0, 0.0);
    glVertex2f(WIDTH, 0.0);
    glVertex2f(WIDTH, HEIGHT);
    glVertex2f(0.0, HEIGHT);
    glEnd();
    
    // Retângulo demarcador de pontos - Player 01
    glColor3f(0.2, 0.5, 0.8);
    glBegin(GL_QUADS);
    glVertex2f(30.0, 450.0);
    glVertex2f(WIDTH / 3 - 10, 450.0);
    glVertex2f(WIDTH / 3 - 10, HEIGHT - 8);
    glVertex2f(30.0, HEIGHT - 8);
    glEnd();
    
    // Retângulo demarcador de pontos - Player 02
	glColor3f(0.2, 0.5, 0.8);
    glBegin(GL_QUADS);
    glVertex2f(420.0, 450.0);
    glVertex2f(WIDTH - 50, 450.0);
    glVertex2f(WIDTH - 50, HEIGHT - 8);
    glVertex2f(420.0, HEIGHT - 8);
    glEnd();
    
	// Linha central vertical
	glLineWidth(2.0);
    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_LINES);
    glVertex2f(WIDTH / 2.0, 0.0);
    glVertex2f(WIDTH / 2.0, HEIGHT - 60.0);
    
    // Linha central horizontal
    glBegin(GL_LINES);
    glVertex2f(0.0, HEIGHT / 2.0 - 30);
	glVertex2f(WIDTH, HEIGHT / 2.0 - 30);
    
	// Linha topo
    glVertex2f(0.0, HEIGHT / 2.0 + 190.0);
    glVertex2f(WIDTH, HEIGHT / 2.0 + 190.0);
    
    // Linha debaixo
    glVertex2f(0.0, HEIGHT / 2.0 - 249.0);
    glVertex2f(WIDTH, HEIGHT / 2.0 - 249.0);
    glEnd();
    
	// Linha esquerda
    glColor3f(1.0, 0.0, 0.0);
    glLineWidth(4.0);
    glBegin(GL_LINES);
    glVertex2f(0.0, HEIGHT / 2.0 - 500);
    glVertex2f(0.0, HEIGHT / 2.0 + 188.0);
    
	// Linha direita
    glVertex2f(WIDTH, HEIGHT / 2.0 - 500.0);
    glVertex2f(WIDTH, HEIGHT / 2.0 + 188.0);
    glEnd();
		
 	// Desenha as paletas
 	//Player 01:
 	glBegin(GL_QUADS);
 	glColor3f(1.0, 1.0, 1.0);
 	glVertex2i(20.0, paddle1_x);
 	glVertex2i(PADDLE_WIDTH, paddle1_x);
 	glVertex2i(PADDLE_WIDTH, paddle1_x + PADDLE_HEIGHT);
 	glVertex2i(20.0, paddle1_x + PADDLE_HEIGHT);
    
 	// Player 02:
 	glColor3f(1.0, 1.0, 1.0);
 	glVertex2i(WIDTH - PADDLE_WIDTH, paddle2_x);
 	glVertex2i(WIDTH - 20.0, paddle2_x);
 	glVertex2i(WIDTH - 20.0, paddle2_x + PADDLE_HEIGHT);
 	glVertex2i(WIDTH - PADDLE_WIDTH, paddle2_x + PADDLE_HEIGHT);
 	glEnd();

 	// Desenha a bola:
 	glBegin(GL_TRIANGLE_FAN);
 	glColor3f(1.0, 1.0, 1.0);
  	for (float angle = 0.0; angle <= 360.0; angle += 5.0)
  	{
		float radian = angle * 3.1416 / 180.0;
		float x = ball_x + BALL_RADIUS * cos(radian);
		float y = ball_y + BALL_RADIUS * sin(radian);
		glVertex2f(x, y);
 	   }
 	   glEnd();

    // Exibir os pontos:
	char buffer[100];
    sprintf(buffer, "%d", score1);
    glColor3f(1.0, 1.0, 1.0);
	glRasterPos2i(150, HEIGHT - 40);
    for (int i = 0; buffer[i] != '\0'; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, buffer[i]);
    }
    
    sprintf(buffer, "Player 1");
    glColor3f(1.0, 1.0, 1.0);
    glRasterPos2i(30, HEIGHT - 20);
    for (int i = 0; buffer[i] != '\0'; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, buffer[i]);
    }
    
    sprintf(buffer, "%d", score2);
    glColor3f(1.0, 1.0, 1.0);
    glRasterPos2i(WIDTH - 100, HEIGHT - 40);
    for (int i = 0; buffer[i] != '\0'; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, buffer[i]);
    }
     
    sprintf(buffer, "Player 2");
    glColor3f(1.0, 1.0, 1.0);
    glRasterPos2i(WIDTH - 220, HEIGHT - 20);
    for (int i = 0; buffer[i] != '\0'; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, buffer[i]);
    }

	if (Gamepaused == true)
	{
		sprintf(buffer, "PAUSED");
    	glColor3f(1.0, 1.0, 1.0);
    	glRasterPos2i(WIDTH - 360, HEIGHT - 40);
    	for (int i = 0; buffer[i] != '\0'; i++)
    	{
        	glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, buffer[i]);
    	}
	}
    glutSwapBuffers();
}
	// Função para evitar distorções caso haja alteração do tamanho da janela: 
	void reshape(int w, int h) 
	{
    	w = WIDTH;
    	h = HEIGHT;

    	// Define a área da janela de renderização
    	glViewport(0, 0, WIDTH, HEIGHT);
	}

void update(int value)
{
	if (Gamepaused == false)
	{	
    	// Mover a bola
    	ball_x += ball_dx;
    	ball_y += ball_dy;

    	// Verificar colisões com as paredes
		if (ball_x + BALL_RADIUS >= WIDTH || ball_x - BALL_RADIUS <= 0)
		{
    	   	ball_dx = -ball_dx;
		}
		if (ball_y + BALL_RADIUS >= HEIGHT - 60 || ball_y - BALL_RADIUS <= 0)
		{
    	   	ball_dy = -ball_dy;
		}
	
		// Marcador de pontuação:
		if (ball_x + BALL_RADIUS >= left || ball_x - BALL_RADIUS == 0)
		{
    		score1++;
    		Beep(800, 100);
    		Beep(900, 300);
    		// reset para a bola voltar ao centro
    		ball_x = 320.0;
    		ball_y = 200.0;
    		ball_dx = BALL_SPEED;
			ball_dy = BALL_SPEED;
		}	

		if (ball_x + BALL_RADIUS >= right || ball_x - BALL_RADIUS <= 0)
		{
    	   	score2++;
   		   	Beep(800, 100);
    		Beep(900, 300);
    	   	// reset para a bola voltar ao centro
    	   	ball_x = 320.0;
    	   	ball_y = 200.0;
    		ball_dx == BALL_SPEED;
			ball_dy == BALL_SPEED;
		}

		// Verificar colisões com as paletas
		// Verifica colisão da bola com as paletas
		if ((ball_x >= 20 && ball_x <= PADDLE_WIDTH + 10) && (ball_y + BALL_RADIUS >= paddle1_x && ball_y - BALL_RADIUS <= paddle1_x + PADDLE_HEIGHT))
		{
			ball_dx = -ball_dx;
			ball_dx++;
			ball_dy++;
		}
		else if ((ball_x >= WIDTH - PADDLE_WIDTH - 10 && ball_x <= WIDTH - 20) && (ball_y + BALL_RADIUS >= paddle2_x && ball_y - BALL_RADIUS <= paddle2_x + PADDLE_HEIGHT))
		{
		   	ball_dx = -ball_dx;
		}

    	// Mover as paletas
    	// Player 01
    	if (GetAsyncKeyState('S') && paddle1_x > 0)
    	{
    	    paddle1_x -= PADDLE_SPEED;
    	}
    	if (GetAsyncKeyState('W') && paddle1_x < WIDTH - PADDLE_WIDTH - 250)
    	{
    	    paddle1_x += PADDLE_SPEED;
    	}
    
    	// Player 02
    	if (GetAsyncKeyState(VK_DOWN) && paddle2_x > 0)
		{
    	    paddle2_x -= PADDLE_SPEED;
    	}
    	if (GetAsyncKeyState(VK_UP) && paddle2_x < WIDTH - PADDLE_WIDTH - 250)
    	{
    	    paddle2_x += PADDLE_SPEED;
    	}

    	glutPostRedisplay();
    	glutTimerFunc(16, update, 0);
	}
	else
	{


    	// Mover as paletas
    	// Player 01
    	if (GetAsyncKeyState('S') && paddle1_x > 0)
    	{
    	    paddle1_x == PADDLE_SPEED;
    	}
    	if (GetAsyncKeyState('W') && paddle1_x < WIDTH - PADDLE_WIDTH - 250)
    	{
    	    paddle1_x == PADDLE_SPEED;
    	}
    
    	// Player 02
    	if (GetAsyncKeyState(VK_DOWN) && paddle2_x > 0)
		{
    	    paddle2_x == PADDLE_SPEED;
    	}
    	if (GetAsyncKeyState(VK_UP) && paddle2_x < WIDTH - PADDLE_WIDTH - 250)
    	{
    	    paddle2_x == PADDLE_SPEED;
    	}
    	
    	glutPostRedisplay();
    	glutTimerFunc(16, update, 0);
	}
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Pong Game - A Game to the Year 2023");
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(0, update, 0);
    init();
    glutMainLoop();
    return 0;
}

