set -e
docker load -i ./Build/Cache/plasma.tar || true
./dockerbuild.sh
docker save -o ./Build/Cache/plasma.tar plasma
