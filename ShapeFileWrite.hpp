//This header provides a high-level interface for creating shapefiles with ShapeLib
//Author: Bryan Poling
#pragma once

//System Includes
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <system_error>

//ShapeLib Includes
#include "shapefil.h"

//Include C++ Filesystem header. Unfortunately, this is in a different spot for different compilers/versions of compilers
#if defined(__GNUC__)
	//In principal, this check should work on VS also, but it doesn't because __cplusplus is reported incorrectly on VS without special compiler flags
	#if (__cplusplus >= 201703L)
		#if __has_include(<filesystem>)
			#include <filesystem>
		#else
			#include <experimental/filesystem>
			namespace std { namespace filesystem { using namespace std::experimental::filesystem; } }
		#endif
	#else
		#include <experimental/filesystem>
		namespace std { namespace filesystem { using namespace std::experimental::filesystem; } }
	#endif
#elif defined(_MSC_VER)
	//Some versions of VS have <filesystem> also, but that gives you the "Microsoft-specific implementation" instead of the C++ standard version
	#include <experimental/filesystem>
	namespace std { namespace filesystem { using namespace std::experimental::filesystem; } }
#endif

struct ShapefileHandle {
	std::filesystem::path pathBase; //Path to shapefile (not including the various file extentions)
	SHPHandle shpHandle;
	DBFHandle dbfHandle;
	size_t recordCount; //Number of records currently written to file
};

//Create a point shapefile. FilePath should be the path to the destination file, not including the file extention. We only support double-type attributes right now.
//Lat and Lon are provided in radians, although they will be converted to degrees when saved.
ShapefileHandle CreatePointShapefile(std::filesystem::path FilePath, std::vector<std::string> const & AttributeNames);
void            AddPointToShapefile (ShapefileHandle & Handle, double Lat, double Lon, std::vector<double> const & Attributes);
void            ClosePointShapefile (ShapefileHandle & Handle);

inline ShapefileHandle CreatePointShapefile(std::filesystem::path FilePath, std::vector<std::string> const & AttributeNames) {
	ShapefileHandle handle;
	handle.pathBase = FilePath;
	
	//Generate paths to the actual files
	std::filesystem::path SHP_path = FilePath; SHP_path += ".shp";
	std::filesystem::path SHX_path = FilePath; SHX_path += ".shx";
	std::filesystem::path DBF_path = FilePath; DBF_path += ".dbf";
	std::filesystem::path PRJ_path = FilePath; PRJ_path += ".prj";
	
	//Remove any existing files if they already exist (do nothing if they don't exist)
	std::error_code ec;
	std::filesystem::remove(SHP_path, ec);
	std::filesystem::remove(SHX_path, ec);
	std::filesystem::remove(DBF_path, ec);
	std::filesystem::remove(PRJ_path, ec);
	
	//Create and open the SHP, SHX, and DBF files
	handle.shpHandle = SHPCreate(SHP_path.string().c_str(), SHPT_POINT);
	handle.dbfHandle = DBFCreate(DBF_path.string().c_str());
	
	//Initialize record counter
	handle.recordCount = 0U;
	
	//Create and write the PRJ file
	std::ofstream fileStream(PRJ_path.string(), std::ofstream::out | std::ofstream::binary);
	if (! fileStream.is_open())
		std::cerr << "Error in CreatePointShapefile(): Could not open PRJ file for writing.\r\n";
	else {
		fileStream << "GEOGCS[\"GCS_WGS_1984\",DATUM[\"D_WGS_1984\",SPHEROID[\"WGS_1984\",6378137,298.257223563]]";
		fileStream << ",PRIMEM[\"Greenwich\",0],UNIT[\"Degree\",0.017453292519943295]]";
		fileStream.close();
	}
	
	//Warn and abort if we couldn't create one of the necessary files
	if (handle.shpHandle == NULL)
		std::cerr << "Error in CreatePointShapefile(): Could not open SHP or SHX file for writing.\r\n";
	if (handle.dbfHandle == NULL)
		std::cerr << "Error in CreatePointShapefile(): Could not open DBF file for writing.\r\n";
	if ((handle.shpHandle == NULL) || (handle.dbfHandle == NULL))
		return handle;
	
	//Create attributes in DBF file
	for (std::string name : AttributeNames) {
		if (name.size() > 11U)
			name = name.substr(0U, 11U);
		if (DBFAddField(handle.dbfHandle, name.c_str(), FTDouble, 12, 6) == -1) {
			std::cerr << "Error in CreatePointShapefile(): Could not add attribute '" << name << "'\r\n";
			return handle;
		}
	}
	
	return handle;
}

inline void AddPointToShapefile(ShapefileHandle & Handle, double Lat, double Lon, std::vector<double> const & Attributes) {
	if ((Handle.shpHandle == NULL) || (Handle.dbfHandle == NULL) || (DBFGetFieldCount(Handle.dbfHandle) != int(Attributes.size()))) {
		std::cerr << "Error in AddPointToShapefile(): Invalid shapefile handle or Attributes.\r\n";
		return;
	}
	
	//Write shape object
	const double PI = 3.14159265358979;
	double latDegrees = Lat*180.0/PI;
	double lonDegrees = Lon*180.0/PI;
	SHPObject	* psShape = SHPCreateObject(SHPT_POINT, -1, 0, NULL, NULL, 1, &lonDegrees, &latDegrees, NULL, NULL);
	SHPWriteObject(Handle.shpHandle, -1, psShape);
	SHPDestroyObject(psShape);
	
	//Write attributes
	for (int fieldCounter = 0; fieldCounter < DBFGetFieldCount(Handle.dbfHandle); fieldCounter++)
		DBFWriteDoubleAttribute(Handle.dbfHandle, Handle.recordCount, fieldCounter, Attributes[fieldCounter]);
	
	//Increment record counter
	Handle.recordCount++;
}

inline void ClosePointShapefile(ShapefileHandle & Handle) {
	if (Handle.shpHandle != NULL)
		SHPClose(Handle.shpHandle);
	if (Handle.dbfHandle != NULL)
		DBFClose(Handle.dbfHandle);
	Handle.shpHandle = NULL;
	Handle.dbfHandle = NULL;
}




