#!/bin/sh
docker run \
	-p 50000:50000 \
	-v /Users/perone/Devel/euclidesdb/docker/euclides_v1/database:/database \
	-it perone/euclidesdb:0.1.1
