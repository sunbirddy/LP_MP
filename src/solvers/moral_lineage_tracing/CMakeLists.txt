SET(SOURCE_FILES 
	mlt_solver.cpp
   )

foreach( source_file ${SOURCE_FILES} )
   string( REPLACE ".cpp" "" executable_file ${source_file} )
   add_executable( ${executable_file} ${source_file} ${headers} ${sources})
   target_link_libraries( ${executable_file} m stdc++ pthread ${HDF5_LIBRARIES} lgl)
   add_dependencies( ${executable_file} all_solvers)
endforeach( source_file ${SOURCE_FILES} )

