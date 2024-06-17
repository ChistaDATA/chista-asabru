#include "SchedulerCommand.h"
#include "logger/Logger.h"
#include <any>

bool validateScheduleJob(ComputationContext *context) {
	std::any name_value = context->Get(PARAM_KEY_NAME);
	if (!name_value.has_value()) {
		LOG_ERROR(std::string("SchedulerCommand::Required parameter missing. ") + PARAM_KEY_NAME)
		return false;
	}
	std::string name = std::any_cast<const std::string>(name_value);

	auto time_value = context->Get(PARAM_KEY_TIME);
	if (!name_value.has_value()) {
		LOG_ERROR(std::string("SchedulerCommand::Required parameter missing. ") + PARAM_KEY_TIME)
		return false;
	}
	std::chrono::time_point<std::chrono::system_clock> time =
		std::any_cast<const std::chrono::system_clock::time_point>(time_value);

	auto params_value = context->Get(PARAM_KEY_PARAMS);
	if (!name_value.has_value()) {
		LOG_ERROR(std::string("SchedulerCommand::Required parameter missing. ") + PARAM_KEY_PARAMS)
		return false;
	}
	std::vector<std::string> params = std::any_cast<std::vector<std::string>>(params_value);
	return true;
}

bool validateGetStatus(ComputationContext *context) {
	std::any run_id_value = context->Get(PARAM_KEY_TASK_RUN_ID);
	if (!run_id_value.has_value()) {
		LOG_ERROR(std::string("SchedulerCommand::Required parameter missing. ") + PARAM_KEY_TASK_RUN_ID)
		return false;
	}
	std::string run_id = std::any_cast<const std::string>(run_id_value);
	return true;
}

bool SchedulerCommand::ValidateParams(ComputationContext *context) {
	std::any action_value = context->Get(PARAM_KEY_ACTION);
	if (!action_value.has_value()) {
		LOG_ERROR(std::string("SchedulerCommand::Required parameter missing. ") + PARAM_KEY_ACTION)
		return false;
	}
	SchedulerAction action = std::any_cast<SchedulerAction>(action_value);
	switch (action) {
	case SchedulerActionScheduleTask:
		return validateScheduleJob(context);
	case SchedulerActionGetTaskStatus:
		return validateGetStatus(context);
	default: {
		LOG_ERROR("Unknown action")
		return false;
	}
	}
}

bool SchedulerCommand::PreExecute(ComputationContext *context) { return ValidateParams(context); }
