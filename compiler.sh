docker start BOX
docker exec -ti BOX make clean -C/root/Toolchain
docker exec -it BOX make clean -C/root/
docker exec -it BOX make -C/root/Toolchain
docker exec -ti BOX make -C/root/
docker stop BOX

# ./run.sh
