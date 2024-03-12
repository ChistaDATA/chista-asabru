FROM golang:1.22-alpine3.18 AS builder
WORKDIR /project
RUN apk add --no-cache --update git
RUN git clone https://github.com/grafana/snowflake-prometheus-exporter.git
WORKDIR /project/snowflake-prometheus-exporter
RUN go build -v -o /snowflake-exporter cmd/snowflake-exporter/main.go

FROM alpine:3.18
COPY --from=builder /snowflake-exporter /snowflake-exporter
CMD ["/snowflake-exporter"]
