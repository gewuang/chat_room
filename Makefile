chatSrv:chatSrv.cpp
	g++ chatSrv.cpp -o chatSrv
    
chatCli:chatCli.cpp
	g++ chatCli.cpp -o chatCli

clean:
	rm -rf *.o chatSrv chatCli 



