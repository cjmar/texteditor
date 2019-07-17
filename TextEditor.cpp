/*
	Author: Cody Marshall
	www.github.com/texteditor

	Simple text editor using GLUT
	Made as project 2 for Computer Graphics class
	Features:
		-edit text
		-click into text
		-blinking cursor
		-save to file (hard coded to c:/temp/saved.txt)
		-each letter can have its own font
		-each letter can have its own color
		-options changed via right click menu

	NOTE:
		Code has been removed from this file which was done by a project partner. 
		The only feature this code did was a seperate help window which could be
		closed using a menu item or key press. No other item has been touched post
		grading, the grade recieved was 100%.

	63~ characters long
	40~ rows
*/

#include <iostream>
#include <fstream>
#include <GL/glut.h>
#include <string>
#include <vector>
#include <fstream>

/*
63~ characters long
40~ rows
*/



//Some structs

/*
Global settings, applies fonts and colors to typed letters
X, Y is based on column and row, NOT x, y (world) position
It is converted using a method
*/
struct settings
{
	//This is color in byte format, values are from {0, 0, 0} to {255, 255, 255}
	int color[3];	//default color of black {0,0,0} 
	void* font = GLUT_BITMAP_8_BY_13;
	//Global x,y coord for editing and drawing caret
	int X = 0;
	int Y = 1;
};

/*
Object for each character typed
Gives each character a unique color and font it can have
*/
struct character
{
	char letter;
	int color[3];	//default color of black {0,0,0}
	void* font = GLUT_BITMAP_8_BY_13;
};

/*
text is a vector of characters, basically a string
rowSize is a counter for how many characters in each row
rowSize is the basis for printing and some bounds checking
*/
struct screenText
{
	std::vector<character> text;	//Text to be drawn
	std::vector<int> rowSize;		//Char count for each row
};

//Globals
screenText text;			//Text on screen
int mainWindow;				//main window ID
settings setting;			//setting object
bool blink = true;			//blink for caret

//prototypes
void backspace();
void keyEntered(unsigned char);
int getIndex();

//#########################################################################

/*
Saves the current text to C:/temp/
Keeps row numbers and text together
#########################################################################
*/
void saveText()
{
	std::ofstream fout;
	fout.open("C:\\temp\\typed.txt");
	if (!fout.is_open())
	{
		std::cout << "Cannot open or create file: C:\temp\typed.txt\n";
		fout.close();
		exit(0);
	}

	int count = 0;
	for (int row : text.rowSize)
	{
		for (int i = 0; i < row; i++)
		{
			character c = text.text.at(count);
			fout << c.letter;
			count++;
		}
		fout << char(13);
	}

	fout.close();
}

void drawPoints()
{
	//Draws each character object in text vector
	int x_coord = 0;
	int y_coord = 0;
	int count = 0;

	for (int row : text.rowSize)
	{
		for (int i = 0; i < row; i++)
		{
			character c = text.text.at(count);
			glColor3ub(c.color[0], c.color[1], c.color[2]);
			glRasterPos2f(x_coord - 195, 285 - y_coord);
			glutBitmapCharacter(c.font, c.letter);
			x_coord += 6;	//6 pixels between each character
			//This might need to be modifed to work with larger fonts
			count++;
		}
		y_coord += 15;
		x_coord = 0;
	}

	if (blink) //Draw the caret
	{
		//13 tall, 1 wide line
		glLineWidth(2);
		glColor3f(0, 0, 0);
		glEnable(GL_LINES);

		glBegin(GL_LINES);

		//Offsets the caret to the right. Might need to be modified for larger fonts
		int x_value = (setting.X + 2) * 6 - 205;

		glVertex2f(x_value, 300 - (setting.Y * 15) - 1);
		glVertex2f(x_value, 300 - (setting.Y * 15) + 13);

		glEnd();
	}
}


/*
Row/Column selecting from mouse clicking
Turns mouse x,y coordinates into row/column of strings and string vector
##########################################################################
*/
int getRow(int x_in)
{
	int value = ceil((x_in - 7) / 6);
	//The 6 here is based on the 6 pixels between each letter
	//Again it might need to be modifed for larger fonts

	if (value > text.rowSize.at(setting.Y - 1))
	{
		value = text.rowSize.at(setting.Y - 1);
	}

	return value;
}
int getCol(int y_in)
{
	unsigned int value = ceil((y_in + 7) / 15);

	//Bounds checking. Only up to 40 lines are displayed
	if (value > 39)
	{
		value = 39;
	}
	if (value < 1)
	{
		value = 1;
	}

	return value;
}


/*
Callback for mouse button click
###########################################################################
*/
void mouseButtonDown(int buttonName, int state, int x_in, int y_in)
{
	if (state == GLUT_DOWN)
	{
		switch (buttonName)
		{
		case GLUT_LEFT_BUTTON:
			setting.Y = getCol(y_in);
			setting.X = getRow(x_in);
			break;
		}
	}

}

/*
Display callback for the main window
############################################################################
*/
void mainDisplayCallback()
{
	glClear(GL_COLOR_BUFFER_BIT);	// draw the background

	drawPoints();

	glFlush(); // flush out the buffer contents
}

/*
Keyboard callback
Adds character to current row
############################################################################
*/
void myKeyboardEvent(unsigned char key, int cursorX, int cursorY)
{
	switch (key)
	{
	case 13:		//Enter key pressed
		setting.X = 0;
		if (setting.Y < 39)
		{
			setting.Y++;
		}
		else
		{
			setting.Y = 39;
		}

		break;

	case 8:			//backspace key pressed
		backspace();
		break;

	default:		//Regular key pressed
		keyEntered(key);
		break;
	}
}

/*
New character added event
############################################################################
*/
void keyEntered(unsigned char key)
{
	character newChar;
	newChar.letter = key;
	//Set color and font to the current setting
	newChar.color[0] = setting.color[0];
	newChar.color[1] = setting.color[1];
	newChar.color[2] = setting.color[2];
	newChar.font = setting.font;

	bool addKey = true;
	//If the new character overruns the set size of row, start on next row
	if (text.rowSize.at(setting.Y - 1) == 63)
	{
		//Check if last row is empty
		if (text.rowSize.at(38) == 0)	//if its empty
		{
			//erase last row
			text.rowSize.erase(text.rowSize.begin() + 38);
			//insert new one below line being edited
			text.rowSize.insert(text.rowSize.begin() + setting.Y, 0);
			setting.X = 0;
			setting.Y++;
		}
		else if (setting.Y != 39 && text.rowSize.at(setting.Y) == 0) //else if the line below is empty
		{
			text.rowSize.erase(text.rowSize.begin() + setting.Y);
			text.rowSize.insert(text.rowSize.begin() + setting.Y, 0);
			setting.X = 0;
			setting.Y++;
		}
		//If creating a new row goes over the 40 row limit, text page is full?
		else
		{
			addKey = false;
		}
	}

	if (addKey)
	{
		text.text.insert(text.text.begin() + getIndex(), newChar);
		setting.X++;
		text.rowSize.at(setting.Y - 1)++;
	}
}

/*
Backspace key event
############################################################################
*/
void backspace()
{
	//If caret isnt all the way to left, and not in last row
	if (setting.X != 0 && text.text.size() != 0)
	{
		text.text.erase(text.text.begin() + getIndex() - 1);
		text.rowSize.at(setting.Y - 1)--;
		//Bound check for X position to stop at 0
		if (setting.X > 0)
		{
			setting.X--;
		}
	}
	//Else its last row or left most character of current row
	else
	{
		//Check if its the very top row, if not go up one row
		if (setting.Y > 1)	//setting Y starts at 1
		{
			setting.Y--;
			setting.X = text.rowSize.at(setting.Y - 1);
		}
		else //No line above this one, so we're at the very top
		{
			setting.X = 0;
		}
	}
}

/*
Arrow keys to move cursor around
############################################################################
*/
void specialKeyboardEvent(int key, int x_in, int y_in)
{
	//All the X values are set using the rowSize vector. They're already bounds checked
	switch (key)
	{
	case GLUT_KEY_UP:	//Goes to next row up if applicable
		if (setting.Y > 1)
		{
			setting.Y--;
			setting.X = text.rowSize.at(setting.Y - 1);
		}
		else	//At top, do nothing
		{
			setting.Y = 1;
			setting.X = text.rowSize.at(setting.Y - 1);
		}
		break;

	case GLUT_KEY_DOWN:	//Goes to next row down if applicable
		if (setting.Y < 39)
		{
			setting.Y++;
			setting.X = text.rowSize.at(setting.Y - 1);
		}
		else
		{
			setting.Y = 39;
			setting.X = text.rowSize.at(setting.Y - 1);
		}
		break;

	case GLUT_KEY_RIGHT: //Moves cursor right if not at max length
		if (setting.X < text.rowSize.at(setting.Y - 1))
		{
			setting.X++;
		}
		else
		{
			setting.X = text.rowSize.at(setting.Y - 1);
		}

		break;

	case GLUT_KEY_LEFT:	//Moves cursor left if not at 0
		if (setting.X > 0)
		{
			setting.X--;
		}
		else
		{
			setting.X = 0;
		}
		break;
	}
}

/*
Display refreshing, ~60fps
Auto updates changes etc
############################################################################
*/
void timeFunc(int id_in)
{
	mainDisplayCallback();
	glutTimerFunc(15, timeFunc, 1);
}

//Menu callback for the right click menu
//############################################################################
void mainMenuCallback(int id_in)
{

	switch (id_in)
	{
	case 0:	//Change color to blue
		setting.color[0] = 0;
		setting.color[1] = 0;
		setting.color[2] = 255;
		break;

	case 1: //Change color to green
		setting.color[0] = 0;
		setting.color[1] = 255;
		setting.color[2] = 0;
		break;

	case 2:	//CHange font to Helvetica
		setting.font = GLUT_BITMAP_HELVETICA_12;
		break;

	case 3: //Change font to new times roman
		setting.font = GLUT_BITMAP_TIMES_ROMAN_10;
		break;

	case 4: //Exit program
		exit(0);
		break;

	case 5:
		break;

	case 6: //Change oclor to black
		setting.color[0] = 0;
		setting.color[1] = 0;
		setting.color[2] = 0;
		break;

	case 7: //Change font to glut 8x13
		setting.font = GLUT_BITMAP_8_BY_13;
		break;

	case 8: //Save to .txt file
		saveText();
		break;
	}
}


/*
Timer for caret, half a second
############################################################################
*/
void drawCaret(int id_in)
{
	blink = !blink;
	glutTimerFunc(500, drawCaret, 1);
}

/*
Returns the index of the current setting.X, setting.Y in the text string
For selecting a letter based on X, Y
############################################################################
*/
int getIndex()
{
	//add up number of characters before the selected one
	int index = 0;
	int current = text.rowSize.at(setting.Y - 1);
	for (int i = 0; i < setting.Y - 1; i++)
	{
		index += text.rowSize.at(i);
	}
	index += setting.X;

	return index;
}

//main
//############################################################################
int main()
{
	//Glut setup main window
	glutInitWindowSize(400, 600);			// specify a window size
	glutInitWindowPosition(100, 0);			// specify a window position
	mainWindow = glutCreateWindow("Editor");// create a titled window

	glClearColor(1, 1, 1, 0);				// specify a background clor: white 
	gluOrtho2D(-200, 200, -300, 300);		// specify a viewing area

	//Callbacks
	glutDisplayFunc(mainDisplayCallback);	// register a callback
	glutKeyboardFunc(myKeyboardEvent);		// Keyboard input callback
	glutTimerFunc(17, timeFunc, 1);			// 59~ frames per second
	glutTimerFunc(1000, drawCaret, 1);
	glutMouseFunc(mouseButtonDown);
	glutSpecialFunc(specialKeyboardEvent);

	//Right click menu items
	int colorMenu = glutCreateMenu(mainMenuCallback);
	glutAddMenuEntry("Blue", 0);
	glutAddMenuEntry("Green", 1);
	glutAddMenuEntry("Black", 6);

	int fontMenu = glutCreateMenu(mainMenuCallback);
	glutAddMenuEntry("Helvetica", 2);
	glutAddMenuEntry("Times Roman", 3);
	glutAddMenuEntry("Glut 8x13", 7);	//GLUT_BITMAP_8_BY_13

	//Main menu
	glutCreateMenu(mainMenuCallback);
	glutAddSubMenu("Color", colorMenu);
	glutAddSubMenu("Font", fontMenu);

	glutAddMenuEntry("Save", 8);
	glutAddMenuEntry("Exit", 4);

	glutAttachMenu(GLUT_RIGHT_BUTTON);

	//Populate rowSize with length 0 characters
	for (int i = 0; i < 41; i++)
	{
		text.rowSize.push_back(0);
	}

	glutMainLoop();

	return 0;
}
