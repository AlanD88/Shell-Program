FLAGS   = -std=c99
SOURCES = myshell.c lex.yy.c
OBJECTS = myshell.o lex.yy.o
EXEBIN  = myshell

all: $(EXEBIN)

$(EXEBIN) : $(OBJECTS)
	cc $(OBJECTS) -lfl -o $(EXEBIN)

$(OBJECTS) : $(SOURCES)
	cc -c $(FLAGS) $(SOURCES)

lex.yy.c: lex.l
	flex lex.l

clean :
	rm -f $(EXEBIN) $(OBJECTS) lex.yy.c
