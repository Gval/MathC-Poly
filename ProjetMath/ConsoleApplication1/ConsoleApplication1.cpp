/*******************************************************/
/*					didac.c							   */
/*******************************************************/
/*													   */
/*	Préambule OpenGL sous Glut			               */
/*  ESGI : 2I année 						           */
/*													   */
/*******************************************************/
/*													   */
/*  Fenêtre graphique 2D vierge                        */
/*  Evènement souris actif, q pour quitter             */
/*													   */
/*******************************************************/



#pragma region DECLARATIONS


#include <windows.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include "stdafx.h"
#include <iostream>
#include <vector> //Ne pas oublier !
#include <math.h>
#include <stack>
#include <list>
#include <algorithm>
using namespace std;


// Struct
struct point
{
	float x, y;

	point(){};
	point(float _x, float _y) : x(_x), y(_y)
	{
	}
};

struct ElementLCA
{
	float x, yMax, slope;

	ElementLCA(){};
	ElementLCA(float _x, float _yMax, float _slope) : x(_x), yMax(_yMax), slope(_slope)
	{
	}
};
inline bool operator<(const ElementLCA& a, const ElementLCA& b){ return a.x < b.x; }
inline bool operator>(const ElementLCA& a, const ElementLCA& b){ return b < a; }
inline bool operator<=(const ElementLCA& a, const ElementLCA& b){ return !(b < a); }
inline bool operator>=(const ElementLCA& a, const ElementLCA& b){ return !(a < b); }
inline bool operator==(const ElementLCA& a, const ElementLCA& b){ return !(a < b || b < a); }
inline bool operator!=(const ElementLCA& a, const ElementLCA& b){ return (a < b || b < a); }


// Window variables
int windowWidth = 500;
int windowHeight = 500;


// States variables
int drawMode = -1;
bool windowIsFinsih = false;
bool sensFenetre = true;
int currentPolygon = -1;
vector<bool> polygonIsFinsih;


// Polygons variables
vector<point> window;
vector<vector<point>> polygons;


// Prototypes
void DisplayPolygons(void);
void DrawPolygonWire(vector<point> polygon, float r, float g, float b);
void DrawPolygonFlat(vector<point> polygon, float r, float g, float b);
void Keyboard(unsigned char touche, int x, int y);
void Mouse(int bouton, int etat, int x, int y); 

void AddMenu();
void Select(int selection);
void SelectDrawWindow(int selection);
void SelectDrawPolygon(int selection);
void Reset();
void StartDrawWindow();
void EndDrawWindow();
void StartDrawNewPolygon();
void EndDrawPolygon();

void RemplissageRégionConnexité4Recursif(int x, int y, float r, float g, float b);
void RemplissageRégionConnexité4Iteratif(int x, int y, float r, float g, float b);
void RemplissageLigne(int x, int y, float r, float g, float b);
void RemplissageRectEG(vector<point> polygon, float r, float g, float b);
void RectangleEnglobant(vector<point> polygon, point * rectEG);
bool Interieur(int x, int y, vector<point> polygon);
bool IntersectSegment(point a, point b, point c, point d);
void RemplissageLCA(vector<point> polygon, float r, float g, float b);

bool DetectWindowDirection();
bool Coupe(point a, point b, point c, point d);
point Intersection(point a, point b, point c, point d);
bool Visible(point test, point a, point b);
vector<point> AlgoSutherlandHodgman(vector<point> polygonToCut);


#pragma endregion DECLARATIONS



#pragma region MAIN


int main(int argc, char **argv)
{  
	// Initialisation de glut et creation de la fenetre 
	glutInit(&argc,argv);                       // Initialisation
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH); // mode d'affichage RGB, et test prafondeur
	glutInitWindowSize(windowWidth, windowHeight);                // dimension fenêtre
	glutInitWindowPosition(100, 100);           // position coin haut gauche
	glutCreateWindow("A vous de jouer!!!");  // nom

	// Repère 2D délimitant les abscisses et les ordonnées
	gluOrtho2D(0, windowWidth, windowHeight, 0);

	// Initialisation d'OpenGL
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glPointSize(1.0);               // taille d'un point: 1px

	// Enregistrement des fonctions de rappel
	// => initialisation des fonctions callback appelées par glut 
	//glutIdleFunc(afficher);
	glutDisplayFunc(DisplayPolygons);
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(Mouse);

	AddMenu();

	// rq: le callback de fonction (fonction de rappel) est une fonction qui est passée en argument à une
	// autre fonction. Ici, le main fait usage des deux fonctions de rappel (qui fonctionnent en même temps)
	// alors qu'il ne les connaît pas par avance.

	// Entrée dans la boucle principale de glut, traitement des évènements 
	glutMainLoop();         // lancement de la boucle de réception des évènements
	system("pause");
	return 0;
}


#pragma endregion MAIN



#pragma region GLUT


void DisplayPolygons()
{
	glClear(GL_COLOR_BUFFER_BIT);

	DrawPolygonWire(window, 0.0, 0.0, 1.0);

	for (int i = 0; i < polygons.size(); ++i)
	{
		DrawPolygonWire(polygons[i], 1.0, 0.0, 0.0);
	}
	
	if (windowIsFinsih)
	{
		for (int i = 0; i < polygons.size(); ++i)
		{
			vector<point> polygonOut = AlgoSutherlandHodgman(polygons[i]);
			if (polygonIsFinsih[i] && polygonOut.size() > 2)
				DrawPolygonFlat(polygonOut, 0.0, 1.0, 0.0);
		}
	}
	
	glutSwapBuffers();
	glFlush();
}


void DrawPolygonWire(vector<point> polygon, float r, float g, float b)
{
	glBegin(GL_LINES);
	glColor3f(r, g, b);
	int size = polygon.size() - 1;
	for (int i = 0; i < size; ++i){
		glVertex2f(polygon[i].x, polygon[i].y);
		glVertex2f(polygon[i + 1].x, polygon[i + 1].y);
	}
	glEnd();
}


void DrawPolygonFlat(vector<point> polygon, float r, float g, float b)
{
	//RemplissageRégionConnexité4Recursif(windowWidth * 0.5f, windowHeight * 0.5f, r, g, b);
	//RemplissageRégionConnexité4Iteratif(windowWidth * 0.5f, windowHeight * 0.5f, r, g, b);
	//RemplissageLigne(windowWidth * 0.5f, windowHeight * 0.5f, r, g, b);
	//RemplissageRectEG(polygon, r, g,  b);
	RemplissageLCA(polygon, r, g, b);
}


void Mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		if (drawMode == 0)
		{
			window.push_back(point(x, y));
		}
		else if (drawMode == 1)
		{
			polygons[currentPolygon].push_back(point(x, y));
		}

		glutPostRedisplay();
	}
}


void Keyboard(unsigned char touche, int x, int y){
	switch (touche)
	{
	case 'q':/* Quitter le programme */
		exit(0);
		break;

	case 'r':
		Reset();
		break;
	}

	glutPostRedisplay();
}


#pragma endregion GLUT



#pragma region MENU


void AddMenu()
{
	int menuWindow = glutCreateMenu(SelectDrawWindow);
	glutAddMenuEntry("Start", 1);
	glutAddMenuEntry("End", 2);
	int menuPolygon = glutCreateMenu(SelectDrawPolygon);
	glutAddMenuEntry("Start New", 1);
	glutAddMenuEntry("End", 2);
	glutCreateMenu(Select);
	glutAddSubMenu("Draw Window", menuWindow);
	glutAddSubMenu("Draw Polygon", menuPolygon);
	glutAddMenuEntry("Reset", 3);
	glutAddMenuEntry("Quitter", 0);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}


void Select(int selection)
{
	switch (selection) {
	case 0:
		exit(0);
		break;
	case 3:
		Reset();
		break;
	}
	glutPostRedisplay();
}


void SelectDrawWindow(int selection)
{
	switch (selection) {
	case 1:
		StartDrawWindow();
		break;
	case 2:
		EndDrawWindow();
		break;
	}
	glutPostRedisplay();
}


void SelectDrawPolygon(int selection)
{
	switch (selection) {
	case 1:
		StartDrawNewPolygon();
		break;
	case 2:
		EndDrawPolygon();
		break;
	}
	glutPostRedisplay();
}


void Reset()
{
	drawMode = -1;
	
	windowIsFinsih = false;
	window.clear();

	polygons.clear();
	currentPolygon = -1;
	polygonIsFinsih.clear();
}


void StartDrawWindow()
{
	if (drawMode == 1)
		EndDrawPolygon();

	drawMode = 0;
	windowIsFinsih = false;
	window.clear();
}


void EndDrawWindow()
{
	if (drawMode == 0)
	{
		drawMode = -1;
		if (window.size() > 2)
		{
			window.push_back(window[0]);
			windowIsFinsih = true;
			sensFenetre = DetectWindowDirection();
		}
		else
		{
			window.clear();
		}
	}
}


void StartDrawNewPolygon()
{
	if (drawMode == 0)
		EndDrawWindow();
	else if (drawMode == 1)
		EndDrawPolygon();

	drawMode = 1;
	currentPolygon++;
	polygons.push_back(vector<point>());
	polygonIsFinsih.push_back(false);
}


void EndDrawPolygon()
{
	if (drawMode == 1)
	{
		drawMode = -1;
		polygons[currentPolygon].push_back(polygons[currentPolygon][0]);
		polygonIsFinsih[currentPolygon] = true;
	}
}


#pragma endregion MENU



#pragma region REMPLISSAGE


void RemplissageRégionConnexité4Recursif(int x, int y, float r, float g, float b)
{
	unsigned char pixel[3] = { 0 };
	glReadPixels(x, y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
	if (pixel[0] == 0 && pixel[1] == 0 && pixel[2] == 0)
	{
		glBegin(GL_POINTS);
		glColor3f(r, g, b);
		glVertex2i(x, y);
		glEnd();
		RemplissageRégionConnexité4Recursif(x + 1, y, r, g, b);
		RemplissageRégionConnexité4Recursif(x - 1, y, r, g, b);
		RemplissageRégionConnexité4Recursif(x, y + 1, r, g, b);
		RemplissageRégionConnexité4Recursif(x, y - 1, r, g, b);
	}
}


void RemplissageRégionConnexité4Iteratif(int x, int y, float r, float g, float b)
{
	stack<point> pile;
	pile.push(point(x, y));

	while (!pile.empty())
	{
		point p = pile.top();

		pile.pop();

		unsigned char pixel[3] = { 0 };
		glReadPixels(p.x, p.y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
		if (pixel[0] == 0 && pixel[1] == 0 && pixel[2] == 0)
		{
			glBegin(GL_POINTS);
			glColor3f(r, g, b);
			glVertex2i(p.x, p.y);
			glEnd();
		}
		
		glReadPixels(p.x + 1, p.y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
		if (pixel[0] == 0 && pixel[1] == 0 && pixel[2] == 0)
			pile.push(point(p.x + 1, p.y));

		glReadPixels(p.x - 1, p.y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
		if (pixel[0] == 0 && pixel[1] == 0 && pixel[2] == 0)
			pile.push(point(p.x - 1, p.y));
		
		glReadPixels(p.x, p.y + 1, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
		if (pixel[0] == 0 && pixel[1] == 0 && pixel[2] == 0)
			pile.push(point(p.x, p.y + 1));

		glReadPixels(p.x, p.y - 1, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
		if (pixel[0] == 0 && pixel[1] == 0 && pixel[2] == 0)
			pile.push(point(p.x, p.y - 1));
	}
}


void RemplissageLigne(int x, int y, float r, float g, float b)
{

}


void RemplissageRectEG(vector<point> polygon, float r, float g, float b)
{
	point RectEG[2];
	RectangleEnglobant(polygon, RectEG);

	for (int x = RectEG[0].x; x <= RectEG[1].x; ++x)
	{
		for (int y = RectEG[0].y; y <= RectEG[1].y; ++y)
		{
			if (Interieur(x, y, polygon))
			{
				glBegin(GL_POINTS);
				glColor3f(r, g, b);
				glVertex2i(x, y);
				glEnd();
			}
		}
	}
}


void RectangleEnglobant(vector<point> polygon, point * rectEG)
{
	rectEG[0].x = polygon[0].x;
	rectEG[0].y = polygon[0].y;
	rectEG[1].x = polygon[0].x;
	rectEG[1].y = polygon[0].y;

	for (int i = 1; i < polygon.size(); ++i)
	{
		if (polygon[i].x < rectEG[0].x)
			rectEG[0].x = polygon[i].x;

		if (polygon[i].y < rectEG[0].y)
			rectEG[0].y = polygon[i].y;

		if (polygon[i].x > rectEG[1].x)
			rectEG[1].x = polygon[i].x;

		if (polygon[i].y > rectEG[1].y)
			rectEG[1].y = polygon[i].y;
	}
}


bool Interieur(int x, int y, vector<point> polygon)
{
	point p = point(x, y);
	point infinity = point(10000, 0);
	int nbIntersection = 0;
	for (int i = 0; i < polygon.size() - 1; ++i)
	{
		if (IntersectSegment(p, infinity, polygon[i], polygon[i + 1]))
			nbIntersection++;
	}
	return (nbIntersection % 2) == 1;
}


bool IntersectSegment(point a, point b, point c, point d)
{
	point e = point(b.x - a.x, b.y - a.y);
	point f = point(d.x - c.x, d.y - c.y);

	float denom = e.x*f.y - e.y*f.x;
	if (denom == 0)
		return false;
	float t = -(a.x*f.y - c.x*f.y - f.x*a.y + f.x*c.y) / denom;
	if (t<0 || t >= 1)
		return false;
	float u = -(-e.x*a.y + e.x*c.y + e.y*a.x - e.y*c.x) / denom;
	if (u<0 || u >= 1)
		return false;
	return true;
}


void RemplissageLCA(vector<point> polygon, float r, float g, float b)
{
	point RectEG[2];
	RectangleEnglobant(polygon, RectEG);

	int yMin = int(RectEG[0].y) - 2;
	int yMax = int(RectEG[1].y) + 2;

	vector<vector<ElementLCA>> SI(yMax - yMin);

	for (int i = 0; i < polygon.size() - 1; ++i)
	{
		float x;
		float yMin_;
		float yMax_;
		float slope;
		if (polygon[i].y < polygon[i + 1].y)
		{
			x = polygon[i].x;
			yMin_ = int(polygon[i].y);
			yMax_ = int(polygon[i + 1].y);
			slope = (polygon[i + 1].x - polygon[i].x) / (yMax_ - yMin_);
		}
		else if (polygon[i].y > polygon[i + 1].y)
		{
			x = polygon[i + 1].x;
			yMin_ = int(polygon[i + 1].y);
			yMax_ = int(polygon[i].y);
			slope = (polygon[i].x - polygon[i + 1].x) / (yMax_ - yMin_);
		}
		SI[yMin_ - yMin].push_back(ElementLCA(x, yMax_, slope));
	}
	
	vector<ElementLCA> LCA;

	for (int i = 0; i < SI.size(); ++i)
	{
		for (int j = 0; j < SI[i].size(); ++j)
		{
			LCA.push_back(SI[i][j]);
		}

		for (int j = LCA.size() - 1; j > -1; --j)
		{
			if (i + yMin >= LCA[j].yMax)
				LCA.erase(LCA.begin() + j);
			else
				LCA[j].x += LCA[j].slope;
		}

		sort(LCA.begin(), LCA.end());

		if (LCA.size() % 2 == 0)
		{
			int nbCouple = LCA.size() / 2;

			for (int j = 0; j < nbCouple; ++j)
			{
				for (int k = LCA[j * 2].x; k < LCA[(j * 2) + 1].x; ++k)
				{
					glBegin(GL_POINTS);
					glColor3f(r, g, b);
					glVertex2i(k, i + yMin);
					glEnd();
				}
			}
		}
	}
}


#pragma endregion REMPLISSAGE



#pragma region FENETRAGE


bool DetectWindowDirection()
{
	bool isClockwise = false;
	double sum = 0;

	for (int i = 0; i < window.size() - 1; i++)
	{
		sum += (window[i + 1].x - window[i].x) * (window[i + 1].y + window[i].y);
	}

	isClockwise = (sum > 0) ? true : false;
	return isClockwise;
}


bool Coupe(point a, point b, point c, point d)
{
	//matrice 1, matrice inverse.
	float matrice[2][2];
	float matriceInverse[2][2];
	float matriceRes[2];
	float matriceB[2];

	float determinant;

	matrice[0][0] = (b.x - a.x);
	matrice[0][1] = (c.x - d.x);
	matrice[1][0] = (b.y - a.y);
	matrice[1][1] = (c.y - d.y);

	determinant = (matrice[0][0] * matrice[1][1]) - (matrice[0][1] * matrice[1][0]);

	if (determinant == 0)
	{
		return false;
	}

	matriceInverse[0][0] = matrice[1][1] / determinant;
	matriceInverse[0][1] = -matrice[0][1] / determinant;
	matriceInverse[1][0] = -matrice[1][0] / determinant;
	matriceInverse[1][1] = matrice[0][0] / determinant;

	matriceB[0] = (c.x - a.x);
	matriceB[1] = (c.y - a.y);

	matriceRes[0] = matriceInverse[0][0] * matriceB[0] + matriceInverse[0][1] * matriceB[1];
	matriceRes[1] = matriceInverse[1][0] * matriceB[0] + matriceInverse[1][1] * matriceB[1];

	if ((matriceRes[0] > 0 && matriceRes[0] < 1))
	{
		return true;
	}

	return false;
}


point Intersection(point a, point b, point c, point d)
{
	//matrice 1, matrice inverse.
	float matrice[2][2];
	float matriceInverse[2][2];
	float matriceRes[2];
	float matriceB[2];

	float determinant;

	matrice[0][0] = (b.x - a.x);
	matrice[0][1] = (c.x - d.x);
	matrice[1][0] = (b.y - a.y);
	matrice[1][1] = (c.y - d.y);

	determinant = (matrice[0][0] * matrice[1][1]) - (matrice[0][1] * matrice[1][0]);

	matriceInverse[0][0] = matrice[1][1] / determinant;
	matriceInverse[0][1] = -matrice[0][1] / determinant;
	matriceInverse[1][0] = -matrice[1][0] / determinant;
	matriceInverse[1][1] = matrice[0][0] / determinant;

	matriceB[0] = (c.x - a.x);
	matriceB[1] = (c.y - a.y);

	matriceRes[0] = matriceInverse[0][0] * matriceB[0] + matriceInverse[0][1] * matriceB[1];
	matriceRes[1] = matriceInverse[1][0] * matriceB[0] + matriceInverse[1][1] * matriceB[1];

	point i;
	i.x = ((1 - matriceRes[0]) * a.x) + (matriceRes[0] * b.x);
	i.y = ((1 - matriceRes[0]) * a.y) + (matriceRes[0] * b.y);

	return i;
}


bool Visible(point test, point a, point b)
{
	point n;
	if (sensFenetre)
	{
		n.x = b.y - a.y;
		n.y = -(b.x - a.x);
	}
	else
	{
		n.x = a.y - b.y;
		n.y = -(a.x - b.x);
	}

	point vec = point(test.x - a.x, test.y - a.y);

	float t;

	t = (vec.x*n.x) + (vec.y*n.y);

	if (t > 0){
		return true;
	}

	return false;
}


vector<point> AlgoSutherlandHodgman(vector<point> polygonToCut)
{
	vector<point> polygon;
	for (int k = 0; k < polygonToCut.size(); ++k)
	{
		polygon.push_back(polygonToCut[k]);
	}

	vector<point> polygonResult;

	if (polygonToCut.size() == 0)
		return polygonResult;

	point firstPoint;

	for (int i = 0; i < window.size() - 1; ++i)
	{
		polygonResult.clear();

		for (int j = 0; j < polygon.size(); ++j)
		{
			if (j == 0)
			{
				firstPoint = polygon[0];
			}
			else if (Coupe(polygon[j - 1], polygon[j], window[i], window[i + 1]))
			{
				point I = Intersection(polygon[j - 1], polygon[j], window[i], window[i + 1]);
				polygonResult.push_back(I);
			}

			if (Visible(polygon[j], window[i], window[i + 1]))
			{
				polygonResult.push_back(polygon[j]);
			}
		}

		if (polygonResult.size() > 0)
		{
			if (Coupe(polygon[polygon.size() - 1], firstPoint, window[i], window[i + 1]))
			{
				point I = Intersection(polygon[polygon.size() - 1], firstPoint, window[i], window[i + 1]);
				polygonResult.push_back(I);
			}

			polygon.clear();
			for (int k = 0; k < polygonResult.size(); ++k)
			{
				polygon.push_back(polygonResult[k]);
			}
		}
	}

	polygonResult.clear();
	int size = polygon.size();
	for (int k = 0; k < size; ++k)
	{
		polygonResult.push_back(polygon[k]);
	}
	if (polygonResult[0].x != polygonResult[size - 1].x || polygonResult[0].y != polygonResult[size - 1].y)
	{
		polygonResult.push_back(polygonResult[0]);
	}

	return polygonResult;
}


#pragma endregion FENETRAGE
