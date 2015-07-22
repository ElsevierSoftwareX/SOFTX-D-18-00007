/*
	This file contains all the generic functions for use in the main routine.
	In future, we could turn this into its own class with the functions as methods.
*/

#include "stdafx.h"
#include <vector>		// Dynamic array
#include <sstream>		// String stream package
#include "definitions.h"

using namespace std;

// ***************************************************************************************************

// Returns a vector with n uniformly spaced values between min and max
vector<double> linspace(double min, double max, int n)
{
	// Decalre resulting vector
	vector<double> result;

	// Set counter to zero
	int count = 0;
 
	// Loop 
	for (int i = 0; i <= n-2; i++)
	{
		double temp = min + i*(max-min)/(floor( (double)n ) - 1); // Cast n to a double to use floor
		result.insert(result.begin() + count, temp); // Insert element
		count += 1;
	}
 
	// Add last element
	result.insert(result.begin() + count, max);
	
	// Return vector
	return result;
}

// ***************************************************************************************************

// Like linspace but spaces elements by 1
vector<int> onespace(int min, int max)
{
	// Declare resulting array
	vector<int> result;

	// Loop and insert elements
	for (int i = 0; i <= (max-min); i++)
	{
		result.insert( result.begin() + i, min + i ); // Insert element

	}
 
	// Return vector
	return result;
}

// ***************************************************************************************************

// Functions to compute the magnitude of a vector. Overloads allow different input types
// 2D vector with arguments supplied separately
double vecnorm( double val1, double val2 )
{
	double result;
	
	result = sqrt( pow(val1,2) + pow(val2,2) );

	return result;
}

// 3D vector with arguments supplied separately
double vecnorm( double val1, double val2, double val3 )
{
	double result;
	
	result = sqrt( pow(val1,2) + pow(val2,2) + pow(val3,2) );

	return result;
}

// Supplied as a vector
double vecnorm( double vec[] )
{
	double result;
	
#if (dims == 3)

		result = sqrt( pow(vec[0],2) + pow(vec[1],2) + pow(vec[2],2) );

#else

		result = sqrt( pow(vec[0],2) + pow(vec[1],2) );

#endif

	return result;
}

// Supplied as a std::vector (as reference just in case vector is big)
double vecnorm( std::vector<double>& vec )
{
	double result = 0.0;
	
	for (size_t d = 0; d < vec.size(); d++) {

		result += pow(vec[d],2);

	}

	return sqrt(result);
}

// ***************************************************************************************************

// Routine to map the index of a coarse grid site to a corresponding fine site on the level below
vector<int> indmapref(int coarse_i, int x_start, int coarse_j, int y_start, int coarse_k, int z_start) {

	// Initialise result
	vector<int> fine_ind;
	
	// Map indices
	fine_ind.insert(fine_ind.begin(), 2*(coarse_i - x_start + 1) - 2 );
	fine_ind.insert(fine_ind.begin() + 1, 2*(coarse_j - y_start + 1) - 2 );
	fine_ind.insert(fine_ind.begin() + 2, 2*(coarse_k - z_start + 1) - 2 );

	return fine_ind;
}
// ***************************************************************************************************

// Dot Product
double dotprod(std::vector<double> vec1, std::vector<double> vec2) {

	// Declare scalar answer
    double answer = 0.0;

	// Do dot product
    for (size_t i = 0; i < vec1.size(); i++) {
        answer += vec1[i] * vec2[i];
    }

	// Return answer
    return answer;
}

// ***************************************************************************************************

// Multiplies matrix A by vector x.
vector<double> matrix_multiply(vector< vector<double> >& A, vector<double>& x) {

	// Check to makes sure dimensions are correct
	if (A[0].size() != x.size()) {
		cout << "Dimension mismatch -- cannot proceed. Exiting." << endl;
		system("pause");
		exit(EXIT_FAILURE);
	}

	// Initialise answer
	vector<double> product (x.size(), 0.0);

	// Do multiplication
    for (size_t row = 0; row < A.size(); row++) {
        for (size_t col = 0; col < x.size(); col++) {
            // Multiply the row of A by the column of B to get the row, column of product.
			product[row] += A[row][col] * x[col];
		}
	}

	return product;
}