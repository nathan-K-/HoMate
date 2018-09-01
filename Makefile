nodered:
	docker run -it -p 1880:1880 nodered/node-red-docker:v6

influxdb:
	docker run -d -p 8086:8086 --name influxdb \
      -v /var/lib/influxdb:/var/lib/influxdb:rw \
      influxdb:1.6
