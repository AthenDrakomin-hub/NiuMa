# Findmysql-concpp.cmake
# 查找 MySQL Connector/C++ 库

# 查找头文件目录 (包含 mysql/jdbc.h 的目录的父目录)
find_path(MYSQL_CONCPP_INCLUDE_DIR
    NAMES mysql/jdbc.h
    PATHS 
        /usr/include/mysql-cppconn
        /usr/local/include/mysql-cppconn
        /usr/include
        /usr/local/include
)

# 查找库文件
find_library(MYSQL_CONCPP_LIBRARY
    NAMES mysqlcppconn mysqlcppconn8
    PATHS 
        /usr/lib
        /usr/local/lib
        /usr/lib/x86_64-linux-gnu
)

# 创建导入目标
if(MYSQL_CONCPP_INCLUDE_DIR AND MYSQL_CONCPP_LIBRARY)
    message(STATUS "Found mysql-concpp: ${MYSQL_CONCPP_LIBRARY}")
    message(STATUS "mysql-concpp include dir: ${MYSQL_CONCPP_INCLUDE_DIR}")
    
    if(NOT TARGET mysql::concpp-jdbc)
        add_library(mysql::concpp-jdbc SHARED IMPORTED)
        set_target_properties(mysql::concpp-jdbc PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${MYSQL_CONCPP_INCLUDE_DIR}"
            IMPORTED_LOCATION "${MYSQL_CONCPP_LIBRARY}"
        )
    endif()
    
    set(mysql-concpp_FOUND TRUE)
else()
    set(mysql-concpp_FOUND FALSE)
endif()

# 提供标准变量
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(mysql-concpp
    REQUIRED_VARS MYSQL_CONCPP_LIBRARY MYSQL_CONCPP_INCLUDE_DIR
)
