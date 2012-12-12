//
// Zach Lockett-Streiff; Taylor Nation; Jacob Lewin
// Implementation of Conway's Game of Life - Threaded Implementation
//
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <pthread.h>

// GLOBAL VARIABLES:
char *newBoard;
char *refBoard;
int rows;
int cols;
struct tid_args *thread_args;
static pthread_mutex_t my_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_barrier_t barrier;
static pthread_barrier_t barrier2;

struct tid_args{
  int my_tid;
  int startRow;
  int endRow;
  int startCol;
  int endCol;
  int willPrint;
  int iter;
};

char* makeBoard(int rows, int cols, FILE* file, int numCoords);
void verifyCmdArgs(int argc, char *argv[]);
int numNeighbors(int xCoord, int yCoord);
char *copyBoard(char *board, int rows, int cols);
void *evolve(void *args);
void print(int willPrint);
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
  rewind(file);
  int a,b,c,d;
  fscanf(file,"%d %d %d %d",&a,&b,&c,&d);
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
  for(i = 0; i < rows; i++){
  	for(j = 0; j < cols; j++){
  		array[i*rows+j]= '-';
  	 }
  }

  // Read in coordinates to update board to its initial state
  while(counter < numCoords){
    if(file == NULL){
      perror("This is the error: ");
      exit(1);
    
    }
    fscanf(file, "%d%d", &x,&y);
	array[x*rows+y]='@';
	counter++;
  }
  
  return array;
}

void print(int willPrint) {
  /*
   *
   *
   * */
  if (!willPrint) {
    return;
  }
  int i;
  for (i = 0; i < rows*cols; i++) {
    printf("%c ",refBoard[i]);
    if (!((i+1) % cols)) {
      printf("\n");
    }
  }
  if (willPrint) {
    printf("\n");
  }
  usleep(500000);
}

void verifyCmdArgs(int argc, char *argv[]) {
  /*
   * Purpose: Verifies if proper command-line arguments were passed
   * Inputs: Number of command-line arguments: argc
   *         Array of command-line arguments:  argv
   *         
   * Returns: Nothing
   */
  
  // Verify caller passed in 6 command arguments
  if (argc != 6) {
   printf("usage: ./gol configFile printCondition numTIDs partition[0:1] print_config[0:1]\n");
   exit(1);
  }

  // Verify valid printCondition
  if (atoi(argv[2]) != 1 && atoi(argv[2]) != 0) {
    printf("Invalid printCondition, must be either 0 or 1.\n");
    exit(1);
  }

  // Verify valid numTIDs
  if (atoi(argv[3]) < 0 || atoi(argv[3]) > 1000) {
    printf("Invalid partition option, must be a positive integer < 1000.\n");
    exit(1);
  }

  // Verify valid partition
  if (atoi(argv[4]) != 0 && atoi(argv[4]) != 1) {
    printf("Invalid partition, must be either 0 or 1\n");
    exit(1);

  }

  // Verify valid print_config option
  if (atoi(argv[5]) != 0 && atoi(argv[5]) != 1) {
    printf("Invalid print_config, must be either 0 or 1\n");
    exit(1);
  }

}

int numNeighbors(int xCoord, int yCoord){
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
      }else if(refBoard[currentRow*rows+currentCol] == '@'){
         neighborCounter++;
      }
    }
  }
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


void *evolve(void *args) {
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

  int x, y, z;
  int start_Row,end_Row,start_Col,end_Col;
  start_Row = ((struct tid_args *)args)->startRow;
  end_Row = ((struct tid_args *)args)->endRow;
  start_Col = ((struct tid_args *)args)->startCol;
  end_Col = ((struct tid_args *)args)->endCol;
  
  // Loop over the specified number of iterations
  for(z = 0; z < ((struct tid_args *)args)->iter; z++) {
    for(x = start_Row; x <= end_Row; x++) {
      for(y = start_Col; y <= end_Col; y++) {
          int neighbors = numNeighbors(x, y);
        // printf("Point (%d,%d) has %d neighbors\n",x,y,neighbors);
        if (neighbors < 0 || neighbors > 8) {
          printf("Invalid number of neighbors. Should be between 0 and 8");
          exit(1);
        }
        //printf("startcol = %d, endcol = %d\n", start_Col, end_Col);
        if(neighbors < 2){
          //printf("x = %d, y = %d\n", x, y);
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
  pthread_barrier_wait(&barrier);
  pthread_barrier_wait(&barrier2);
  }
}

void *update(void *args) {
  /*
   * Purpose: Updates the game board after other threads have evolved
   *          
   * Inputs: Argument struct: *args
   * Returns: Nothing
   */
  int thisIter;
  int printCond = ((struct tid_args *)args)->willPrint;
  char *temp = NULL;
  for(thisIter = 0; thisIter < ((struct tid_args *)args)->iter; thisIter++) {
    pthread_barrier_wait(&barrier);
    temp = refBoard; 
    refBoard = copyBoard(newBoard,rows,cols);
    free(temp);
    if (printCond) { 
      printf("Iteration %d\n",thisIter);
    }
    print(printCond);
    if (printCond) {
    system("clear");
    }
    pthread_barrier_wait(&barrier2);
  }
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
    printf("Invalid test parameter file, must be a .txt file.\n");
    exit(1);
  }
  return inFile;
}


void partition(struct tid_args *thread_args, int numTids, int partitionType){
  /*
   * Purpose: Partitions the board either row-wise or column-wise and assigns
   *          partitions to threads.
   * Inputs:  Arg struct:     *thread_args
   *          # of threads:   numtids
   *          Partition type: partitionType
   * Returns: Nothing
   * */
  int partitions,remainder,i,currentRow;
  currentRow = 0;
  partitions = rows/numTids-1;
  remainder = rows % numTids;
  if(!partitionType){
    for(i=0;i<numTids;i++){
      int startRow,endRow;
	  startRow = currentRow;
	  endRow = currentRow+partitions;
	  if(remainder){
	    endRow++;
	    partitions++;
  	    remainder--;
  	  }
  	  thread_args[i].my_tid = i;
	  thread_args[i].startRow = startRow;
	  thread_args[i].endRow = endRow;
	  thread_args[i].startCol = 0;
	  thread_args[i].endCol = cols-1;
	  currentRow+=partitions+1;
	  partitions = rows/numTids-1;
    }
  }else{    
    partitions = cols/numTids-1;
    int currentCol;
    currentCol = 0;
    for(i=0;i<numTids;i++){
      int startCol,endCol;
	  startCol = currentCol;
	  endCol = currentCol+partitions;
	  if(remainder){
	    endCol++;
	    partitions++;
  	    remainder--;
  	  }
  	  thread_args[i].my_tid = i;
	  thread_args[i].startRow = 0;
	  thread_args[i].endRow = rows-1;
	  thread_args[i].startCol = startCol;
	  thread_args[i].endCol = endCol;
	  currentCol+=partitions+1;
	  partitions = rows/numTids-1;

    }
  }

}

void printPartitions(struct tid_args *thread_args, int tid, int willPrint){
  /*
   * Purpose: prints the partitions
   * Inputs: Arg struct:      *thread_args
   *         Thread ID:       tid
   *         Print Condition: willPrint
   *
   * Returns: Nothing
   * */
  
  if(!willPrint){
    return;
  }else{
      struct tid_args current;
      current = thread_args[tid];
      int tid,startRow,endRow,rowPartSize,startCol,endCol,colPartSize;
      tid = current.my_tid;
      startRow = current.startRow;
      endRow = current.endRow;
      rowPartSize = current.endRow-current.startRow+1;
      startCol = current.startCol;
      endCol = current.endCol;
      colPartSize = current.endCol-current.startCol+1;
      printf("tid %d: rows: %d:%d (%d) cols: %d:%d (%d)\n",tid,startRow,endRow,rowPartSize,
        startCol,endCol,colPartSize);
    }
  
}

int main(int argc, char *argv[]) {
  system("clear");
  
  // Variable declarations
  int count = 1;
  int iters,numCoords,numThreads,printPartition,partitionType,print_alloc;
  struct timeval start, end;
  newBoard = NULL;
  refBoard = NULL;
  char *temp;
  pthread_t *tids;
  verifyCmdArgs(argc, argv);
  FILE *inFile = openFile(argv);
  numThreads = atoi(argv[3]);
  partitionType = atoi(argv[4]);
  printPartition = atoi(argv[2]);
  print_alloc = atoi(argv[5]);
 
  // allocate space for array of pthreads
  if(!(tids = (pthread_t *)malloc(sizeof(pthread_t)*(numThreads+1)))){
    printf("malloc error\n");
    exit(1);
  }
  // allocate space for array of pthread args
  if(!(thread_args = (struct tid_args *)malloc(sizeof(struct tid_args)*(1+numThreads)))){
    printf("malloc error\n");
    exit(1);
  }
  // Initialize barriers
  if(pthread_barrier_init(&barrier,0,numThreads+1)){
    perror("Pthread barrier init error\n");
    exit(1);
  }   
  if(pthread_barrier_init(&barrier2,0,numThreads+1)){
    perror("Pthread barrier2 init error\n");
    exit(1);
  }
  
  // Open test parameter file and read in first 4 lines
  fscanf(inFile, "%d %d %d %d", &rows, &cols, &iters, &numCoords);

  // Create game board initialized to starting state
  newBoard = makeBoard(rows,cols,inFile,numCoords);
  refBoard = copyBoard(newBoard,rows,cols);
  
  // Apply the life and death conditions to the board
  gettimeofday(&start, NULL);
 
  /*
   *  Spawn worker threads
   *  each thread does a round, taking a specified a part of the board
   *    refBoard still passed into evolve, but we're going to be looking at
   *    different portions of it. 
   *
   *    
   */
  partition(thread_args,numThreads,partitionType);

  int i, ret;

  // spawn threads
  for(i = 0; i<numThreads; i++) {
     
     thread_args[i].willPrint = printPartition;
     thread_args[i].iter = iters;
     ret = pthread_create(&tids[i],0,evolve,(void *)&thread_args[i]);
     if(ret){
       perror("Error pthread_create\n");
     }
  }
     i = numThreads;
     thread_args[i].willPrint = printPartition;
     thread_args[i].iter = iters;
     thread_args[i].my_tid = i;
     ret = pthread_create(&tids[i],0,update,(void *)&thread_args[i]);
     if(ret){
       perror("Error pthread_create\n");
     }
  for(i=0; i<numThreads+1;i++) {
     pthread_join(tids[i],0);
  }
  
  gettimeofday(&end, NULL);
  
  // Time calculations
  long elapsed = (end.tv_sec-start.tv_sec)*1000000 + (end.tv_usec - 
                  start.tv_usec);
  printf("Elapsed time for %d steps of a %d x %d board is: %f seconds\n",
                  iters, rows, cols, elapsed/1000000.);

  // Free space
  free(tids);
  free(thread_args);
  free(newBoard);
  free(refBoard);
  fclose(inFile);
  refBoard = NULL;
  newBoard = NULL;
  temp = NULL;

  return 0;
}

