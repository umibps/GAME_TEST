#include <GL/glut.h>

void Display(void)
{

}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);

	(void)glutCreateWindow("GAME TEST");

	glutDisplayFunc(Display);

	glutMainLoop();

	return 0;
}