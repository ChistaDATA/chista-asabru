add_executable(schedule_task schedule_task.cpp)

target_include_directories(schedule_task
	PUBLIC ../lib/schedulers
	${asabru_commons_SOURCE_DIR}/src
)

target_link_directories(schedule_task
	PUBLIC ../build/lib/
)
target_link_libraries(schedule_task scheduler_command rundeck_scheduler)
