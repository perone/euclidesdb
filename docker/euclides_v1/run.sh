#!/bin/sh
docker run \
	-p 50000:50000 \
	-v /tmp:/database \
	-it perone/euclidesdb:0.1.1
