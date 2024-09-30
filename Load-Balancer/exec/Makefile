SRC=../implem/
TEST=../simulation/
PROGS=../progs/
BIN=../bin/
MAIN=../main/

CC=gcc
MCC=mpicc

all: loadbalancer smt_loadbalancer $(PROGS)loop

loadbalancer: $(BIN)loadbalancer.o $(BIN)client.o $(BIN)balancer.o $(BIN)inout.o $(BIN)load.o $(BIN)work.o $(BIN)utils.o $(BIN)struct.o
	$(MCC) $(CFLAGS) -o $@ $^

smt_loadbalancer: $(BIN)smt_loadbalancer.o $(BIN)balancer.o $(BIN)inout.o $(BIN)load.o $(BIN)work.o $(BIN)struct.o $(BIN)utils.o
	$(MCC) $(CFLAGS) -o $@ $^


$(PROGS)loop: $(BIN)loop.o
	$(CC) $(CFLAGS) -o $@ $^

$(PROGS)dico: $(BIN)dico.o
	$(CC) $(CFLAGS) -o $@ $^


$(BIN)%.o: $(MAIN)%.c 
	$(MCC) $(CFLAGS) -c $< -o $@

$(BIN)%.o: $(PROGS)%.c | $(BIN)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN)%.o: $(SRC)%.c | $(BIN)
	$(MCC) $(CFLAGS) -c $< -o $@

$(BIN)%.o: $(TEST)%.c | $(BIN)
	$(MCC) $(CFLAGS) -c $< -o $@



clean:
	rm -f loadbalancer smt_loadbalancer $(BIN)*.o
