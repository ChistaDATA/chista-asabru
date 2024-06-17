add_library(rundeck_scheduler SHARED RundeckScheduler.cpp)
CPMAddPackage(
		NAME nlohmann_json
		VERSION 3.11.3
		GITHUB_REPOSITORY nlohmann/json
		OPTIONS
			"JSON_BuildTests OFF"
)
CPMAddPackage(
		NAME cpr
		GITHUB_REPOSITORY libcpr/cpr
		GIT_TAG 0817715923c9705e68994eb52ef9df3f6845beba
)
target_link_libraries(rundeck_scheduler scheduler_command nlohmann_json cpr)
target_include_directories(rundeck_scheduler PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../)
