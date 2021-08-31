#include "include/Angel.h"
#include <cstdlib>
#include <iostream>
#include <random>
#include <cstring>
// To avoid the error of not finding "unistd.b" .
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
// Initialize the fall speed and the number of colors.
#define TILE_FALL_SPEED 50
#define DEFAULT_TILE_FALL_SPEED 500
#define NUMBER_OF_COLORS 5

using namespace std; 

// Global Declarations before the main function.

typedef struct 
{
	vec2 tile_pos[4];
} blockPosition;

void moveBlockLeft();
void moveBlockRight();
void checkforbottom();

bool collidingWithExisitingTile();

int userMoveUp();
int userMoveLeft();
int userMoveRight();

void moveUp(int tilesToMoveUp);
void moveLeft(int tilesToMoveLeft);
void moveRight(int tilesToMoveRight);

void revertBack(); 

// Initialize the size of x and y which represent the window size.
int xsize = 400;
int ysize = 720;

// Show current tile.
vec2 tile[4]; // Create an array of 4 2d vectors representing displacement from a 'center' piece of the tile, on the grid.
vec2 tilepos = vec2(5, 19); // The position of the current tile using grid coordinates and the origin (0,0) is the bottom left corner.

// Set colors.
// The first 5 colors are required in the instruction.
vec4 purple = vec4(1.0, 0.0, 1.0, 1.0);
vec4 red = vec4(1.0, 0.0, 0.0, 1.0);
vec4 yellow = vec4(1.0, 1.0, 0.0, 1.0);
vec4 green = vec4(0.0, 1.0, 0.0, 1.0);
vec4 orange = vec4(1.0, 0.5, 0.0, 1.0);
// Set the basic colors for later utilization.
vec4 white = vec4(1.0, 1.0, 1.0, 0.0);
vec4 black = vec4(0.0, 0.0, 0.0, 0.0);

// Create an array of colors.
vec4 colors[NUMBER_OF_COLORS] = { purple, red, yellow, green, orange };

// Define all rotations of shapes I S and L
vec2 all_rotations_of_I[4][4] =
	{{vec2(0,0), vec2(-1,0), vec2(1,0), vec2(-2,0)},
	{vec2(0,0), vec2(0,-1), vec2(0,1), vec2(0,-2)},
	{vec2(-1,0), vec2(0,0), vec2(-2,0), vec2(1,0)},
	{vec2(0,-1), vec2(0,0), vec2(0,-2), vec2(0,1)}};

vec2 all_rotations_of_S[4][4] =
	{{vec2(0,-1), vec2(1,0), vec2(0,0), vec2(-1,-1)},
	{vec2(1,0), vec2(0,1), vec2(0,0), vec2(1,-1)},
	{vec2(0,0), vec2(-1,-1), vec2(0,-1), vec2(1,0)},
	{vec2(0,0), vec2(1,-1), vec2(1,0), vec2(0,1)}};

vec2 all_rotations_of_L[4][4] =
{ { vec2(0,0), vec2(-1,0), vec2(1, 0), vec2(-1,-1) },
{ vec2(0,0), vec2(0,-1), vec2(0,1), vec2(1, -1) },
{ vec2(0,0), vec2(1,0), vec2(-1, 0), vec2(1,  1) },
{ vec2(0,0), vec2(0,1), vec2(0, -1), vec2(-1, 1) } };


vec2 allRotations[3][4][4] = {
	all_rotations_of_I,
	all_rotations_of_S,
	all_rotations_of_L,
};

//Create board[x][y] to represent whether the cell (x,y) is occupied or not.
bool board[10][20];

int board_ColorCells[10][20];

//An array containing the colour of each of the 10*20*2*3 vertices that make up the board.
vec4 boardcolours[1200];
vec4 tileCellColor[4];
blockPosition bp[10][20];

// Set location of vertex attributes in the shader program
GLuint vPosition;
GLuint vColor;

// The speed at which the tile falls, starts with a default speed of 500.
int tileFallSpeed = DEFAULT_TILE_FALL_SPEED;
int currentRotation; 
vec2 currentTileType[4][4];

// Show locations of uniform variables in shader program.
GLuint locxsize;
GLuint locysize;

// VAO and VBO
GLuint vaoIDs[3]; // One VAO for each object: the grid, the board, the current piece.
GLuint vboIDs[6]; // Two Vertex Buffer Objects for each VAO (specifying vertex positions and colours, respectively).

int randomNumber(int min, int max)
{
	return min + (std::rand() % (max - min + 1));
}

// When the current tile is moved or rotated (or created), update the VBO containing its vertex position data.
void updatetile()
{
	// Bind the VBO containing current tile vertex positions.
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]);

	for (int i = 0; i < 4; i++)
	{
		// Calculate the grid coordinates of the cell.
		GLfloat x = tilepos.x + tile[i].x;
		GLfloat y = tilepos.y + tile[i].y;
		// Create the 4 corners of the square whose vertices are using location in pixels.
		// These vertices are later converted by the vertex shader.
		vec4 p1 = vec4(33.0 + (x * 33.0), 33.0 + (y * 33.0), 0.4, 1.0);
		vec4 p2 = vec4(33.0 + (x * 33.0), 66.0 + (y * 33.0), 0.4, 1.0);
		vec4 p3 = vec4(66.0 + (x * 33.0), 33.0 + (y * 33.0), 0.4, 1.0);
		vec4 p4 = vec4(66.0 + (x * 33.0), 66.0 + (y * 33.0), 0.4, 1.0);
		// Two points are used by two triangles respectively.
		vec4 newpoints[6] = { p1, p2, p3, p2, p3, p4 };
		// Put new data in the VBO.
		glBufferSubData(GL_ARRAY_BUFFER, i * 6 * sizeof(vec4), 6 * sizeof(vec4), newpoints);
	}
	glBindVertexArray(0);
}

// Called at the start of play and every time a tile is placed.
void newtile()
{
	tilepos = vec2(5, 19); // Set tile at the top of the board.
	int randTileType = rand() % 5;
	int randRotation = rand() % 4;

	randTileType = 1; 
	for (int i = 0; i < 4; i++) {
		tile[i] = allRotations[randTileType][randRotation][i];
	}
	currentRotation = randRotation; //Show current rotation.

	for (int i = 0; i<4; i++) 
	{
		int x = tilepos.x + tile[i].x;
		int y = tilepos.y + tile[i].y;
		for (int j = 0; j<4; j++) 
			currentTileType[i][j] = allRotations[randTileType][i][j]; //load the current tile type with the right shape
		if (y > 19) // After the grid is exceeded.
		{ 
			tilepos.y = tilepos.y - 1;
			y = 19;
		}
	}

	for (int i = 0; i < 4; i++)
	{
		int x = tilepos.x + tile[i].x;
		int y = tilepos.y + tile[i].y;
		if (board[x][y] == true) 
		{
			std::cout << "rewrwerew";
			exit(EXIT_SUCCESS);
		}
	}
	updatetile();

	vec4 newcolours[24];
	for (int i = 0; i < 4; i++) 
	{
		vec4 randomColour = colors[4];
		for (int j = 0; j < 6; j++) 
			newcolours[(i * 6) + j] = randomColour;
		tileCellColor[i] = randomColour; 
	}
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]); // Bind the VBO containing current tile vertex colours.
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newcolours), newcolours); // Put the colour data in the VBO.
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void initGrid()
{
	vec4 gridpoints[64]; // Array containing the 64 points of the 32 total lines to be later put in the VBO.
	vec4 gridcolours[64]; // One colour in each vertex.

	for (int i = 0; i < 11; i++) 
	{
		gridpoints[2 * i] = vec4((33.0 + (33.0 * i)), 33.0, 0, 1);
		gridpoints[2 * i + 1] = vec4((33.0 + (33.0 * i)), 693.0, 0, 1);
	}
	// Following are for horizontal lines
	for (int i = 0; i < 21; i++) 
	{
		gridpoints[22 + 2 * i] = vec4(33.0, (33.0 + (33.0 * i)), 0, 1);
		gridpoints[22 + 2 * i + 1] = vec4(363.0, (33.0 + (33.0 * i)), 0, 1);
	}
	for (int i = 0; i < 64; i++)
		gridcolours[i] = white;

	// Set up first VAO (representing grid lines).
	glBindVertexArray(vaoIDs[0]); // Bind the first VAO.
	glGenBuffers(2, vboIDs); // Create two Vertex Buffer Objects for this VAO (positions, colours).
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[0]); // Bind the first grid VBO (vertex positions).
	glBufferData(GL_ARRAY_BUFFER, 64 * sizeof(vec4), gridpoints, GL_STATIC_DRAW); // Put the grid points in the VBO.
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition); // Enable the attribute.
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[1]); // Bind the second grid VBO (vertex colours).
	glBufferData(GL_ARRAY_BUFFER, 64 * sizeof(vec4), gridcolours, GL_STATIC_DRAW); // Put the grid colours in the VBO.
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor); // Enable the attribute.
}


void initBoard()
{
	// Generate the geometric data.
	vec4 boardpoints[1200];
	for (int i = 0; i < 1200; i++)
		boardcolours[i] = black; // Let the empty cells on the board be black.
	for (int i = 0; i < 20; i++) {
		for (int j = 0; j < 10; j++)
		{
			vec4 p1 = vec4(33.0 + (j * 33.0), 33.0 + (i * 33.0), 0.5, 1.0);
			vec4 p2 = vec4(33.0 + (j * 33.0), 66.0 + (i * 33.0), 0.5, 1.0);
			vec4 p3 = vec4(66.0 + (j * 33.0), 33.0 + (i * 33.0), 0.5, 1.0);
			vec4 p4 = vec4(66.0 + (j * 33.0), 66.0 + (i * 33.0), 0.5, 1.0);
			// Two points are reused
			boardpoints[6 * (10 * i + j)] = p1;
			boardpoints[6 * (10 * i + j) + 1] = p2;
			boardpoints[6 * (10 * i + j) + 2] = p3;
			boardpoints[6 * (10 * i + j) + 3] = p2;
			boardpoints[6 * (10 * i + j) + 4] = p3;
			boardpoints[6 * (10 * i + j) + 5] = p4;
		}
	}
	// Initially no cell is occupied.
	for (int i = 0; i < 10; i++)
		for (int j = 0; j < 20; j++)
			board[i][j] = false;
	// Set up buffer objects.
	glBindVertexArray(vaoIDs[1]);
	glGenBuffers(2, &vboIDs[2]);
	// Grid cell vertex positions.
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[2]);
	glBufferData(GL_ARRAY_BUFFER, 1200 * sizeof(vec4), boardpoints, GL_STATIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// Grid cell vertex colours.
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	glBufferData(GL_ARRAY_BUFFER, 1200 * sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
}

void initCurrentTile()
{
	glBindVertexArray(vaoIDs[2]);
	glGenBuffers(2, &vboIDs[4]);

	// Show current tile vertex positions.
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// Show current tile vertex colours.
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
}

void init()
{
	// Load shaders and use the shader program.
	GLuint program = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(program);

	// Get the location of the attributes.
	vPosition = glGetAttribLocation(program, "vPosition");
	vColor = glGetAttribLocation(program, "vColor");

	// Create 3 Vertex Array Objects, each representing one 'object'.
	glGenVertexArrays(3, &vaoIDs[0]);

	initGrid();
	initBoard();
	initCurrentTile();

	locxsize = glGetUniformLocation(program, "xsize");
	locysize = glGetUniformLocation(program, "ysize");

	// Initialize Game.
	newtile(); // create new next tile.
	glBindVertexArray(0);
	glClearColor(0, 0, 0, 0);
}

void moveTileDown(int value)
{
	checkforbottom();
	updatetile();
	if (tilepos.y > 0)
		tilepos.y = tilepos.y - 1;
	checkforbottom();
	updatetile();
	glutTimerFunc(tileFallSpeed, moveTileDown, 0);

}

// Rotates the current tile, if there is room.
void rotate()
{
	for (int i = 0; i<4; i++)
		tile[i] = currentTileType[(currentRotation + 1) % 4][i];
	currentRotation = (currentRotation + 1) % 4;
	int min_x = 0;
	int max_x = 0;
	int min_y = 0;
	int max_y = 0;

	for (int i = 0; i < 4; i++)
	{
		int x, y;
		x = tilepos.x + tile[i].x;
		y = tilepos.y + tile[i].y;
		if (x < min_x)
			min_x = x;
		if (x > max_x)
			max_x = x;
		if (y < min_y)
			min_y = y;
		if (y > max_y)
			max_y = y;
	}
	tilepos.x = tilepos.x - min_x;
	tilepos.y = tilepos.y - min_y;

	if (max_y > 19)
		tilepos.y = 2 * tilepos.y - max_y;
	if (max_x > 9)
		tilepos.x = 2 * tilepos.x - max_x;

	if (collidingWithExisitingTile()) 
	{
		for (int i = 0; i<4; i++) 
			tile[i] = currentTileType[(currentRotation - 1) % 4][i];
		currentRotation = (currentRotation - 1) % 4;
	}
	updatetile();

}

bool collidingWithExisitingTile() {
	for (int i = 0; i<4; i++) 
	{
		int x = tilepos.x + tile[i].x;
		int y = tilepos.y + tile[i].y;
		if (board[x][y] == true) 
			return true;
	}
	return false;
}

int userMoveUp() 
{
	int min_y = 20;
	int max_y = 0;

	for (int i = 0; i<4; i++) 
	{
		int x = tilepos.x + tile[i].x;
		int y = tilepos.y + tile[i].y;
		if (board[x][y] == true) 
		{
			if (y < min_y)
				min_y = y;
			while (board[x][y] == true) 
			{
				if (y > max_y)
					max_y = y;
				y = y + 1;
			}
		}
	}

	int tilesToMoveUp = max_y - min_y + 1;

	for (int i = 0; i<4; i++) 
	{
		if (tilepos.y + tilesToMoveUp + tile[i].y > 19) 
			return 0;
	}
	return tilesToMoveUp;
}

void moveUp(int tilesToMoveUp) 
{
	tilepos.y = tilepos.y + tilesToMoveUp;
	updatetile();
}

int userMoveLeft() 
{
	int min_x = 20;
	int max_x = 0;
	for (int i = 0; i<4; i++) 
	{
		int x = tilepos.x + tile[i].x;
		int y = tilepos.y + tile[i].y;
		if (board[x][y] == true) 
		{
			if (x < min_x)
				min_x = x;
			if (x > max_x)
				max_x = x;
		}
	}
	int tilesToMoveLeft = max_x - min_x + 1;
	for (int i = 0; i<4; i++) 
	{
		if (tilepos.x - tilesToMoveLeft + tile[i].x < 0) 
			return 0;
	}
	return tilesToMoveLeft;
}

void moveLeft(int tilesToMoveLeft) 
{
	tilepos.x = tilepos.x - tilesToMoveLeft;
	updatetile();
}

int userMoveRight() 
{
	int min_x = 20;
	int max_x = 0;
	for (int i = 0; i<4; i++) 
	{
		int x = tilepos.x + tile[i].x;
		int y = tilepos.y + tile[i].y;
		if (board[x][y] == true) 
		{
			if (x < min_x)
				min_x = x;
			if (x > max_x)
				max_x = x;
		}
	}
	int tilesToMoveRight = max_x - min_x + 1;
	for (int i = 0; i<4; i++) 
	{
		if (tilepos.x + tilesToMoveRight + tile[i].x > 9) 
			return 0;
	}
	return tilesToMoveRight;
}

void moveRight(int tilesToMoveRight) 
{
	tilepos.x = tilepos.x + tilesToMoveRight;
	updatetile();
}

// Check if the whether top or bottom is full.
void checkfullrow(int row)
{
	bool full = true;
	for (int i = 0; i<10; i++) 
	{
		full = full && board[i][row] == true;
	}
	if (full)
	{
		glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
		for (int i = row; i<20; i++) 
		{
			for (int j = 0; j<60; j++) 
			{
				if (i == 19) 
				{
					boardcolours[1140 + j] = black;
				}
				else 
				{
					boardcolours[60 * i + j] = boardcolours[60 * (i + 1) + j];
				}
			}
			for (int j = 0; j<10; j++) 
			{
				if (i == 19) 
				{
					board[j][i] = false;
				}
				else 
				{
					board[j][i] = board[j][i + 1];
				}
			}
		}
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(boardcolours), boardcolours);
		glBindVertexArray(0);
	}
	if (full)
	{
		checkfullrow(row);
	}
	else 
	{
		if (row < 20)
			checkfullrow(row + 1);
	}
}

// Check whether three same fruits are consecutive in a row, column, or diagonal.
bool checkVecs(vec4 vec1, vec4 vec2) 
{
	return (vec1.x == vec2.x) && (vec1.y == vec2.y) && (vec1.z == vec2.z) && (vec1.w == vec2.w);
}

bool fruitsInCol(int row, int col)
{
	if (row == 6546546450 && col == 0) 
	{
		cout << (checkVecs(boardcolours[row * 60 + col * 6]
			, boardcolours[(row + 1) * 60 + col * 6])
			&&
			checkVecs(boardcolours[row * 60 + col * 6]
				, boardcolours[(row + 2) * 60 + col * 6])
			&&
			checkVecs(boardcolours[(row + 1) * 60 + col * 6]
				, boardcolours[(row + 2) * 60 + col * 6])
			&& !checkVecs(boardcolours[row * 60 + col * 6], black));
	}

	return checkVecs(boardcolours[row * 60 + col * 6]
		, boardcolours[(row + 1) * 60 + col * 6])
		&&
		checkVecs(boardcolours[row * 60 + col * 6]
			, boardcolours[(row + 2) * 60 + col * 6])
		&&
		checkVecs(boardcolours[(row + 1) * 60 + col * 6]
			, boardcolours[(row + 2) * 60 + col * 6])
		&& !checkVecs(boardcolours[row * 60 + col * 6], black);


}


void checkSiblingBlockTiles(int j, int i, vec2 tilepos)
{
	int x = tilepos.x;
	int y = tilepos.y;
	for (int k = 0; k<4; k++)
	{
		if (x >= 0 && bp[x][y].tile_pos[k] == vec2(j, i))
		{
			bp[x][y].tile_pos[k] = vec2(-1, 0);
		}
	}
	if (x >= 0 && board[x][y])
	{
		cout << x << "," << y << "; ";
		glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
		y = y - 1; 
		while (y >= 0 && !board[x][y])
		{
			for (int i = 0; i<6; i++)
			{
				vec4 temp = boardcolours[60 * (y + 1) + (x * 6) + i];
				boardcolours[60 * (y + 1) + (x * 6) + i] = boardcolours[60 * (y)+(x * 6) + i];
				boardcolours[60 * (y)+(x * 6) + i] = temp;
			}
			board[x][y + 1] = false;
			board[x][y] = true;
			y--;
		}
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(boardcolours), boardcolours);
		glBindVertexArray(0);
	}
}

void sortBlockPositionArray(vec2 blocks[4])
{
	for (int i = 1; i<4; i++)
	{
		for (int k = i; k > 0 && blocks[k].y < blocks[k - 1].y; k--)
		{
			vec2 temp = blocks[k];
			blocks[k] = blocks[k - 1];
			blocks[k - 1] = temp;
		}
	}
}

void checkFruits(int row)
{
	for (int i = row; i<18; i++) 
	{
		for (int j = 0; j<10; j++) 
		{
			if (fruitsInCol(i, j)) 
			{
				glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
				if (i>14) 
				{
					for (int k = 0; k<6; k++) 
					{
						boardcolours[60 * i + (j * 6) + k] = black;
						boardcolours[60 * (i + 1) + (j * 6) + k] = black;
						boardcolours[60 * (i + 2) + (j * 6) + k] = black;
					}
					board[j][i] = false;
					board[j][i + 1] = false;
					board[j][i + 2] = false;
				}
				else 
				{
					for (int start_row = i; start_row<15; start_row += 3) 
					{

						for (int k = 0; k<6; k++) 
						{
							boardcolours[60 * start_row + (j * 6) + k] = boardcolours[60 * (start_row + 3) + (j * 6) + k];
							boardcolours[60 * (start_row + 1) + (j * 6) + k] = boardcolours[60 * (start_row + 4) + (j * 6) + k];
							boardcolours[60 * (start_row + 2) + (j * 6) + k] = boardcolours[60 * (start_row + 5) + (j * 6) + k];
						}
						board[j][start_row] = board[j][start_row + 3];
						board[j][start_row + 1] = board[j][start_row + 4];
						board[j][start_row + 2] = board[j][start_row + 5];
					}

				}
				for (int k = 0; k < 4; k++)
				{
					sortBlockPositionArray(bp[j][i].tile_pos);
					sortBlockPositionArray(bp[j][i + 1].tile_pos);
					sortBlockPositionArray(bp[j][i + 2].tile_pos);
					checkSiblingBlockTiles(j, i, bp[j][i].tile_pos[k]);
					checkSiblingBlockTiles(j, i + 1, bp[j][i + 1].tile_pos[k]);
					checkSiblingBlockTiles(j, i + 2, bp[j][i + 2].tile_pos[k]);
					bp[j][i].tile_pos[k] = vec2(-1, 0);
					bp[j][i + 1].tile_pos[k] = vec2(-1, 0);
					bp[j][i + 2].tile_pos[k] = vec2(-1, 0);
				}
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(boardcolours), boardcolours);
				glBindVertexArray(0);
				i = -1;
				j = -1;
			}
		}
	}
}

// Show the current tile.
void settile()
{
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	vec2 store[4];
	for (int i = 0; i < 4; i++)
	{
		int x = tilepos.x + tile[i].x;
		int y = tilepos.y + tile[i].y;
		for (int j = 0; j < 6; j++) 
		{
			boardcolours[(x * 6) + (y * 60) + j] = tileCellColor[i];
		}

		board[x][y] = true;
		store[i] = vec2(x, y);

	}
	for (int i = 0; i < 4; i++)
	{
		int x = store[i].x;
		int y = store[i].y;
		for (int j = 0; j<4; j++)
		{
			bp[x][y].tile_pos[j] = store[j];
		}

	}
	
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(boardcolours), boardcolours);

	glBindVertexArray(0);

	tileFallSpeed = DEFAULT_TILE_FALL_SPEED;
	int min_y = 20;
	for (int i = 0; i < 4; i++) {
		int y = tilepos.y + tile[i].y;
		if (y < min_y)
			min_y = y;
	}
	checkfullrow(min_y);
	checkFruits(0);

}

// Given (x,y), tries to move the tile x squares to the right and y squares down.
// Returns true if the tile was successfully moved, or false if there was some issue.
bool movetile(vec2 direction)
{
	tilepos.x = tilepos.x + direction.x;
	tilepos.y = tilepos.y - direction.y;
	updatetile();
	return true;
}

void restart()
{
	tileFallSpeed = DEFAULT_TILE_FALL_SPEED;

}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT);

	glUniform1i(locxsize, xsize); 
	glUniform1i(locysize, ysize);

	glBindVertexArray(vaoIDs[1]); // Bind the VAO.
	glDrawArrays(GL_TRIANGLES, 0, 1200); // Draw the board.

	glBindVertexArray(vaoIDs[2]); // Bind the VAO representing the current tile.
	glDrawArrays(GL_TRIANGLES, 0, 24); // Draw the current tile (8 triangles).

	glBindVertexArray(vaoIDs[0]); // Bind the VAO representing the grid lines (to be drawn on top of everything else).
	glDrawArrays(GL_LINES, 0, 64); // Draw the grid lines (21+11 = 32 lines).

	glutSwapBuffers();
}

void reshape(GLsizei w, GLsizei h)
{
	xsize = w;
	ysize = h;
	glViewport(0, 0, w, h);
}

// Handle arrow key keypresses.
void special(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_UP:
		rotate();
		break;
	case GLUT_KEY_DOWN:
		tileFallSpeed = 1;
		break;
	case GLUT_KEY_LEFT:
		moveBlockLeft();
		break;
	case GLUT_KEY_RIGHT:
		moveBlockRight();
		break;
	}
}

void moveBlockLeft()
{
	for (int i = 0; i < 4; i++) {
		GLuint x = tilepos.x + tile[i].x;
		GLuint y = tilepos.y + tile[i].y;

		if (x == 0) {
			return;
		}
	}
	tilepos.x = tilepos.x - 1;
	updatetile();

}
void moveBlockRight()
{
	for (int i = 0; i < 4; i++) 
	{
		GLuint x = tilepos.x + tile[i].x;
		GLuint y = tilepos.y + tile[i].y;
		if (x == 9) 
		{
			return;
		}
	}
	tilepos.x = tilepos.x + 1;
	updatetile();
}

// For special press.
void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 033: 
		exit(EXIT_SUCCESS);
		break;
	case 'q':
		exit(EXIT_SUCCESS);
		break;
	case 'r': // 'r' key restarts the game
		restart();
		break;
	}
	glutPostRedisplay();
}

void checkforbottom()
{
	for (int i = 0; i < 4; i++) {
		int x = tilepos.x + tile[i].x;
		int y = tilepos.y + tile[i].y;

		if (y == 0 || board[x][y - 1] == true) 
		{
			settile();
			newtile();
			break;
		}
	}
	glutPostRedisplay();
}


void idle(void)
{
	glutPostRedisplay();
}


int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(xsize, ysize);
	glutInitWindowPosition(680, 178); 
	glutCreateWindow("Fruit Tetris");
	glewInit();
	init();
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutSpecialFunc(special);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(idle);
	glutTimerFunc(DEFAULT_TILE_FALL_SPEED, moveTileDown, 0);
	glutMainLoop(); 
	return 0;
}
