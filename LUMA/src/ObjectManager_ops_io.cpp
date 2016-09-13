/*
 * --------------------------------------------------------------
 *
 * ------ Lattice Boltzmann @ The University of Manchester ------
 *
 * -------------------------- L-U-M-A ---------------------------
 *
 *  Copyright (C) 2015, 2016
 *  E-mail contact: info@luma.manchester.ac.uk
 *
 * This software is for academic use only and not available for
 * distribution without written consent.
 *
 */

#include "../inc/stdafx.h"
#include "../inc/ObjectManager.h"
#include "../inc/MpiManager.h"
#include <sstream>

// ***************************************************************************************************
// Routine to write out the coordinates of IBBodies at a given time step
void ObjectManager::io_write_body_pos(int timestep) {

	for (size_t ib = 0; ib < iBody.size(); ib++) {


			// Open file for given time step
			std::ofstream jout;
			jout.open(GridUtils::path_str + "/Body_" + std::to_string(ib) + "_position_" + std::to_string(timestep) + 
				"_rank" + std::to_string(MpiManager::my_rank) + ".out", std::ios::out);
			jout << "x" + std::to_string(timestep) + ", y" + std::to_string(timestep) + ", z" << std::endl;

			// Write out position
			for (size_t i = 0; i < iBody[ib].markers.size(); i++) {
#if (L_dims == 3)
				jout	<< iBody[ib].markers[i].position[0] << ", " 
						<< iBody[ib].markers[i].position[1] << ", " 
						<< iBody[ib].markers[i].position[2] << std::endl;
#else
				jout	<< iBody[ib].markers[i].position[0] << ", " 
						<< iBody[ib].markers[i].position[1] << ", " 
						<< 0.0 << std::endl;
#endif
			}
			jout.close();

	}

}


// ***************************************************************************************************
// Routine to write out the coordinates of IBbodies at a given time step
void ObjectManager::io_write_lift_drag(int timestep) {

	for (size_t ib = 0; ib < iBody.size(); ib++) {


			// Open file for given time step
			std::ofstream jout;
			jout.open(GridUtils::path_str + "/Body_" + std::to_string(ib) + "_LD_" + std::to_string(timestep) + "_rank" + std::to_string(MpiManager::my_rank) + ".out", std::ios::out);
			jout << "L" + std::to_string(timestep) + ", D" + std::to_string(timestep) << std::endl;

			// Sum variables
			double Lsum = 0.0, Dsum = 0.0;

			// Compute lift and drag
			for (size_t i = 0; i < iBody[ib].markers.size(); i++) {
				jout	<< iBody[ib].markers[i].force_xyz[0] << ", " 
						<< iBody[ib].markers[i].force_xyz[1] << std::endl;
				Lsum += iBody[ib].markers[i].force_xyz[0];
				Dsum += iBody[ib].markers[i].force_xyz[1];
			}

			jout << "Totals = " << std::endl;
			jout << Lsum << ", " << Dsum << std::endl;
			jout.close();

	}

}

// ***************************************************************************************************
void ObjectManager::io_restart(bool IO_flag, int level) {

	if (IO_flag) {

		// Output stream
		std::ofstream file;

		if (MpiManager::my_rank == 0 && level == 0) { // Overwrite as first to write
			file.open(GridUtils::path_str + "/restart_IBBody.out", std::ios::out);
		} else if (level == 0) { // Append
			file.open(GridUtils::path_str + "/restart_IBBody.out", std::ios::out | std::ios::app);
		} else { // Must be a subgrid which doesn't own any IB-bodies so return
			return;
		}


		// Counters
		size_t b, m, num_bod = iBody.size();

		// Write out the number of bodies
		file << num_bod << std::endl;

		// Loop over bodies and markers and write out the positions
		for (b = 0; b < num_bod; b++) {

			// Add a separator between bodies
			file << "\t/\t";

			// Number of markers with separator
			file << iBody[b].markers.size() << "\t/\t";

			for (m = 0; m < iBody[b].markers.size(); m++) {

				// Positions of each marker
				file	<< iBody[b].markers[m].position[0] << "\t"
						<< iBody[b].markers[m].position[1] << "\t"
						<< iBody[b].markers[m].position[2] << "\t";

				if (iBody[b].flex_rigid) {
					// Old positions of each marker
					file << iBody[b].markers[m].position_old[0] << "\t"
						<< iBody[b].markers[m].position_old[1] << "\t"
						<< iBody[b].markers[m].position_old[2] << "\t";
				}

			}

		}

		// Close file
		file.close();


	} else {

		// Input stream
		std::ifstream file;

		// Only level 0 grids can own IB-bodies
		if (level == 0) {
			file.open("./input/restart_IBBody.out", std::ios::in);
		}

		if (!file.is_open()) {
			std::cout << "Error: See Log File" << std::endl;
			*GridUtils::logfile << "Error opening IBM restart file. Exiting." << std::endl;
			exit(LUMA_FAILED);
		}

		// Read in one line of file at a time
		std::string line_in;	// String to store line
		std::istringstream iss;	// Buffer stream

		// Get line up to separator and put in buffer
		std::getline(file,line_in,'/');
		iss.str(line_in);
		iss.seekg(0); // Reset buffer position to start of buffer

		// Counters
		int b, m, num_bod, num_mark;

		// Read in number of bodies
		iss >> num_bod;

		// Check number of bodies is correct
		if (iBody.size() != num_bod) {
			std::cout << "Error: See Log File" << std::endl;
			*GridUtils::logfile << "Number of IBM bodies does not match the number specified in the restart file. Exiting." << std::endl;
			exit(LUMA_FAILED);
		}

		// Loop over bodies
		for (b = 0; b < num_bod; b++) {

			// Exit if reached end of file
			if (file.eof()) break;

			// Get next bit up to separator and put in buffer
			std::getline(file,line_in,'/');
			iss.str(line_in);
			iss.seekg(0); // Reset buffer position to start of buffer

			// Read in the number of markers for this body
			iss >> num_mark;

			// Check number of markers the same
			if (iBody[b].markers.size() != num_mark) {
				std::cout << "Error: See Log File" << std::endl;
				*GridUtils::logfile << "Number of IBM markers does not match the number specified for body " <<
					b << " in the restart file. Exiting." << std::endl;
				exit(LUMA_FAILED);
			}

			// Read in marker data
			for (m = 0; m < num_mark; m++) {

				// Positions of each marker
				iss		>> iBody[b].markers[m].position[0]
						>> iBody[b].markers[m].position[1]
						>> iBody[b].markers[m].position[2];

				if (iBody[b].flex_rigid) {
					// Old positions of each marker
					iss >> iBody[b].markers[m].position_old[0]
						>> iBody[b].markers[m].position_old[1]
						>> iBody[b].markers[m].position_old[2];
				}

			}

		}

		// Close file
		file.close();

	}

}

// ***************************************************************************************************
// Routine to write out the vtk (position) for each IB body at time step t (current capability is for unclosed objects only)
void ObjectManager::io_vtk_IBwriter(double tval) {

    // Loop through each iBody
    for (size_t ib = 0; ib < iBody.size(); ib++) {

        // Create file name then output file stream
        std::stringstream fileName;
        fileName << GridUtils::path_str + "/vtk_IBout.Body" << ib << "." << (int)tval << ".vtk";

        std::ofstream fout;
        fout.open( fileName.str().c_str() );

        // Add header information
        fout << "# vtk DataFile Version 3.0f\n";
        fout << "IB Output for body ID " << ib << " at time t = " << (int)tval << "\n";
        fout << "ASCII\n";
        fout << "DATASET POLYDATA\n";


        // Write out the positions of each Lagrange marker
        fout << "POINTS " << iBody[ib].markers.size() << " float\n";
        for (size_t i = 0; i < iBody[ib].markers.size(); i++) {

#if (L_dims == 3)
				fout	<< iBody[ib].markers[i].position[0] << " " 
						<< iBody[ib].markers[i].position[1] << " " 
						<< iBody[ib].markers[i].position[2] << std::endl;
#else
				fout	<< iBody[ib].markers[i].position[0] << " " 
						<< iBody[ib].markers[i].position[1] << " " 
						<< 1.0 << std::endl; // z = 1.0 as fluid ORIGIN is at z = 1.0
#endif
        }


        // Write out the connectivity of each Lagrange marker
        size_t nLines = iBody[ib].markers.size() - 1;

        if (iBody[ib].closed_surface == false)
            fout << "LINES " << nLines << " " << 3 * nLines << std::endl;
        else if (iBody[ib].closed_surface == true)
            fout << "LINES " << nLines + 1 << " " << 3 * (nLines + 1) << std::endl;

        for (size_t i = 0; i < nLines; i++) {
            fout << 2 << " " << i << " " << i + 1 << std::endl;
        }

        // If iBody[ib] is a closed surface then join last point to first point
        if (iBody[ib].closed_surface == true) {
            fout << 2 << " " << nLines << " " << 0 << std::endl;
        }

        fout.close();
    }
}


// ***************************************************************************************************
// Routine to read in point cloud data in tab separated, 3-column format from the input directory
void ObjectManager::io_readInCloud(PCpts* _PCpts, eObjectType objtype) {

	// Temporary variables
	double tmp_x, tmp_y, tmp_z;
	int a = 0;

	// Case-specific variables
	int on_grid_lev, on_grid_reg, body_length, body_start_x, body_start_y, body_centre_z;
	eCartesianDirection scale_direction;
	GridObj* g = NULL;

	// Open input file
	std::ifstream file;
	switch (objtype) {

	case eBBBCloud:
		file.open("./input/bbb_input.in", std::ios::in);
		on_grid_lev = L_object_on_grid_lev;
		on_grid_reg = L_object_on_grid_reg;
		body_length = L_object_length;
		body_start_x = L_start_object_x;
		body_start_y = L_start_object_y;
		body_centre_z = L_centre_object_z;
		scale_direction = L_object_scale_direction;
		break;

	case eBFLCloud:
		file.open("./input/bfl_input.in", std::ios::in);
		on_grid_lev = L_bfl_on_grid_lev;
		on_grid_reg = L_bfl_on_grid_reg;
		body_length = L_bfl_length;
		body_start_x = L_start_bfl_x;
		body_start_y = L_start_bfl_y;
		body_centre_z = L_centre_bfl_z;
		scale_direction = L_bfl_scale_direction;
		break;

	case eIBBCloud:
		file.open("./input/ibb_input.in", std::ios::in);

		// Definitions are in phsyical units so convert to LUs
		on_grid_lev = L_ibb_on_grid_lev;
		on_grid_reg = L_ibb_on_grid_reg;
		GridUtils::getGrid(_Grids, on_grid_lev, on_grid_reg, g);
		body_length = static_cast<int>(L_ibb_length / g->dx);
		body_start_x = static_cast<int>(L_start_ibb_x / g->dx);
		body_start_y = static_cast<int>(L_start_ibb_y / g->dx);
		body_centre_z = static_cast<int>(L_centre_ibb_z / g->dx);
		scale_direction = L_ibb_scale_direction;
		break;

	}
	
	// Handle failure to open
	if (!file.is_open()) {
		std::cout << "Error: See Log File" << std::endl;
		*GridUtils::logfile << "Error opening cloud input file. Exiting." << std::endl;
		exit(LUMA_FAILED);
	}

	// Get grid pointer
	GridUtils::getGrid(_Grids, on_grid_lev, on_grid_reg, g);

	// Return if this process does not have this grid
	if (g == NULL) return;

	// Loop over lines in file
	while (!file.eof()) {

		// Read in one line of file at a time
		std::string line_in;	// String to store line in
		std::istringstream iss;	// Buffer stream to store characters

		// Get line up to new line separator and put in buffer
		std::getline(file,line_in,'\n');
		iss.str(line_in);	// Put line in the buffer
		iss.seekg(0);		// Reset buffer position to start of buffer

		// Add coordinates to data store
		iss >> tmp_x;
		iss >> tmp_y;
		iss >> tmp_z;
			
		_PCpts->x.push_back(tmp_x);
		_PCpts->y.push_back(tmp_y);
			
		// If running a 2D calculation, only read in x and y coordinates and force z coordinates to match the domain
#if (L_dims == 3)
		_PCpts->z.push_back(tmp_z);
#else
		_PCpts->z.push_back(0);
#endif

	}
	file.close();

	// Error if no data
	if (_PCpts->x.empty() || _PCpts->y.empty() || _PCpts->z.empty()) {
		std::cout << "Error: See Log File" << std::endl;
		*GridUtils::logfile << "Failed to read object data from cloud input file." << std::endl;
		exit(LUMA_FAILED);
	} else {
		*GridUtils::logfile << "Successfully acquired object data from cloud input file." << std::endl;
	}

	
	// Rescale coordinates and shift to global lattice units
	// (option to scale based on whatever bounding box dimension chosen)
#ifdef L_CLOUD_DEBUG
	*GridUtils::logfile << "Rescaling..." << std::endl;
#endif
#if (scale_direction == eXDirection)
	double scale_factor = body_length /
		std::fabs(*std::max_element(_PCpts->x.begin(), _PCpts->x.end()) - *std::min_element(_PCpts->x.begin(), _PCpts->x.end()));
#elif (scale_direction == eYDirection)
	double scale_factor = body_length /
		std::fabs(*std::max_element(_PCpts->y.begin(), _PCpts->y.end()) - *std::min_element(_PCpts->y.begin(), _PCpts->y.end()));
#elif (scale_direction == eZDirection)
	double scale_factor = body_length /
		std::fabs(*std::max_element(_PCpts->z.begin(), _PCpts->z.end()) - *std::min_element(_PCpts->z.begin(), _PCpts->z.end()));
#endif
	double shift_x =  std::floor( body_start_x - scale_factor * *std::min_element(_PCpts->x.begin(), _PCpts->x.end()) );
	double shift_y =  std::floor( body_start_y - scale_factor * *std::min_element(_PCpts->y.begin(), _PCpts->y.end()) );
	// z-shift based on centre of object
	double shift_z =  std::floor( body_centre_z - scale_factor * (
		*std::min_element(_PCpts->z.begin(), _PCpts->z.end()) + 
		(*std::max_element(_PCpts->z.begin(), _PCpts->z.end()) - *std::min_element(_PCpts->z.begin(), _PCpts->z.end())) / 2
		) );

	// Apply to each point
	for (a = 0; a < static_cast<int>(_PCpts->x.size()); a++) {
		_PCpts->x[a] *= scale_factor; _PCpts->x[a] += shift_x;
		_PCpts->y[a] *= scale_factor; _PCpts->y[a] += shift_y;
		_PCpts->z[a] *= scale_factor; _PCpts->z[a] += shift_z;
	}

	// Declare local variables
	std::vector<int> locals;
	int global_i, global_j, global_k;

	// Exclude points which are not on this rank
#ifdef L_CLOUD_DEBUG
	*GridUtils::logfile << "Filtering..." << std::endl;
#endif
	a = 0;
	do {

		// Get global voxel index
		global_i = getVoxInd(_PCpts->x[a]);
		global_j = getVoxInd(_PCpts->y[a]);
		global_k = getVoxInd(_PCpts->z[a]);

		// If on this rank
		if (GridUtils::isOnThisRank(global_i, global_j, global_k, *g)) {
			// Increment counter
			a++;

		}
		// If not, erase
		else {
			_PCpts->x.erase(_PCpts->x.begin() + a);
			_PCpts->y.erase(_PCpts->y.begin() + a);
			_PCpts->z.erase(_PCpts->z.begin() + a);
		}

	} while (a < static_cast<int>(_PCpts->x.size()));


	// Write out the points remaining in for debugging purposes
#ifdef L_CLOUD_DEBUG
	*GridUtils::logfile << "Writing to file..." << std::endl;
	if (!_PCpts->x.empty()) {
		std::ofstream fileout;
		fileout.open(GridUtils::path_str + "/CloudPts_Rank" + std::to_string(MpiManager::my_rank) + ".out",std::ios::out);
		for (size_t i = 0; i < _PCpts->x.size(); i++) {
			fileout << std::to_string(_PCpts->x[i]) + '\t' + std::to_string(_PCpts->y[i]) + '\t' + std::to_string(_PCpts->z[i]);
			fileout << std::endl;
		}
		fileout.close();
	}
#endif

	// If there are points left
	if (!_PCpts->x.empty())	{

		// Perform a different post-processing action depending on the type of body
		switch (objtype)
		{

		case eBBBCloud:
#ifdef L_CLOUD_DEBUG
			*GridUtils::logfile << "Labelling..." << std::endl;
#endif
			// Label the grid sites
			for (a = 0; a < static_cast<int>(_PCpts->x.size()); a++) {
				// Get globals
				global_i = getVoxInd(_PCpts->x[a]);
				global_j = getVoxInd(_PCpts->y[a]);
				global_k = getVoxInd(_PCpts->z[a]);
				// Get local indices
				GridUtils::global_to_local(global_i, global_j, global_k, g, locals);

				// Update Typing Matrix
				if (g->LatTyp(locals[0], locals[1], locals[2], g->YInd.size(), g->ZInd.size()) == eFluid)
				{
					g->LatTyp(locals[0], locals[1], locals[2], g->YInd.size(), g->ZInd.size()) = eSolid;
				}
			}
			break;
		case eBFLCloud:

#ifdef L_CLOUD_DEBUG
			*GridUtils::logfile << "Building..." << std::endl;
#endif
			// Call BFL body builder
			bfl_build_body(_PCpts);
			break;

		case eIBBCloud:
			global_j = getVoxInd(_PCpts->y[a]);
			global_k = getVoxInd(_PCpts->z[a]);

#ifdef L_CLOUD_DEBUG
			*GridUtils::logfile << "Building..." << std::endl;
#endif
			// Call IBM body builder
			ibm_build_body(_PCpts, g);
			break;

		}

	}
}
// *****************************************************************************
// Routine for writing out the lift and drag forces on a BB object 
void ObjectManager::io_writeForceOnObject(double tval) {
	// Get grid on which object resides
	GridObj *g = NULL;
	GridUtils::getGrid(_Grids, L_object_on_grid_lev, L_object_on_grid_reg, g);
	// If this grid exists on this process
	if (g != NULL)
	{
		// Create stream
		std::ofstream fout;
		fout.precision(L_output_precision);
		// Filename
		std::stringstream fileName;
		fileName << GridUtils::path_str + "/LiftDrag" << "Rnk" << MpiManager::my_rank << ".csv";
		// Open file
		fout.open(fileName.str().c_str(), std::ios::out | std::ios::app);
		// Write out the header (first time step only)
		if (static_cast<int>(tval) == 0) fout << "Time,Fx,Fy,Fz" << std::endl;

		fout << std::to_string(tval) << ","
			<< std::to_string(force_on_object_x / pow(2, L_object_on_grid_lev)) << ","
			<< std::to_string(force_on_object_y / pow(2, L_object_on_grid_lev)) << ","
#if (dims == 3)
			<< std::to_string(force_on_object_z / pow(2, L_object_on_grid_lev))
#else
			<< std::to_string(0.0)
#endif
			<< std::endl;

		fout.close();

		// Reset for next time step
		force_on_object_x = 0.0;
		force_on_object_y = 0.0;
		force_on_object_z = 0.0;
	}

}
// *****************************************************************************