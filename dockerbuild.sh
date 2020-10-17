set -e
docker build --cache-from plasma:latest --build-arg USER_ID=`id -u` -t plasma .
