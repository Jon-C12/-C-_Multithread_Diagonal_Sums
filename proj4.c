#include "proj4.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <stdatomic.h>

char * inputFileName; //Global variable that stores name of input file.
pthread_mutex_t matchListMutex = PTHREAD_MUTEX_INITIALIZER; //Mutex for safe access to MatchList struct
atomic_int currentRow = 0; //Atomic variable that tracks current row being processed

void initializeMatchList(MatchList * matchList) {
    matchList->matchCount = 0;
    matchList->matchCapacity = 10;
    matchList->matches = malloc(matchList->matchCapacity * sizeof(Match)); //Allocate memory for matches
    if (!(matchList->matches)) {
      perror("Memory allocation failed for match list"); //Error message if allocation fails
        exit(1);
    }
}

void addMatch(MatchList * matchList, int row, int column) {
  pthread_mutex_lock(&matchListMutex); //Locks mutex for safe access
  if (matchList->matchCount >= matchList->matchCapacity) { //Check if list needs to be resized
        matchList->matchCapacity *= 2;
        matchList->matches = realloc(matchList->matches, matchList->matchCapacity * sizeof(Match));
        if (!(matchList->matches)) {
	  perror("Memory allocation failed while expanding match list"); //Error message if allocation fails
	  exit(1);
        }
    }
  matchList->matches[matchList->matchCount].row = row; //Add row index to new match
  matchList->matches[matchList->matchCount].column = column; //Add column index to new match
  matchList->matchCount++;
  pthread_mutex_unlock(&matchListMutex); //Unlock mutex after updating
}

void freeMatchList(MatchList * matchList) {
  free(matchList->matches); //Free memory allocated for matches
}

void initializeGrid(grid * g, char * fileName) {
  inputFileName = fileName; //Set file name

  FILE * inputFile = fopen(fileName, "r");
  if (!inputFile) {
    perror("Error opening file"); //Error message if file cannot be opened
    exit(1);
  }

  char line[4096];
  unsigned int gridWidth = 0; //Variable to store width
  unsigned int gridHeight = 0; //Variable to store height

  //Determine grid dimenstions by reading file
  while (fgets(line, sizeof(line), inputFile)) {
    unsigned int rowWidth = strlen(line);
    if (line[rowWidth - 1] == '\n') {
      rowWidth--;
    }

    if (gridHeight == 0) { //Set grid width
      gridWidth = rowWidth; 
    }
    gridHeight++;
  }

    //Set grid dimensions and allocate memory for the grid
    (*g).n = gridWidth;
    (*g).p = malloc((*g).n * sizeof(unsigned char *));
    for (unsigned int i = 0; i < (*g).n; i++) {
        (*g).p[i] = malloc((*g).n * sizeof(unsigned char));
    }

    rewind(inputFile); //Reset file to the beginning for reading data

    //Read data into the grid
    for (unsigned int row = 0; row < (*g).n; row++) {
      if (fgets(line, sizeof(line), inputFile)) {
	for (unsigned int column = 0; column < (*g).n; column++) {
	  (*g).p[row][column] = line[column] - '0';
	}
      }
    }
    fclose(inputFile);
}

void * threadFunction(void * args) {
  ThreadArguments * threadArgs = (ThreadArguments *)args; //
  unsigned int gridSize = threadArgs->gridSize; //Obtain grid size
  int tileSize = 32; //Define tile size for proccessing data efficiently
  int startRow;
  
  //Process rows
  while ((startRow = atomic_fetch_add(&currentRow, tileSize)) < threadArgs->endRow) {
    for (int row = startRow; row < startRow + tileSize && row < threadArgs->endRow; row++) {
      for (unsigned int col = 0; col < gridSize; col++) {
	unsigned long mainDiagonalSum = 0, antiDiagonalSum = 0;
	int foundMainMatch = 0;
	int foundAntiMatch = 0;

	unsigned int maxMainLength = threadArgs->mainLengths[row][col]; //Max length for diagonal
	unsigned int maxAntiLength = threadArgs->antiLengths[row][col]; //Max length for anti-diagonal

	//Compute main diagonal sums
	for (unsigned int k = 0; k < maxMainLength && !foundMainMatch; k++) {
	  mainDiagonalSum += threadArgs->inputGrid[row + k][col + k];
	  if (mainDiagonalSum == threadArgs->targetSum) {
	    for (unsigned int i = 0; i <= k; i++) {
	      threadArgs->outputGrid[row + i][col + i] =
		threadArgs->inputGrid[row + i][col + i];
	    }
	    foundMainMatch = 1; //Match found
	  }
	}
	//Compute anti-diagonal sums
	for (unsigned int k = 0; k < maxAntiLength && !foundAntiMatch; k++) {
	  antiDiagonalSum += threadArgs->inputGrid[row + k][col - k];
	  if (antiDiagonalSum == threadArgs->targetSum) {
	    for (unsigned int i = 0; i <= k; i++) {
	      threadArgs->outputGrid[row + i][col - i] =
		threadArgs->inputGrid[row + i][col - i];
	    }
	    foundAntiMatch = 1; //Match found
	  }
	}
      }
    }
  }
  return NULL;
}

void * writeChunk(void * args) {
  WriteArguments * writeArgs = (WriteArguments *)args; //Cast args to WriteArguments
  char * localBuffer = malloc(writeArgs->gridSize * writeArgs->gridSize + writeArgs->gridSize); //Allocate buffer for grid being written

  if (!localBuffer) {
    perror("Memory allocation failed for write buffer"); //Error message if allocation fails
    pthread_exit(NULL);
  }

  char * ptr = localBuffer;
  for (unsigned int row = writeArgs->startRow; row < writeArgs->endRow; row++) {
    for (unsigned int col = 0; col < writeArgs->gridSize; col++) { //Convert grid value and store
      *ptr++ = writeArgs->grid[row][col] + '0';
    }
    *ptr++ = '\n';
  }

  fwrite(localBuffer, 1, ptr - localBuffer, writeArgs->file); //Write buffer contents to desired file
  free(localBuffer); //Free allocated buffer
  return NULL;
}

void parallelWrite(unsigned char ** grid, unsigned int gridSize, char * fileName, int threadCount) {
  FILE * outputFile = fopen(fileName, "w"); //Open output file for writing

  if (!outputFile) {
    perror("Error opening output file"); //Error message if file opening fails
    exit(1);
  }

  pthread_t threads[threadCount]; 
  WriteArguments writeArgs[threadCount];
  int rowsPerThread = (gridSize + threadCount - 1) / threadCount; //Rows per thread calculation

  for (int i = 0; i < threadCount; i++) { //Create threads to write portions of grid
    writeArgs[i] = (WriteArguments){
      .file = outputFile, 
      .grid = grid,
      .startRow = i * rowsPerThread,
      .endRow = (i + 1) * rowsPerThread > gridSize ? gridSize : (i + 1) * rowsPerThread,
      .gridSize = gridSize};

    pthread_create(&threads[i], NULL, writeChunk, &writeArgs[i]); //Thread initialization
  }

  for (int i = 0; i < threadCount; i++) { 
    pthread_join(threads[i], NULL); //Join threads
  }

  fclose(outputFile);
}

    

void writeGrid(grid * output, char * fileName) {
  FILE * outputFile = fopen(fileName, "w"); //Open output file for writing

  if (!outputFile) {
    perror("Error opening output file"); //Error message if opening file fails
    exit(1);
  }

  for (unsigned int row = 0; row < (*output).n; row++) {
    for (unsigned int col = 0; col < (*output).n; col++) {
      fputc((*output).p[row][col] + '0', outputFile); //Write grid value as a character
    }
    fputc('\n', outputFile);
  }

  fclose(outputFile);

  //Free allocated memory
  if ((*output).p) {
    free((*output).p[0]); //Free block of memory
    free((*output).p); //Free the array of row pointers
    (*output).p = NULL; 
  }
}

void precomputeDiagonalLengths(unsigned int gridSize, unsigned int ** mainLengths, unsigned int ** antiLengths) {
  for (unsigned int row = 0; row < gridSize; row++) {
    for (unsigned int col = 0; col < gridSize; col++) {
      mainLengths[row][col] = gridSize - (row > col ? row : col);
      //Compute main diagonal length (top-left - bottom-right) 
      antiLengths[row][col] = gridSize - (row > (gridSize - 1 - col) ? row : (gridSize - 1 - col));
      //Compute anti-diagonal length (top-right - bottom-left)
    }
  }
}

void diagonalSums(grid * input, unsigned long targetSum, grid * output, int threadCount) {
    (*output).n = (*input).n;

    //Allocate memory for the output grid
    unsigned char * outputData = calloc((*output).n * (*output).n, sizeof(unsigned char));

    if (!outputData) {
      perror("Memory allocation failed for outputData"); //Error message if allocation fails
      exit(1);
    }

    (*output).p = malloc((*output).n * sizeof(unsigned char *));
    if (!(*output).p) {
      perror("Memory allocation failed for output pointers"); //Error message if allocation fails
      free(outputData); //Free allocaed data
      exit(1);
    }

    for (unsigned int row = 0; row < (*output).n; row++) { //Initialize row of pointers for ouput grid
        (*output).p[row] = outputData + row * (*output).n;
    }

    //Allocate memory for diagonal lengths
    unsigned int ** mainLengths = malloc((*input).n * sizeof(unsigned int *));
    unsigned int ** antiLengths = malloc((*input).n * sizeof(unsigned int *));
    for (unsigned int i = 0; i < (*input).n; i++) {
        mainLengths[i] = malloc((*input).n * sizeof(unsigned int));
        antiLengths[i] = malloc((*input).n * sizeof(unsigned int));
    }
    precomputeDiagonalLengths((*input).n, mainLengths, antiLengths); //Compute diagonal lengths for grid

    //Multithreading setup
    pthread_t threads[threadCount];
    ThreadArguments threadArgsArray[threadCount];

    atomic_store(&currentRow, 0); //Initialize shared counter

    for (int i = 0; i < threadCount; i++) {
    threadArgsArray[i] = (ThreadArguments){
        .targetSum = targetSum,
        .startRow = 0,
        .endRow = (*input).n,
        .gridSize = (*input).n,
        .inputGrid = (*input).p,
        .outputGrid = (*output).p,
        .mainLengths = mainLengths, //Pass precomputed main diagonal lengths
        .antiLengths = antiLengths  //Pass precomputed anti-diagonal lengths
    };

    pthread_create(&threads[i], NULL, threadFunction, &threadArgsArray[i]); //Start thread
}

    for (int i = 0; i < threadCount; i++) {
      pthread_join(threads[i], NULL); //Join threads
    }

    //Free diagonal lengths allocated memory
    for (unsigned int i = 0; i < (*input).n; i++) {
      free(mainLengths[i]);
      free(antiLengths[i]);
    }
    free(mainLengths); //Free array of main diagonal pointers
    free(antiLengths); //Free array of anti-diagonal pointers
}

void freeGrid(grid * g) {
  if ((*g).p != NULL) { //Check for allocated grid pointers
    for (unsigned int row = 0; row < (*g).n; row++) {
      free((*g).p[row]); //Free each row of the grid
    }
    free((*g).p); //Free array of row pointers
    (*g).p = NULL;
  }
}
