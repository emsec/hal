cmake_minimum_required(VERSION 2.8)

set(TARGET bddcalc)

set(HEADERS
  hashtbl.h
  parser.h
  parser_.h
  slist.h
)

set(SOURCES 
  hashtbl.cxx
  lexer.cxx
  parser.cxx
)

add_executable(${TARGET} ${HEADERS} ${SOURCES})
target_include_directories(${TARGET} PRIVATE ${BUDDY_ROOT_DIR}/src)
target_link_libraries(${TARGET} ${BUDDY_TARGET})
set_target_properties(${TARGET} PROPERTIES FOLDER "Examples")
