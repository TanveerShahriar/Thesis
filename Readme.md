Install Docker

Give the user permission
sudo usermod -aG docker $USER

Install docker compose v2
sudo apt install -y docker-compose-plugin

docker compose up --build

docker compose up

docker exec -it thesis_container /bin/bash

cd thesis
make

make clean
docker compose down