#!/bin/bash

# This script is used to send email and/or Slack notification when Keepalived state changes

TYPE=$1 # MASTER, BACKUP, or FAULT
NAME=$2 # Name of the Keepalived instance
STATE=$3 # New state of the instance
TIMESTAMP=$(date '+%Y-%m-%d %H:%M:%S')

# Customizable Variables
LOG_FILE="/var/log/keepalived-notify.log"
EMAIL_TO="admin@example.com"
SLACK_WEBHOOK_URL="https://hooks.slack.com/services/T00000000/B00000000/xxxxxxxxxxxxxx"

# Log the state change
echo "$TIMESTAMP - Keepalived State Change: $TYPE $NAME $STATE" >> $LOG_FILE

Function to send email
send_email() {
    SUBJECT="Keepalived State Change: $STATE"
    BODY="Timestamp: $TIMESTAMP\nInstance: $NAME\nNew State: $STATE\nType: $TYPE"
    echo -e "Subject:$SUBJECT\n\n$BODY" | sendmail $EMAIL_TO
}

# Function to send Slack notification
send_slack_notification() {
    PAYLOAD="payload={\"text\": \"*:rotating_light: Keepalived State Change :rotating_light:*\n- Timestamp: $TIMESTAMP\n- Instance: $NAME\n- New State: $STATE\n- Type: $TYPE\"}"
    curl -X POST --data-urlencode "$PAYLOAD" $SLACK_WEBHOOK_URL
}

# Send email and/or Slack notification based on the state change
case $STATE in
    "MASTER")
        send_email
        send_slack_notification
        ;;
    "BACKUP")
        send_email
        send_slack_notification
        ;;
    "FAULT")
        send_email
        send_slack_notification
        ;;
    *)
        echo "Unknown state: $STATE" >> $LOG_FILE
        ;;
esac
```
