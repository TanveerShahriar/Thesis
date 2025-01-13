Install Docker

Give the user permission
sudo usermod -aG docker $USER

Install docker compose v2
sudo apt install -y docker-compose-plugin

docker compose up --build

docker compose up

docker exec -it thesis_container /bin/bash


cd thesis
cd call_graph 

make 
cd build

./CallGraphAnalyzer ../../input/input.cpp 

docker compose down