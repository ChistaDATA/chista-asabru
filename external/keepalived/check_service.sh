#!/bin/bash
# Check if my_service.sh is running
if pgrep -f my_service.sh > /dev/null; then
    exit 0
else
    exit 1
fi
