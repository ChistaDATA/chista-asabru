#include "BaseComputationCommand.h"
#include "SchedulerCommand.h"
#include "logger/Logger.h"
#include "rundeck/RundeckScheduler.h"
#include <chrono>
#include <cstdlib>
#include <vector>

void usage(char *argv[]) { std::cout << "Usage: " << argv[0] << " run <task_id> | status <run_id>" << std::endl; }

int main(int argc, char *argv[]) {
	if (argc != 3) {
		usage(argv);
		exit(-1);
	}
	std::string sub_command = argv[1];
	std::string id = argv[2];
	SchedulerCommand *cmd = new RundeckScheduler("http://localhost:4440", "44", "H0adk5i79Vy72wxynciXsBJzzRA2wEKF");
	ComputationContext context;
	if (sub_command == "run") {
		context.Put(PARAM_KEY_ACTION, SchedulerAction::SchedulerActionScheduleTask);
		context.Put(PARAM_KEY_NAME, id);
		context.Put(PARAM_KEY_TIME, std::chrono::system_clock::now());
		context.Put(PARAM_KEY_PARAMS, std::vector<std::string>{});
	} else if (sub_command == "status") {
		context.Put(PARAM_KEY_ACTION, SchedulerAction::SchedulerActionGetTaskStatus);
		context.Put(PARAM_KEY_TASK_RUN_ID, id);
	} else {
		usage(argv);
		exit(-1);
	}
	if (!cmd->PreExecute(&context)) {
		LOG_ERROR(sub_command + " PreExecute failed")
		exit(-1);
	}
	if (!cmd->Execute(&context)) {
		LOG_ERROR(sub_command + " Execute failed")
		exit(-1);
	}
	if (!cmd->PostExecute(&context)) {
		LOG_ERROR(sub_command + " PostExecute failed")
		exit(-1);
	}
	auto result = context.Get("result");
	// LOG_INFO(result);
	delete cmd;
}
