FROM rundeck/rundeck:5.4.0
USER root
RUN apt update
RUN apt install -y python3
ENV RUNDECK_SERVER_ADDRESS=0.0.0.0
ENV RUNDECK_GRAILS_URL=http://localhost:4440