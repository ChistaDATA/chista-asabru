#include "RundeckScheduler.h"
#include "BaseComputationCommand.h"
#include "ComputationContext.h"
#include "cpr/api.h"
#include "cpr/cprtypes.h"
#include "logger/Logger.h"
#include <any>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <vector>

using json = nlohmann::json;

std::string RundeckScheduler::scheduleTask(const std::string &name, const std::chrono::system_clock::time_point &time,
										   const std::vector<std::string> params) {
	const std::string api_url = host + "/api/" + apiVersion + "/job/" + name + "/run";
	cpr::Response resp = cpr::Post(cpr::Url{api_url}, cpr::Header{{"X-Rundeck-Auth-Token", apiToken}},
								   cpr::Header{{"Accept", "application/json"}});
	if (resp.status_code != cpr::status::HTTP_OK) {
		LOG_ERROR(std::to_string(resp.status_code))
		LOG_ERROR(resp.text)
		return "";
	}
	LOG_INFO(std::to_string(resp.status_code))
	LOG_INFO(resp.status_line)
	LOG_INFO(resp.text)
	nlohmann::json body = nlohmann::json::parse(resp.text);
	std::string run_id = nlohmann::to_string(body["id"]);
	return run_id;
}

TaskStatus RundeckScheduler::getTaskStatus(const std::string &run_id) {
	const std::string api_url = host + "/api/" + apiVersion + "/execution/" + run_id + "/state";
	cpr::Response resp =
		cpr::Get(cpr::Url{api_url}, cpr::Header{{"X-Rundeck-Auth-Token", apiToken}}, cpr::Header{{"Accept", "application/json"}});
	nlohmann::json body = nlohmann::json::parse(resp.text);
	std::string execution_state = body["executionState"];
	if (execution_state == "SUCCEEDED") {
		return TaskStatusComplete;
	} else if (execution_state == "FAILED") {
		return TaskStatusFailed;
	} else if (execution_state == "RUNNING") {
		return TaskStatusRunning;
	} else if (execution_state == "ABORTED") {
		return TaskStatusAborted;
	} else if (execution_state == "WAITING") {
		return TaskStatusWaiting;
	}
	return TaskStatusUnknown;
}

// Function to start executing tasks
void RundeckScheduler::start() {}

void RundeckScheduler::stop() {}

bool RundeckScheduler::Execute(ComputationContext *context) {
	SchedulerAction action;
	try {
		action = std::any_cast<SchedulerAction>(context->Get(PARAM_KEY_ACTION));
	} catch (std::exception &e) {
		LOG_ERROR(std::string("action: ") + e.what())
		return false;
	}
	switch (action) {
	case SchedulerActionScheduleTask: {
		std::chrono::system_clock::time_point time;
		std::string name;
		std::vector<std::string> params;
		try {
			time = std::any_cast<std::chrono::system_clock::time_point>(context->Get(PARAM_KEY_TIME));
		} catch (std::exception &e) {
			LOG_ERROR(std::string("time: ") + e.what())
			return false;
		}
		try {
			name = std::any_cast<std::string>(context->Get(PARAM_KEY_NAME));
		} catch (std::exception &e) {
			LOG_ERROR(std::string("name: ") + e.what())
			return false;
		}
		try {
			params = std::any_cast<std::vector<std::string>>(context->Get(PARAM_KEY_PARAMS));
		} catch (std::exception &e) {
			LOG_ERROR(std::string("params: ") + e.what())
			return false;
		}
		std::string task_run_id = scheduleTask(name, time, params);
		context->Put("result", task_run_id);
		break;
	}
	case SchedulerActionGetTaskStatus: {
		std::string task_run_id = std::any_cast<std::string>(context->Get(PARAM_KEY_TASK_RUN_ID));
		LOG_INFO("Task run id: " + task_run_id)
		TaskStatus task_status = getTaskStatus(task_run_id);
		LOG_INFO("Task status: " + std::to_string(task_status))
		context->Put("result", task_status);
		break;
	}
	default: {
		throw "Unknown scheduler action";
	}
	}
	return true;
}
