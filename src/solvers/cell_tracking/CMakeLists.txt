add_executable( cell_tracking_mother_machine cell_tracking_mother_machine.cpp ${headers} ${sources} )
target_link_libraries( cell_tracking_mother_machine m stdc++ pthread lgl )

#add_executable( cell_tracking cell_tracking.cpp ${headers} ${sources} )
#target_link_libraries( cell_tracking m stdc++ pthread lgl )

add_library( json_cpp jsoncpp.cpp )

add_executable( conservation_tracking conservation_tracking.cpp graphreader.cpp jsongraphreader.cpp jsoncpp.cpp ${headers} ${sources} )
add_dependencies(conservation_tracking all_solvers)
target_link_libraries( conservation_tracking m stdc++ pthread lgl)
