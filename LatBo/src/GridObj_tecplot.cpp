// Routine for doing a TecPlot write out to file

#include "../inc/stdafx.h"
#include "../inc/GridObj.h"
#include "../inc/MpiManager.h"


// ************************************************************** //
void GridObj::io_tecplot(double tval) {

	std::ofstream tecfile;

	// Filename
	std::string filename ("./" + GridUtils::path_str + "/tecplotout.Lev" + std::to_string(level) + ".Reg" + std::to_string(region_number)
			+ "." + std::to_string((int)tval) + ".dat");

	// Check if file exists
	bool exists_flag = false;
	if (std::ifstream(filename)) {
		exists_flag = true;
	}

	// Create file
	tecfile.open(filename, std::ios::out|std::ios::app);

	// Set precision and force fixed formatting
	tecfile.precision(output_precision);
	tecfile.setf(std::ios::fixed);
	tecfile.setf(std::ios::showpoint);

	// Only the first rank to write to the file writes the header
	if (!exists_flag) {

		// Declare sizes
		int i_count = 0, j_count = 0, k_count = 1;

		// Get number of sites
		if (level == 0) {

			// Level 0 admissable sites are straightforward
			i_count = N;
			j_count = M;
#if (dims == 3)
			k_count = K;
#endif

		} else {

			// Sub-grid sites from global sizes
			i_count = (RefXend[level-1][region_number] - RefXstart[level-1][region_number] + 1) * 2;
			j_count = (RefYend[level-1][region_number] - RefYstart[level-1][region_number] + 1) * 2;
#if (dims == 3)
			k_count = (RefZend[level-1][region_number] - RefZstart[level-1][region_number] + 1) * 2;
#endif
		}
		
		// Add header
		tecfile << "TITLE = L" << level << " R" << region_number << " --> All grid quantities" << std::endl;
		tecfile << "FILETYPE = FULL" << std::endl;
		tecfile << "VARIABLES = \"X\" \"Y\" \"Z\" \"RHO\" \"UX\" \"UY\" \"UZ\" \"TA_RHO\" \"TA_UX\" \"TA_UY\" \"TA_UZ\" "<<
			"\"TA_UXUX\" \"TA_UXUY\" \"TA_UXUZ\" \"TA_UYUY\" \"TA_UYUZ\" \"TA_UZUZ\"" << std::endl;
		tecfile << "ZONE" << std::endl;

		tecfile << "I = " << i_count
				<< ", J = " << j_count
				<< ", K = " <<	k_count
				<< std::endl;

		tecfile << "ZONETYPE = Ordered, DATAPACKING = POINT" << std::endl;
		tecfile << "SOLUTIONTIME = " << std::to_string(tval) << std::endl;
	}


	// Indices
	size_t i,j,k,v;
		
	// Write out values
	for (k = 0; k < ZInd.size(); k++) {
		for (j = 0; j < YInd.size(); j++) {
			for (i = 0; i < XInd.size(); i++) {


#ifdef BUILD_FOR_MPI
				// Don't write out the receiver overlap in MPI
				if (!GridUtils::isOnRecvLayer(XPos[i],YPos[j],ZPos[k]))
#endif	// BUILD_FOR_MPI				
				{
				
					// Write out X, Y, Z
					tecfile << XPos[i] << "\t" << YPos[j] << "\t" << ZPos[k] << "\t";

					// Write out rho and u
					tecfile << rho(i,j,k,YInd.size(),ZInd.size()) << "\t";
					for (v = 0; v < dims; v++) {
						tecfile << u(i,j,k,v,YInd.size(),ZInd.size(),dims) << "\t";
					}
#if (dims != 3)
					tecfile << 0 << "\t";
#endif
				
					// Write out time averaged rho and u
					tecfile << rho_timeav(i,j,k,YInd.size(),ZInd.size()) << "\t";
					for (v = 0; v < dims; v++) {
						tecfile << ui_timeav(i,j,k,v,YInd.size(),ZInd.size(),dims) << "\t";
					}
#if (dims != 3)
					tecfile << 0 << "\t";
#endif

					// Write out time averaged u products
					tecfile << uiuj_timeav(i,j,k,0,YInd.size(),ZInd.size(),(3*dims-3)) << "\t";
					tecfile << uiuj_timeav(i,j,k,1,YInd.size(),ZInd.size(),(3*dims-3)) << "\t";
#if (dims == 3)
					tecfile << uiuj_timeav(i,j,k,2,YInd.size(),ZInd.size(),(3*dims-3)) << "\t";
#else
					tecfile << 0 << "\t";
#endif
#if (dims == 3)
					tecfile << uiuj_timeav(i,j,k,3,YInd.size(),ZInd.size(),(3*dims-3)) << "\t";
					tecfile << uiuj_timeav(i,j,k,4,YInd.size(),ZInd.size(),(3*dims-3)) << "\t";
					tecfile << uiuj_timeav(i,j,k,5,YInd.size(),ZInd.size(),(3*dims-3)) << "\t";
#else
					tecfile << uiuj_timeav(i,j,k,2,YInd.size(),ZInd.size(),(3*dims-3)) << "\t";
					tecfile << 0 << "\t" << 0 << "\t";
#endif

					// New line
					tecfile << std::endl;


				}

			}
		}
	}


	// Now do any sub-grids
	if (NumLev > level) {
		for (size_t reg = 0; reg < subGrid.size(); reg++) {
			subGrid[reg].io_tecplot(tval);
		}
	}

}