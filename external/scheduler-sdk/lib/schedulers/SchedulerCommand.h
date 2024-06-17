#pragma once
#include "BaseComputationCommand.h"

#define PARAM_KEY_NAME "name"
#define PARAM_KEY_TIME "time"
#define PARAM_KEY_PARAMS "params"
#define PARAM_KEY_TASK_RUN_ID "task_run_id"
#define PARAM_KEY_ACTION "action"

enum TaskStatus {
	TaskStatusFailed,
	TaskStatusRunning,
	TaskStatusComplete,
	TaskStatusAborted,
	TaskStatusWaiting,
	TaskStatusUnknown,
};

enum SchedulerAction {
	SchedulerActionScheduleTask,
	SchedulerActionGetTaskStatus,
};

class SchedulerCommand : public BaseComputationCommand {
	bool ValidateParams(ComputationContext *context);

  public:
	virtual bool PreExecute(ComputationContext *context) override;
};
