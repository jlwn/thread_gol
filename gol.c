//
// Zach Lockett-Streiff; Taylor Nation; Jacob Lewin
// Implementation of Conway's Game of Life - Threaded Implementation
//
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>

// NOTES: I don't know if all txt input files work.
//        oscillator, walker, pulsar, and testedges seem to
//        work, but I don't know about grower and gosper.

char* makeBoard(int rows, int cols, FILE* file, int numCoords);
void print(char* arr, int willPrint, int rows, int cols, int iters);
void verifyCmdArgs(int argc, char *argv[]);
int numNeighbors(int xcoord, int ycoord, int rows, int cols, char *board,
                 int numcoords);
char *copyBoard(char *board, int rows, int cols);
void evolve(int x, int y, int rows, int cols, char *newBoard, char *refBoard, 
    char *argv[], int numCoords, int iters);
FILE *openFile(char *filename[]);

char* makeBoard(int rows, int cols, FILE* file, int numCoords){
  /*
   * Purpose: Creates the game board as a 1D array
   * Inputs: Rows & Columns:        rows, cols
   *         Input file:            file
   *         Number of coordinates: numCoords
   *          
   * Returns: Nothing
   */
 
  // Allocate space for array
  char* array = NULL;
  array = (char *)malloc(sizeof(char)*(rows*cols));
  
  // Verify malloc worked
  if (array == NULL) {
    printf("malloc failed");
    exit(1);
  }

  // Initialize elements of board array to '-'
  int i,j,x,y,counter;
  x = 0;
  y = 0;
  counter = 0;
  for(i = 0; i< rows; i++){
  	for(j = 0; j< cols; j++){
  		array[i*rows+j]= '-';
  	 }
  }

  // Read in coordinates to update board to its initial state
  while(counter < numCoords){
    fscanf(file, "%d%d", &x,&y);
	array[x*rows+y]='@';
	counter++;
  }
  
  return array;
}

void print(char* arr, int willPrint, int rows, int cols, int iters) {
  /*
  * Purpose: Prints the current iteration and board
  * Inputs: Game board:                arr
  *         2nd command line argument: willPrint
  *         Rows & Columns:            rows, cols
  *         Number of iterations       iters
  * 
  * Returns: Nothing
  */

  if (!willPrint) {
    return;
  }
  
  usleep(200000);
  system("clear");
  printf("Iteration %d:\n\n",iters); 
  int i;
  for (i = 0; i < rows*cols; i++) {
    printf("%c ",arr[i]);
    if (!((i+1) % cols)) {
      printf("\n");
    }
  }
}

void verifyCmdArgs(int argc, char *argv[]) {
  /*
   * Purpose: Verifies if proper command-line arguments were passed
   * Inputs: Number of command-line arguments: argc
   *         Array of command-line arguments:  argv
   *         
   * Returns: Nothing
   */
  
  // Verify caller passed in 3 command arguments
  if (argc != 3) {
   printf("usage: ./gol configFile printCondition\n");
   exit(1);
  }

  // Verify caller passed in .txt file
  if (!strstr(argv[1],".txt")) {
    printf("Invalid test parameter file, must be a .txt file.\n");
    exit(1);
  }

  // Verify valid printCondition
  if (atoi(argv[2]) != 1 && atoi(argv[2]) != 0) {
    printf("Invalid printCondition, must be either 0 or 1.\n");
    exit(1);
  }
}

int numNeighbors(int xCoord, int yCoord, int rows, int cols, char *board,
                 int numCoords){
  /*
   * Purpose: Finds the number of neighbors of a point on the board
   * Inputs: Coordinates:            xCoord, yCoord
   *         Rows & Columns:         rows, cols
   *         Game board:             board
   *         Number of coordinates:  numCoords
   * Returns: Number of neighbors of (xCoord, yCoord) neighborCounter
   */
  
  int neighborCounter, i, j, k, x, y;
  x = xCoord;
  y = yCoord;
  neighborCounter = 0;
  
  // Locates the current row and determines if it has wrapped
  for(j = -1; j< 2; j++){
    int currentRow;
    currentRow = x+j;
    if(currentRow == rows){
      currentRow = 0;  	  
    }else if(currentRow ==-1){
      currentRow = rows-1;
    }
    
    // Locates the current column and determines if it has wrapped
    for(k = -1;k<2;k++){
      int currentCol;
      currentCol = y +k;
      if(currentCol == cols){
          currentCol = 0;
      }else if(currentCol == -1){
        currentCol = cols-1;
      }  	  
      
      // Increments neighborCounter for each neighbor found
      if(currentRow == x && currentCol == y){
        continue;
      }else if(board[currentRow*rows+currentCol] == '@'){
         neighborCounter++;
      }
    }
  }
  //printf("neighborCounter: %d\n",neighborCounter);
  return neighborCounter;
}

char *copyBoard(char *board, int rows, int cols) {
  /*
   * Purpose: Creates a copy of the board
   * Inputs: Game board:             board
   *         Rows & Columns:         rows, cols
   * Returns: Number of neighbors of (xCoord, yCoord) neighborCounter
   */
  int i;
  char *newBoard = (char *)malloc(sizeof(char)*(rows*cols));

  // Verify malloc worked
  if (newBoard == NULL) {
    printf("malloc failed");
    exit(1);
  }
  
  // Copy values of old array into new array
  for (i = 0; i < rows*cols; i++) {
    newBoard[i] = board[i];
  }
  return newBoard;
}

void evolve(int x, int y, int rows, int cols, char *newBoard, char *refBoard,
    char *argv[], int numCoords, int iters) {
  /*
   * Purpose: Examines the board and applies the rules of the Game of Life
   * Inputs: Coordinates:            x, y
   *         Rows & Columns:         rows, cols
   *         Game board:             board
   *         Command-line arguments: argv[]
   *         Number of coordinates:  numCoords
   *         Number of iterations:   iters
   * Returns: Nothing
   */ 
  
  //char *newBoard = copyBoard(board, rows, cols);
  
  for(x = 0; x < rows; x++) {
    for(y = 0; y < cols; y++) {
      int neighbors = numNeighbors(x, y, rows, cols, refBoard, numCoords);
      //printf("Point (%d,%d) has %d neighbors\n",x,y,neighbors);
      if (neighbors < 0 || neighbors > 8) {
        printf("Invalid number of neighbors. Should be between 0 and 8");
        exit(1);
      }
      if(neighbors < 2){
        newBoard[x*rows+y]= '-';
      
      } else if(neighbors > 3){
          newBoard[x*rows+y] = '-';

      } else if(neighbors == 3){
          newBoard[x*rows+y] = '@';
      
      } else {
          newBoard[x*rows+y] = refBoard[x*rows+y];
      }
        
    }
  }
    print(newBoard,atoi(argv[2]),rows,cols,iters);
}

FILE *openFile(char *filename[]) {
  /*
   * Purpose: Bundles together a few lines for opening the test parameter
   *          file
   * Inputs:  Input file: filename
   * Returns: Opened file: inFile
   */
  FILE *inFile = fopen(filename[1],"r");
  if (inFile == NULL) {
    printf("Unable to load test parameters.\n");
    exit(1);
  }
  return inFile;
}

int main(int argc, char *argv[]) {
  // Variable declarations
  int count = 1;
  int rows,cols,iters,numCoords,x,y,neighbors;
  FILE *inFile = openFile(argv);
  struct timeval start, end;
  char *newBoard = NULL;
  char *refBoard = NULL;
  char *temp = NULL;

  // Process command line arguments
  verifyCmdArgs(argc, argv);

  // Open test parameter file and read in first 4 lines
  fscanf(inFile, "%d %d %d %d", &rows, &cols, &iters, &numCoords);

  // Create game board initialized to starting state
  newBoard = makeBoard(rows,cols,inFile,numCoords);
  refBoard = copyBoard(newBoard,rows,cols);
  print(refBoard,atoi(argv[2]),rows,cols,0);
  //printf("refBoard2: %s\n", refBoard);
  // Apply the life and death conditions to the board
  gettimeofday(&start, NULL);
  while (count < iters+1) {
    evolve(x,y,rows,cols,newBoard,refBoard,argv,numCoords,count);
    /*printf("temBoard: %s\n", temp);
    printf("refBoard: %s\n", refBoard);
    printf("newBoard: %s\n", newBoard);*/
    temp = copyBoard(newBoard,rows,cols); // I think Samer and Manny had a problem with this...
    refBoard = temp; // reference board updated to be the newer board
    //newBoard = temp;
    ++count;
  }
  gettimeofday(&end, NULL);
  
  // Time calculations
  long elapsed = (end.tv_sec-start.tv_sec)*1000000 + (end.tv_usec - 
                  start.tv_usec);
  printf("\nElapsed time for %d steps of a %d x %d board is: %f seconds\n",
                  iters, rows, cols, elapsed/1000000.);

  free(newBoard);
  free(refBoard);
  fclose(inFile);
  refBoard = NULL;
  newBoard = NULL;
  temp = NULL;

  return 0;
}

