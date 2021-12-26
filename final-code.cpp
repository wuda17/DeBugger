// clang++ -pthread -std=c++17 -o DeBugger team07-final-code-DeBugger.cpp
#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <cctype>
#include <limits>

using namespace std;

//Global Variables
const int FLAGGED = -1;
const int HIDDEN = 0;
const int REVEALED = 1;

const int BUG = 9; //Set to 9 as the maximum number of Bugs surrounding a square is 8

//Memory management (prevent from making too big of a game)
const int MAXDIMENSIONS = 9;


struct square
{
  //-1 Is Flagged, 0 is Hidden, 1 is Revealed
  int state = 0;
  
  //9 for bug
  int neighbors = 0;
};

typedef vector<square> squareVector;
typedef vector<squareVector> squareMatrix;

//Function Declarations
squareMatrix createBoard(int rows, int cols);
void computeNeighbors(squareMatrix & matrix, int rows, int cols);
void hideBoard(squareMatrix & matrix, int rows, int cols);
void cleanBoard(squareMatrix & matrix);
void printBoard(squareMatrix matrix, int rows, int cols);
int reveal(squareMatrix & matrix, int rows, int cols, int rowGuess, int colGuess);
int mark(squareMatrix & matrix, int rows, int cols, int rowGuess, int colGuess);
bool isGameWon(squareMatrix matrix, int rows, int cols);

bool game();
void startGame(squareMatrix & matrix, int & rows, int & cols, unsigned int & numBugs);

char getAction();
void actionShow(squareMatrix & matrix, int & rows, int & cols, unsigned int & numBugs);
void actionMark(squareMatrix & matrix, int rows, int cols);
void placeMines(squareMatrix & matrix, int rows, int cols, unsigned int numBugs);


// Creates and returns the default starting game board

squareMatrix createBoard(int rows, int cols)
{
  squareMatrix matrix;
  square tempSquare {.state = HIDDEN, .neighbors = 0}; 

  squareVector tempRow;// = squareVector();
  for (int i = 0; i < rows; i ++) {
    for (int j = 0; j < cols; j++) { //
      // set every square to 'default' state
      tempRow.push_back(tempSquare);
    }
    matrix.push_back(tempRow);
    tempRow.clear();
  }
  //cout << "Creating Board..." << endl;
  return matrix;
}

// Iterates through game board and sets all the squares to their corresponding number (if they are not a bug)
// If a bug is encountered in the matrix, increment the number on the squares within a 1 square radius of the bug
void computeNeighbors(squareMatrix & matrix, int rows, int cols)
{
  int bugRow{0};
  int bugCol {0};

  for (int i = 0; i < rows; i ++) {
    for (int j = 0; j < cols; j++) {
      
      if (matrix[i][j].neighbors == 9)
      {
        bugRow = i;
        bugCol = j;

        //An error we must consider is that our second nested for loop iterates in a 1-square radius around each bug, however if the square is out of the contraints of the board we cannot count it (otherwise we get a segmentation fault). This is similar to the reveal function so we have to make sure the 

        //goes around bug in a square including itself
        for (int indexRowofIncrement = bugRow-1; indexRowofIncrement <= bugRow+1; indexRowofIncrement++) 
        {
          for (int indexColofIncrement = bugCol-1; indexColofIncrement <= bugCol+1; indexColofIncrement++)
          {
            //Check if the square is in the board first. Using the same logic as in reveal() function. Check that function for the full logic. 
            if ((indexRowofIncrement > -1 && indexColofIncrement > -1) && (indexRowofIncrement < rows && indexColofIncrement < cols))
            {
              if (matrix[indexRowofIncrement][indexColofIncrement].neighbors != 9) //to negate the bug in the middle and any other Bugs in surrounding 1 radius
              {
                matrix[indexRowofIncrement][indexColofIncrement].neighbors += 1;
              }
            }
          }
        }
        bugRow = 0;
        bugCol = 0;
      }
    }
  }
}

//Needed for startGame and when game is restarted reset all of the states to hidden as we want none of the squares shown to the user
void hideBoard(squareMatrix &matrix, int rows, int cols)
{
  for (int i = 0; i < rows; i ++) {
    for (int j = 0; j < cols; j++)
    {
      matrix[i][j].state = HIDDEN;
    }
  }
}

void cleanBoard(squareMatrix & matrix)
{
  matrix.clear();
}


void printBoard(squareMatrix matrix, int rows, int cols) {
  
  //Printing out column labels
  for (int i = 0; i < cols; i++) {
    if (i == 0) {
      cout << "   " << i + 1;
    } else{
      cout << "  " << i + 1;
    }

    if (i == cols-1) {
      cout << endl;
    }
  }

  //Iterate through the board and print out based on the square state (coloured the bugs/flags as red and numbers as blue.)
  for (int i = 0; i < rows; i++) {
    
    //Print out row labels
    cout << i + 1 << " ";
    for (int j = 0; j < cols; j++) {

      if (matrix[i][j].state == FLAGGED)
      {
        cout << "\033[1;31m" << " M " << "\033[0m";
      }
      else if (matrix[i][j].state == HIDDEN)
      {
        cout << " * " ; 
      }
      else if(matrix[i][j].state == REVEALED && matrix[i][j].neighbors == 9)
      {
        cout << "\033[1;31m" << " B " << "\033[0m";
      }
      //Else: Revealed and show neighbors
      else
      {
        cout << "\033[1;34m" << " " << matrix[i][j].neighbors << " " << "\033[0m";
      }
    }
    cout  << endl;
  }
}

int reveal(squareMatrix & matrix, int rows, int cols, int rowGuess, int colGuess)
{
  /*We need to ensure the user input for ROW/COL = 1 is the 0th index in the matrix since it starts at one
      * * * * * <- row 0 (user input row = 1)
      * * * * *
      ^
      col 0 (user input column = 1)

  */


  //cout << "Starting to Reveal..." << endl;
  int rowGuessIndex = rowGuess - 1; 
  int colGuessIndex = colGuess - 1;
  
  int stateRevealedSquare = matrix[rowGuessIndex][colGuessIndex].state;
  int neighborsRevealedSquare = matrix[rowGuessIndex][colGuessIndex].neighbors;

  if (stateRevealedSquare == FLAGGED)
  {
    return 1;
  }
  else if (stateRevealedSquare == REVEALED)
  {
    return 2;
  }
  else if (neighborsRevealedSquare == 9) // revealing bug
  {
    return 9;
  }
  //else if (stateRevealedSquare == HIDDEN && neighborsRevealedSquare > 0) 
  //{
  //stateRevealedSquare = REVEALED;

  //  return 0;
  //}

  // Edge cases where if revealing on the edge of the matrix we want to not attempt to access memory outside of our matrix
  else //if (stateRevealedSquare == HIDDEN && neighborsRevealedSquare == 0)
  {
    /*
    If the revealed square has a zero: this counts basically as a useless move. In traditional minesweeper the games loops recursively, checking the adjacent squares and if they are '0', it reveals the square. Continuing until all of the edges of the revealed area are surrounded by non-zero squares

          In our case, we wanted to just reveal in a 1-square radius around the '0' square, (since these squares are all guaranteed to be not Bugs)
          * * * 
          * 0 *
          * * * 
    
    EDGE CASES
    In this edge case, we are on the edges/corners of the board so we dont want to reach outside of our board matrix

    rowGuess is the y coordinate in the grid that the user is guessing
    colGues is the x coordinate in the grid that the user is guessing

    if x is the number of rows, then x-1 is the last row
    if rowGuess is the last row, rowGuess = x

    which means row - rowGuess = -1 if its at the end of the grid

    for any other VALID case, then row - rowGuess > -1 

    THIS IS THE SAME FOR COLUMNS

    */
    //cout << "Revealing square..." << endl;

    if (neighborsRevealedSquare == 0)
    {
      stateRevealedSquare = REVEALED;

      for (int rowToReveal = rowGuessIndex-1; rowToReveal <= rowGuessIndex+1; rowToReveal++)
      {
        for (int colToReveal = colGuessIndex-1; colToReveal <= colGuessIndex+1; colToReveal++)
        {
          //cout << "Checking around the square" << endl;

          /*Check for if its out of reach in HERE instead
          
          Although the only invalid minimum inputs for row and column guess are 1, and therefore rowGuess-1/colGuess-1 = 0 (since we checked for valid user input which is >1, we want to check for all potential problematic cases if there is any chance that the user gave input of 0) 
          
          This only reveals if the square around the 0-square is within the board*/

          if ((rowToReveal > -1 && colToReveal > -1) && (rowToReveal <= rows-1 && colToReveal <= cols-1))
          {
            matrix[rowToReveal][colToReveal].state = REVEALED;
            //cout << "This square has been revealed" << endl;
          }
        }
      }
    }
    else if (neighborsRevealedSquare != 0)
    {
      matrix[rowGuessIndex][colGuessIndex].state = REVEALED;
    } 
    return 0;  
  }
}


int mark(squareMatrix &matrix, int rows, int cols, int rowGuess, int colGuess)
{
  int rowGuessIndex = rowGuess - 1;
  int colGuessIndex = colGuess - 1;

  int stateSquareToReveal = matrix[rowGuessIndex][colGuessIndex].state;
  int neighborsSquareToReveal = matrix[rowGuessIndex][colGuessIndex].neighbors;

  if (stateSquareToReveal == FLAGGED)
  {
    //Acessing the state itself in the board matrix is required, we can't use the stateSquareReveal variable here since it has a static value.
    matrix[rowGuessIndex][colGuessIndex].state = HIDDEN;
    //Line
    return 0;
  }
  else if (stateSquareToReveal != FLAGGED && stateSquareToReveal != HIDDEN)
  {
    return 2;
  }
  else
  {
    matrix[rowGuessIndex][colGuessIndex].state = FLAGGED;
    // cout << "state of square: " << stateSquareToReveal << endl;
    return 0;
  }
}


/*

  isGameWon FUNCTION

  The main objective of this game is to not detonate or flag all the correct location of the bugs.

  There are a few cases that need to be
  checked to determine whether the game has been won.

  CASES:

  1. To win the game the player must have flagged the correct locations of the 'bugs,' otherwise the
    player didn't properly identify where the bugs are. If there are any non bug squares that are flagges, the player has not won yet and must continue the game.

  2. In this case, not all non-bugged were shown. However, if all the bugs are correctly flagged, the game is won.

  3. In this case, all the non-bugged squares have been revealed. The bugs are the only squares that are hidden but they aren't necessarily flagges, the game is still won.

  4. The last case to check is if the Bugs are all flagges AND all the neutral squares have been revealed. This case also indicates the game has been won. 

  If none of these cases are satisfies, this still means the game has not been won yet, and therefore the player will have to continue to play.
*/

bool isGameWon(squareMatrix matrix, int rows, int cols)
{
  //These are our truth values on each possible win condition
  bool unmasked{true};
  bool marked{true};
  bool masked{false};

  //Checking each square in the board (each element of the 2D matrix) and iterating checking the state and neighbors. The win states will be set to false if any square in the board violates the win condition. 
  for (int i = 0; i < rows; i++)
  {
    for (int j = 0; j < cols; j++)
    {
      int squareState = matrix[i][j].state;
      int squareNeighbors = matrix[i][j].neighbors;

      if (squareNeighbors == BUG)
      {
        if (squareState != FLAGGED)
        {
          marked = false;
        }
      }
      else if (squareNeighbors != BUG)
      {
        if (squareState == HIDDEN)
        {
          unmasked = false;
        }
        else if (squareState == FLAGGED)
        { 
          masked = true;
        }
      }
    }
  }
  
  //Win Conditions
  if (masked == true)
      return false;
  else if (unmasked == false && marked == true){
      return true;
  }
  else if (unmasked == true && marked == false){
      return true;
  }
  else if (unmasked == true && marked == true){
    return true;
  }    
  else{
    return false;
  }
}

// runs the game after every move
bool game() {
  // getActions
  int rows{0};
  int cols{0};
  unsigned int numBugs{0};
  squareMatrix matrix;

  startGame(matrix, rows, cols, numBugs);
  
  char currAction{0};
  while(currAction != 'Q')
  {
    switch (currAction)
    {
      //User input to show the square
      case 'S' :
      {
        //Indirection that starts the actionShow function and runs from there
        actionShow(matrix, rows, cols, numBugs);
        break;
      }
      //Flag
      case 'M' :
      {
        //Indirection that starts the actionMark function and runs from there
        actionMark(matrix, rows, cols);
        break;
      }
      //Restart Game
      case 'R' :
      {
        //Indirection that starts the startGame function outside of the game function to allow specifically for multiple games without rerunning the program
        cout << "Restarting the game." << endl;
        startGame(matrix, rows, cols, numBugs);
        break;
      }
    }
    //print to screen
    printBoard(matrix, rows, cols);
    
    //If game is won: reveal the board and print to screen
    if (isGameWon(matrix, rows, cols))
    {
      cout << "You have revealed all the fields without running into a bug!" << endl;
      cout << "YOU WON!!!" << endl;
      for (int revealRow = 0; revealRow < rows; revealRow++)
      {
        for (int revealCol = 0; revealCol < cols; revealCol++)
        {
          matrix[revealRow][revealCol].state = REVEALED;
        }
      }
      printBoard(matrix, rows, cols);
      
      //Print State
      cout << "Resetting the game board." << endl;
      startGame(matrix, rows, cols, numBugs);
      printBoard(matrix, rows, cols);
    }
    currAction = getAction();
  }

  //Clear the board
  cleanBoard(matrix);
  //Game is still running if restart
  return true;  
}

// helper function to see if string can be converted into an int
bool isInt(string& s)
{
    string::const_iterator it = s.begin();
    while (it != s.end() && isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

// starts the game
//Pass by reference: need to be able to change the variables
void startGame (squareMatrix & matrix, int & rows, int & cols, unsigned int & numBugs) {
  
  cout << "Welcome to DeBugger" << endl;
  cout << "Reveal the whole board and try to avoid the bugs!" << endl;
  cout << "THERE ARE 2 TYPES OF TILES:" << "\t" << "COVERED" << "\t" << "and" << "\t" << "UNCOVERED" << endl;
  cout << "COVERED:" << "\t" << "*" << endl;
  cout<<"UNCOVERED ARE NUMBERED:" << "\t" << "1,2,3,4,5,6,7,8" << endl;
  cout << "A ROWxCOL BOARD WILL BE PRESENTED, WHICH, INITIALLY HAS ALL TILES COVERED. YOUR TASK IS TO UNCOVER ALL TILES WITHOUT BUGS." << endl;
  cout<<"YOU HAVE TO ENTER ROW AND COL OF A PARTICULAR TILE TO UNCOVER THAT TILE.\n";
  cout<<"EACH NUMBERED TILE REPRESENTS THE NUMBER OF BUGS ADJACENT TO THAT NUMBERED TILE.\n";

  string tempCol;
  string tempRow;
  string tempBugs;

  /*
  When checking input of the users there are several exceptions that need to be
  considered.

  1. Is user doesn't input an integer
  2. If the input is an integer but is out of bounds

  An error message will be displayed to the user and they will have to retry their input. This is done in a do-while loop so the input message will continuously be
  iterated through until the proper input type is received.

  */


  do {
      cout << "Please enter the x dimension (max " << MAXDIMENSIONS << "): ";
      cin >> tempCol;

      if (!isInt(tempCol)) {
        cout << "Input was not an integer. Please try again." << endl;
      } else {
        cols = stoi(tempCol);
      }
      if (cols > MAXDIMENSIONS) {
        cout << "Dimensions are too large." << endl;
      }

  } while( !isInt(tempCol) || (cols < 1 || cols > MAXDIMENSIONS));

  do {
    cout << "Please enter the y dimension (max " << MAXDIMENSIONS << "): ";
    cin >> tempRow;

    if (!isInt(tempRow)) {
      cout << "Input was not an integer. Please try again." << endl;
    } else {
      rows = stoi(tempRow);
    }
    if (rows > MAXDIMENSIONS) {
      cout << "Dimensions are too large." << endl;
    }

  } while(!isInt(tempRow) || (rows < 1 || rows > MAXDIMENSIONS));
  
  do {
    cout << "Please enter the number of bugs: ";
    cin >> tempBugs;

    if (!isInt(tempBugs)) {
      cout << "Input was not an integer. Please try again." << endl;
    } else {
      numBugs = stoi(tempBugs);
    }
    if (numBugs < 1) {
      cout << "Please enter at least 1 bug on the board" << endl;

    } else if (numBugs > cols * rows ) {
      cout << "That's too many bugs!" << endl;
    }

  } while((!isInt(tempBugs) || numBugs < 1) || numBugs > cols * rows ); 
 

  
  //Debugging print statements
  //cout << "The Game is Starting..." << endl;
  cleanBoard(matrix);
  matrix = createBoard(rows, cols);
  placeMines(matrix, rows, cols, numBugs);
  //Debugging print statements
  //cout << "Mines have been placed" << endl;
  computeNeighbors(matrix, rows, cols);
  hideBoard(matrix, rows, cols);
}

char getAction() {
  // take in user input (show, mark, rest, quit)
  char action;

  // By using a do-while loop, it will continuously ask the user to re-input their command if the input they provided was not one of the possible moves (S, M, R or Q)

  // the user, however, should be able to use both upper or lowercase letters as their command therefore
  // this function should be able to take in both possibilities of input as commands.
  do {
    cout << "Please enter your move ([S]how, [M]ark, [R]estart, [Q]uit): ";
    cin >> action;

    //cout << "This is your action: " << action << endl;
    // we need to make sure that the lower/uppercase status of the user's input does not affect the user's move command
    if (islower(action)) {
      action = toupper(action);
    }

    if ((action != 'S' && action != 'M') && (action != 'R' && action != 'Q')) {

      cout << "Input was not valid. Please try again" << endl;
    }

  }  while ((action != 'S' && action != 'M') && (action != 'R' && action != 'Q'));

  return action;
}

void actionShow(squareMatrix & matrix, int & rows, int & cols, unsigned int & numBugs) {
  //shows the square
  int rowToReveal{0};
  int colToReveal{0};
  
  cout << "Please enter the row coordinate to show: ";
  cin >> rowToReveal;
  cout << "Please enter the column coordinate to show: ";
  cin >> colToReveal;
  
  int rowIndexToReveal = rowToReveal-1;
  int colIndexToReveal = colToReveal-1;

  int stateSquareToReveal = matrix[rowIndexToReveal][colIndexToReveal].state;
  
  // Out of bounds
  if (rowToReveal > rows || colToReveal > cols)
  {
    cout << "Location entered is not on the matrix." << endl;
  }
  else if (stateSquareToReveal == FLAGGED) // checks if location is marked
  {
    cout << "Location is marked, and therefore cannot be revealed." << endl;
    cout << "Use Mark on location to unmark." << endl;
  }
  else if (stateSquareToReveal == REVEALED)
  {
    cout << "You have already revealed this square!" << endl;
    cout << "Try another square." << endl;
  }

  // checking this conditional automatically calls the reveal function (and thus reveals the square if no errors arise)
  else if (reveal(matrix, rows, cols, rowToReveal, colToReveal) == 9) 
  {
    // checks if location to reveal is a bug
    cout << "You hit a mine! Your game has ended." << endl;
    // set the state of all squares to revealed to reveal the entire board to user 

    for (squareVector &row: matrix) {
      for (square &col : row) {
        col.state = REVEALED;
      }
    }
    printBoard(matrix, rows, cols);
    cout << "Starting a new game." << endl;
    startGame(matrix, rows, cols, numBugs);
  }
}

void actionMark(squareMatrix & matrix, int rows, int cols)
{
  // marks the square
  int rowMark;
  int colMark;
  
  cout << "Please enter the row coordinate to mark: ";
  cin >> rowMark;
  cout << "Please enter the column coordinate to mark: ";
  cin >> colMark;
  
  // out of bounds
  if (rowMark > rows|| colMark > cols)
  {
    cout << "Location entered is not on the board." << endl;
  }
   // running the mark function here follows the same logic as the reveal function in actionReveal
  else if (mark(matrix, rows, cols, rowMark, colMark) == 2)
  // marks square, and checks if alread marked
  {
    cout << "Position already revealed. Cannot be marked." << endl;
  }
}

void placeMines(squareMatrix & matrix, int rows, int cols, unsigned int numBugs){

  if (static_cast<int>(matrix.size()) != 0)
   {
      //Checking how many Bugs we have placed in total
      int BugsPlacedAlready{0}; 

      //Coordinates of current bug we are placing
      int rowOfbug;
      int colOfbug;
      default_random_engine generator;
      uniform_int_distribution<int> randomrow(0,rows-1);
      uniform_int_distribution<int> randomcol(0,cols-1);
      
      //cout << "Placing Bugs..." << endl;

      while (BugsPlacedAlready < numBugs)
      {
        rowOfbug = randomrow(generator);
        colOfbug = randomcol(generator);

        //Debugging print out the coordinates of the bugs
        //cout << rowOfbug << colOfbug << endl;

        matrix[rowOfbug][colOfbug].neighbors = BUG;
        BugsPlacedAlready += 1;
      }
   }
}

int main ()
{
  game();
  return 0;
}
