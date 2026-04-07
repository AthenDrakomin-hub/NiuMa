
file(GLOB FRAMEWORK_SRC_LIST
  ${CMAKE_CURRENT_SOURCE_DIR}/*/*.c
  ${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/*/*.h)

assign_folder_group(${FRAMEWORK_SRC_LIST})

add_library(Framework STATIC ${FRAMEWORK_SRC_LIST})

message("${CMAKE_CURRENT_SOURCE_DIR}")

# 关键：添加 Boost.Log 动态链接定义
target_compile_definitions(Framework PRIVATE
    BOOST_LOG_DYN_LINK
    BOOST_ALL_DYN_LINK
)

target_compile_options(Framework PRIVATE ${COMPILE_OPTIONS_DEFAULT})

target_include_directories(Framework PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(Framework PUBLIC
    ${Boost_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    ${3RDPART_LIBRARIES}
    ${MYSQL_LIBRARIES}
    ${RABBITMQ_LIBRARIES}
)

update_cached_list(FRAMEWORK_LIBRARY Framework)
