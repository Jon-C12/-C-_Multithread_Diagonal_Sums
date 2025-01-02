This project involves designing and implementing a multithreaded C program that computes diagonal sums equal to a given value s in a 2D grid of size n x n, where n ≥ 1. The grid contains digits ranging from 1 to 9 and is provided as an input file. The program will utilize up to three threads (t threads, where 1 ≤ t ≤ 3) to parallelize the computation of diagonal sums, ensuring improved performance for larger grids. The results will be written to an output file, listing all diagonal sums that match the given input value s.

The implementation requires adherence to the provided proj4.h header file and its defined functions, and the source code must compile correctly alongside a given main.c file and Makefile. The program must run on a vcf node and manage dynamic memory effectively, avoiding memory leaks as verified by valgrind. The program should also comply with Unix file system requirements for file handling, permissions, and multithreading.

This project strengthens understanding of multithreaded programming, efficient algorithm design, and system-level programming in Unix. It emphasizes performance optimization, thread synchronization, and proper dynamic memory allocation and deallocation while leveraging the benefits of parallel processing.

Test Cases:


./proj4.out in1.txt out1.txt 10 1; diff out1.txt correctOut1.txt | wc -c;
Computing the diagonal sums equal to 10 in a 5-by-5 grid using 1 thread(s).
Elapsed time for computing the diagonal sums using 1 thread(s): 0.000002 seconds.
Writing the diagonal sums equal to 10 to the file out1.txt.
Program is complete. Goodbye!
0

./proj4.out in2.txt out2.txt 19 2; diff out2.txt correctOut2.txt | wc -c;
Computing the diagonal sums equal to 19 in a 17-by-17 grid using 2 thread(s).
Elapsed time for computing the diagonal sums using 2 thread(s): 0.000130 seconds.
Writing the diagonal sums equal to 19 to the file out2.txt.
Program is complete. Goodbye!
0

./proj4.out in3.txt out3.txt 3 3; diff out3.txt correctOut3.txt | wc -c;
Computing the diagonal sums equal to 3 in a 672-by-672 grid using 3 thread(s).
Elapsed time for computing the diagonal sums using 3 thread(s): 0.003658 seconds.
Writing the diagonal sums equal to 3 to the file out3.txt.
Program is complete. Goodbye!
0

./proj4.out in4.txt out4.txt 13 2; diff out4.txt correctOut4.txt | wc -c;
Computing the diagonal sums equal to 13 in a 2778-by-2778 grid using 2 thread(s).
Elapsed time for computing the diagonal sums using 2 thread(s): 0.215538 seconds.
Writing the diagonal sums equal to 13 to the file out4.txt.
Program is complete. Goodbye!
0

./proj4.out in5.txt out5.txt 1222 3; diff out5.txt correctOut5.txt | wc -c;
Computing the diagonal sums equal to 1222 in a 3567-by-3567 grid using 3 thread(s).
Elapsed time for computing the diagonal sums using 3 thread(s): 11.546067 seconds.
Writing the diagonal sums equal to 1222 to the file out5.txt.
Program is complete. Goodbye!
0
