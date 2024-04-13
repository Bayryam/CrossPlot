#include <GL/glut.h>
#include <cstdlib>
#include <vector>

using namespace std;
const GLsizei ARROW_LINE_LENGTH = 8;
const GLsizei DISTANCE_BETWEEN_AXIS_AND_CHAR = 15;
const float INCREASE_IN_CURVES_FOR_SMOOTHNESS = 0.01;

// pop-up window sizes
GLsizei windowWidth = 500;
GLsizei windowHeight = 500;

// parametres y-axis
GLfloat beginingYAxisY = 0;
GLfloat endingYAxisY = windowHeight;
GLfloat widthYAxis = (windowWidth / 2);

// parametres of x-axis
GLfloat beginingXAxisX = 0;
GLfloat endingXAxisX = windowWidth;
GLfloat heightXAxis = (windowHeight / 2);

// flags when something is drawn or not
GLboolean areControlLinesDrawn = GL_TRUE;
GLboolean areControlPointsDrawn = GL_TRUE;
GLboolean areFunctionsDrawn = GL_FALSE;

// colors of the Bezier curve
GLfloat curveRedColor = 1.0f;
GLfloat curveBlueColor = 0.0f;
GLfloat curveGreenColor = 0.0f;

// colors of the control lines
GLfloat lineRedColor = 0.0f;
GLfloat lineBlueColor = 1.0f;
GLfloat lineGreenColor = 0.0f;

char coordinateAxesSymbols[] = { 'x', 'y', 'T', 'T' };
int charactersXCoordinates[] = { endingXAxisX - DISTANCE_BETWEEN_AXIS_AND_CHAR, widthYAxis + DISTANCE_BETWEEN_AXIS_AND_CHAR,
								 beginingXAxisX + DISTANCE_BETWEEN_AXIS_AND_CHAR, widthYAxis + DISTANCE_BETWEEN_AXIS_AND_CHAR };
int charactersYCoordinates[] = { heightXAxis + DISTANCE_BETWEEN_AXIS_AND_CHAR, endingYAxisY - DISTANCE_BETWEEN_AXIS_AND_CHAR,
								 heightXAxis + DISTANCE_BETWEEN_AXIS_AND_CHAR, beginingYAxisY + DISTANCE_BETWEEN_AXIS_AND_CHAR };

struct Point
{
	GLfloat x;
	GLfloat y;

	Point(GLfloat x, GLfloat y) :x(x), y(y) {}

	Point() = default;
};

vector<Point> controlPoints;
vector<Point> xFuncPoints;
vector<Point> yFuncPoints;

void init()
{
	glClearColor(0.0, 0.0, 0.0, 0.0);

	// reduce jagged edges
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_LINEAR);
	glEnable(GLUT_MULTISAMPLE);
}

void drawPoint(Point point)
{
	glBegin(GL_POINTS);
	glVertex2f(point.x, point.y);
	glEnd();
	glFlush();
}

void drawLine(Point first, Point second)
{
	glBegin(GL_LINES);
	glVertex2f(first.x, first.y);
	glVertex2f(second.x, second.y);
	glEnd();
	glFlush();
}

void drawCoordinateSystem()
{
	//white
	glColor3f(1.0, 1.0, 1.0);

	Point leftPointOfXAxis(beginingXAxisX, heightXAxis);
	Point rightPointOfXAxis(endingXAxisX, heightXAxis);
	Point bottomPointOfYAxis(widthYAxis, beginingYAxisY);
	Point upperPointOfYAxis(widthYAxis, endingYAxisY);

	// draw coordinate axes
	drawLine(leftPointOfXAxis, rightPointOfXAxis);
	drawLine(bottomPointOfYAxis, upperPointOfYAxis);

	// draw all the arrows
	drawLine({ endingXAxisX, heightXAxis }, { endingXAxisX - ARROW_LINE_LENGTH, heightXAxis + ARROW_LINE_LENGTH });
	drawLine({ endingXAxisX, heightXAxis }, { endingXAxisX - ARROW_LINE_LENGTH, heightXAxis - ARROW_LINE_LENGTH });
	drawLine({ beginingXAxisX, heightXAxis }, { beginingXAxisX + ARROW_LINE_LENGTH, heightXAxis + ARROW_LINE_LENGTH });
	drawLine({ beginingXAxisX, heightXAxis }, { beginingXAxisX + ARROW_LINE_LENGTH, heightXAxis - ARROW_LINE_LENGTH });
	drawLine({ widthYAxis, endingYAxisY }, { widthYAxis + ARROW_LINE_LENGTH, endingYAxisY - ARROW_LINE_LENGTH });
	drawLine({ widthYAxis, endingYAxisY }, { widthYAxis - ARROW_LINE_LENGTH, endingYAxisY - ARROW_LINE_LENGTH });
	drawLine({ widthYAxis, beginingYAxisY }, { widthYAxis + ARROW_LINE_LENGTH, beginingYAxisY + ARROW_LINE_LENGTH });
	drawLine({ widthYAxis, beginingYAxisY }, { widthYAxis - ARROW_LINE_LENGTH, beginingYAxisY + ARROW_LINE_LENGTH });

	// print symbols
	for (int i = 0; i < 4; i++)
	{
		glRasterPos2i(charactersXCoordinates[i], charactersYCoordinates[i]);
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, coordinateAxesSymbols[i]);
		
	}
}

void computeXFuncPoints()
{
	int size = controlPoints.size();
	xFuncPoints.resize(size);

	for (int i = 1; i <= size; i++)
	{
		xFuncPoints[i - 1].y = (windowHeight / 2) - i * ((windowHeight / 2) / (size + 1));
		// x coordinate is the same
		xFuncPoints[i - 1].x = controlPoints[i - 1].x;
	}
}

void computeYFuncPoints()
{
	int size = (int)controlPoints.size();
	yFuncPoints.resize(size);

	for (int i = 1; i <= size; i++)
	{
		yFuncPoints[i - 1].x = (windowWidth / 2) - i * ((windowWidth / 2) / (size + 1));
		// y coordinate stays the same
		yFuncPoints[i - 1].y = controlPoints[i - 1].y;
	}
}

// calculate point for some t
Point computeBezierPoint(int r, int index, double valueT, vector<Point>& points)
{
	if (r == 0)
		return points[index];

	Point p1 = computeBezierPoint(r - 1, index, valueT, points);
	Point p2 = computeBezierPoint(r - 1, index + 1, valueT, points);

	Point newPt;
	newPt.x = (1 - valueT) * p1.x + valueT * p2.x;
	newPt.y = (1 - valueT) * p1.y + valueT * p2.y;

	return newPt;
}

// this is where we draw the curve
void computeBezier(vector<Point>& points)
{
	glColor3f(curveRedColor, curveGreenColor, curveBlueColor);
	Point previous = points[0];
	Point next;

	// for the curve to look smoother
	for (float t = 0.0; t <= 1.0; t += INCREASE_IN_CURVES_FOR_SMOOTHNESS)
	{
		next = computeBezierPoint(points.size() - 1, 0, t, points);
		drawLine(previous, next);
		previous = next;
	}
}

 // action performed when a change is made
void drawBezier()
{
	glClear(GL_COLOR_BUFFER_BIT);

	drawCoordinateSystem();
	int controlPointsCount = controlPoints.size();

	glLineWidth(1);

	if (controlPointsCount > 1 && areControlLinesDrawn)
	{
		glColor3f(lineRedColor, lineGreenColor, lineBlueColor);
		for (int i = 0; i < controlPointsCount - 1; i++)
			drawLine(controlPoints[i], controlPoints[i + 1]);
	}

	glLineWidth(2);

	if (areControlPointsDrawn)
	{
		glColor3f(0.9, 0.1, 1.0);
		glPointSize(5);
		for (int i = 0; i < controlPointsCount; i++)
			drawPoint(controlPoints[i]);
	}

	if (controlPointsCount > 0)
		computeBezier(controlPoints);

	if (areFunctionsDrawn && controlPointsCount > 0)
	{
		computeXFuncPoints();
		computeYFuncPoints();

		glLineWidth(1);

		if (controlPointsCount > 1 && areControlLinesDrawn)
		{
			glColor3f(lineRedColor, lineGreenColor, lineBlueColor);
			for (int i = 0; i < controlPointsCount - 1; i++)
				drawLine(xFuncPoints[i], xFuncPoints[i + 1]);
		}

		glLineWidth(2);

		if (areControlPointsDrawn)
		{
			glColor3f(0.9, 0.1, 1.0);
			glPointSize(6);
			for (int i = 0; i < controlPointsCount; i++)
				drawPoint(xFuncPoints[i]);
		}

		computeBezier(xFuncPoints);

		glLineWidth(1);

		if (controlPointsCount > 1 && areControlLinesDrawn)
		{
			glColor3f(lineRedColor, lineGreenColor, lineBlueColor);
			for (int i = 0; i < controlPointsCount - 1; i++)
				drawLine(yFuncPoints[i], yFuncPoints[i + 1]);
		}

		glLineWidth(2);

		if (areControlPointsDrawn)
		{
			glColor3f(0.9, 0.1, 1.0);
			glPointSize(6);
			for (int i = 0; i < controlPointsCount; i++)
				drawPoint(yFuncPoints[i]);
		}

		computeBezier(yFuncPoints);
	}

	glFlush();
}

bool isInFirstQuadrant(int x, int y)
{
	return (x >= windowWidth / 2 && x <= windowWidth) && (y >= windowHeight / 2 && y <= windowHeight);
}

void mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && isInFirstQuadrant(x, windowHeight - y))
	{
		Point point;
		point.x = x;
		point.y = windowHeight - y;
		controlPoints.push_back(point);
	}
	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN && isInFirstQuadrant(x, windowHeight - y))
	{
		if (!controlPoints.empty())
			controlPoints.pop_back();
	}
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case '-':
		areControlLinesDrawn = !areControlLinesDrawn;
		break;
	case '.':
		areControlPointsDrawn = !areControlPointsDrawn;
		break;
	case 'f':
		areFunctionsDrawn = !areFunctionsDrawn;
		break;
	case 'r':
		if (curveRedColor < 0.1)
			curveRedColor = 1;
		else
			curveRedColor -= 0.1;
		break;
	case 'b':
		if (curveBlueColor < 0.1)
			curveBlueColor = 1;
		else
			curveBlueColor -= 0.1;
		break;
	case 'g':
		if (curveGreenColor < 0.1)
			curveGreenColor = 1;
		else
			curveGreenColor -= 0.1;
		break;
	case '1':
		if (lineRedColor < 0.1)
			lineRedColor = 1;
		else
			lineRedColor -= 0.1;
		break;
	case '2':
		if (lineBlueColor < 0.1)
			lineBlueColor = 1;
		else
			lineBlueColor -= 0.1;
		break;
	case '3':
		if (lineGreenColor < 0.1)
			lineGreenColor = 1;
		else
			lineGreenColor -= 0.1;
		break;
	default:
		break;
	}

	drawBezier();
}

void update(GLint newWidth, GLint newHeight)
{
	GLfloat widthRatio = (GLfloat)newWidth / windowWidth;
	GLfloat heightRatio = (GLfloat)newHeight / windowHeight;
	windowWidth = newWidth; 
	windowHeight = newHeight;

	endingXAxisX = windowWidth;
	endingYAxisY = windowHeight;
	heightXAxis = (windowHeight / 2);
	widthYAxis = (windowWidth / 2);

	charactersXCoordinates[0] = endingXAxisX - DISTANCE_BETWEEN_AXIS_AND_CHAR;
	charactersXCoordinates[1] = widthYAxis + DISTANCE_BETWEEN_AXIS_AND_CHAR;
	charactersXCoordinates[2] = beginingXAxisX + DISTANCE_BETWEEN_AXIS_AND_CHAR;
	charactersXCoordinates[3] = widthYAxis + DISTANCE_BETWEEN_AXIS_AND_CHAR;
	charactersYCoordinates[0] = heightXAxis + DISTANCE_BETWEEN_AXIS_AND_CHAR;
	charactersYCoordinates[1] = endingYAxisY - DISTANCE_BETWEEN_AXIS_AND_CHAR;
	charactersYCoordinates[2] = heightXAxis + DISTANCE_BETWEEN_AXIS_AND_CHAR;
	charactersYCoordinates[3] = beginingYAxisY + DISTANCE_BETWEEN_AXIS_AND_CHAR;

	for (int i = 0; i < controlPoints.size(); i++)
	{
		controlPoints[i].x *= widthRatio;
		controlPoints[i].y *= heightRatio;
	}
}

void reshape(GLint newWidth, GLint newHeight)
{
	glViewport(0, 0, (GLsizei)newWidth, (GLsizei)newHeight);

	update(newWidth, newHeight);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, windowWidth, 0.0, windowHeight);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT);
}

void displayFunc()
{
	glClear(GL_COLOR_BUFFER_BIT);

	drawBezier();

	glFlush();
}

int main(int argc, char** argv)
{
	controlPoints.reserve(1000);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glutInitWindowPosition(500, 100);
	glutInitWindowSize(windowWidth, windowHeight);
	glutCreateWindow("Bezier Curve");

	init();
	glutReshapeFunc(reshape);
	glutDisplayFunc(displayFunc);
	glutMouseFunc(mouse);
	glutKeyboardFunc(keyboard);

	glutMainLoop();
	return 0;
}