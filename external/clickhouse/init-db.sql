CREATE TABLE IF NOT EXISTS fluentbit.query_logs
(
    `time` DateTime DEFAULT now(),
    `correlation_id` UUID,
    `query` String
) ENGINE = Log;

CREATE TABLE IF NOT EXISTS fluentbit.latency_logs
(
    `time` DateTime DEFAULT now(),
    `correlation_id` UUID,
    `latency` Int32,
    `host` String
) ENGINE = Log;
