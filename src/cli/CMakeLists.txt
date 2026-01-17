target_sources(vanitas PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/app.cpp
  ${CMAKE_CURRENT_LIST_DIR}/commands/file.cpp
  ${CMAKE_CURRENT_LIST_DIR}/commands/help.cpp
  ${CMAKE_CURRENT_LIST_DIR}/commands/pipe.cpp
  ${CMAKE_CURRENT_LIST_DIR}/commands/run.cpp
  ${CMAKE_CURRENT_LIST_DIR}/commands/profile.cpp
  ${CMAKE_CURRENT_LIST_DIR}/commands/analyze_stream.cpp
)

target_include_directories(vanitas PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
)
