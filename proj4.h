#ifndef PROJ4_H
#define PROJ4_H

#include <pthread.h> 
#include <stdio.h>   
#include <stdlib.h>  
#include <stdbool.h>  

//Added structs from original proj4.h

/*
 * The WriteArugments struct is used for all the
 * the necessary data required for writing operations in 
 * the source code. The fields: file, grid, startRow, endRow,
 * and gridSize all help write the grids for our
 * grid-based program.
 */
typedef struct {
    FILE * file;
    unsigned char ** grid;
    unsigned int startRow;
    unsigned int endRow;
    unsigned int gridSize;
} WriteArguments;

/*
 * The Match struct is used to represent specific positions
 * within our grid. This struct is useful for tracking and/or
 * identifying specific parts of our grid.
 */
typedef struct {
    int row;
    int column;
} Match;

/*
 * The MatchList struct is used to manage the dynamic list of 
 * match objects and efficient store and modify grid positions.
 * The fields matches, matchCount, and matchCapacity provide a
 * flexible way to handle collections of grid locations while
 * also supporting dynamic resizing when needed.
 */
typedef struct {
    Match * matches;
    size_t matchCount;
    size_t matchCapacity;
} MatchList;

/*
 * The ThreadArguments struct is used to store all data required
 * for a thread to process grids and compute the necessary diagnol sums.
 * All the mentioned fields in the struct allow each thread to work on 
 * specific portions of the given grids while sharing necessary data
 * for cumulative calculations and output storage.
 */
typedef struct {
    unsigned long targetSum;
    MatchList * outputMatches;
    int startRow;
    int endRow;
    unsigned int gridSize;
    unsigned char ** inputGrid;
    unsigned char ** outputGrid;
    unsigned long ** cumulativeMain;
    unsigned long ** cumulativeAnti;
    unsigned int ** mainLengths;
    unsigned int ** antiLengths;
} ThreadArguments;


//End of added structs from original proj4.h

/*
 * The struct grid_t contains a pointer p to a 2D array of 
 * unsigned chars with n rows and n columns stored on
 * the heap of this process. Once this is initialized properly,
 * p[i][j] should be a valid unsigned char for all i = 0..(n-1)
 * and j = 0..(n-1).
 * Do not alter this typedef or struct in any way.
 */
typedef struct grid_t {
  unsigned int n;
  unsigned char ** p;
} grid;


/*
 * Initialize g based on fileName, where fileName
 * is a name of file in the present working directory
 * that contains a valid n-by-n grid of digits, where each
 * digit in the grid is between 1 and 9 (inclusive).
 * Do not alter this function prototype in any way.
 */
void initializeGrid(grid * g, char * fileName);


/*
 * This function will compute all diagonal sums in input that equal s using
 * t threads, where 1 <= t <= 3, and store all of the resulting
 * diagonal sums in output. Each thread should do
 * roughly (doesn't have to be exactly) (100 / t) percent of the 
 * computations involved in calculating the diagonal sums. 
 * This function should call (or call another one of your functions that calls)
 * pthread_create and pthread_join when 2 <= t <= 3 to create additional POSIX
 * thread(s) to compute all diagonal sums. 
 * Do not alter this function prototype in any way.
 */
void diagonalSums(grid * input, unsigned long s, grid * output, int t);


/*
 * Write the contents of g to fileName in the present
 * working directory. If fileName exists in the present working directory, 
 * then this function should overwrite the contents in fileName.
 * If fileName does not exist in the present working directory,
 * then this function should create a new file named fileName
 * and assign read and write permissions to the owner. 
 * Do not alter this function prototype in any way.
 */
void writeGrid(grid * g, char * fileName);

/*
 * Free up all dynamically allocated memory used by g.
 * This function should be called when the program is finished using g.
 * Do not alter this function prototype in any way.
 */
void freeGrid(grid * g);

/*
 * You may add any additional function prototypes and any additional
 * things you need to complete this project below this comment. 
 * Anything you add to this file should be commented. 
 */

//Added prototypes from original proj4.h   

/*
 * The writeChunk function is a thread function designed to write portion of the
 * grid into a file. It processes rows within the designated range, formats the 
 * grid data into the character buffer, and writes it to the provided file. It uses
 * the args void pointer from the WriteArguments structure to help process the grid
 * segment and write the output.
 *
 */
void * writeChunk(void * args);

/*
 * The threadFunction processes a portion of the grid in parallel, which can help
 * help with efficiency. It identifies the sequences along main and anti-diagonals
 * that match the target sum. This function utilizes tiles for cache locality 
 * improvement and also updates the output grid with our matches.
 */
void * threadFunction(void * args);

/*
 * The initializeMatchList function initizliazes our MatchList struct to manage
 * the dynamic lsit of Match objects efficiently. It sets up the initial state
 * of the MatchList and allocates memory for its internal array. Having this 
 * function implemented, it helps ensure accurate results in our program.
 */
void initializeMatchList(MatchList * matchList);

/*
 * The freeMatchList function releases the dynamically allocated memory 
 * used by the MatchList struct. It also ensures that the memory allocated for the 
 * 'matches' array is freed properly, and preventing any memory leaks.
 */
void freeMatchList(MatchList * matchList);

/*
 * The parallelWrite funciton divides our grid into segments and writes it to a file
 * in parallel using multiple threads. Each thread processes a portion of the grid and writes
 * its designated section to the necessary file. The use of this function helps us efficiently
 * write large grids in parallel when multi-threading is in use.
 */
void parallelWrite(unsigned char ** grid, unsigned int gridSize, char * fileName, int threadCount);

/*
 * The precomputeDiagonalLengths function calculates the maximum lengths of both the main/anti-diagonals
 * starting from each value in the grid. The lengths are then stored in two 2D arrays ('mainLengths'
 * and 'antiLengths'), which are used for efficient diagonal traversal during other computations. 
 * This function is useful for populating our given grid size.
 */
void precomputeDiagonalLengths(unsigned int gridSize, unsigned int ** mainLengths, unsigned int ** antiLengths);

//End of added prototypes from original proj4.h 

#endif

