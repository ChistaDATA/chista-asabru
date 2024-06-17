#pragma once
#include "BaseComputationCommand.h"
#include "SchedulerCommand.h"

class RundeckScheduler : public SchedulerCommand {
  private:
	std::string host;
	std::string apiVersion;
	std::string apiToken;

  public:
	bool Execute(ComputationContext *context) override;

	RundeckScheduler(std::string host, std::string apiVersion, std::string apiToken)
		: host(host), apiVersion(apiVersion), apiToken(apiToken) {}

	std::string scheduleTask(const std::string &name, const std::chrono::system_clock::time_point &time,
							 const std::vector<std::string> params);

	TaskStatus getTaskStatus(const std::string &run_id);

	// Function to start executing tasks
	void start();

	// Function to stop the scheduler
	void stop();
};
