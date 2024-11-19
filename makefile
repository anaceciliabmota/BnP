# Detecta se o sistema é de 32 ou 64 bits
BITS_OPTION = -m64

# Diretórios com as libs do CPLEX
CPLEXDIR  = /opt/ibm/ILOG/CPLEX_Studio2211/cplex
CONCERTDIR = /opt/ibm/ILOG/CPLEX_Studio2211/concert

CPLEXLIBDIR   = $(CPLEXDIR)/lib/x86-64_linux/static_pic
CONCERTLIBDIR = $(CONCERTDIR)/lib/x86-64_linux/static_pic

# Define o compilador
CPPC = g++

# Opções de compilação e includes
CCOPT = $(BITS_OPTION) -O3 -fPIC -fexceptions -DNDEBUG -DIL_STD -std=c++11
CONCERTINCDIR = $(CONCERTDIR)/include
CPLEXINCDIR   = $(CPLEXDIR)/include
CCFLAGS = $(CCOPT) -I$(CPLEXINCDIR) -I$(CONCERTINCDIR)

# Flags do linker
CCLNFLAGS = -L$(CPLEXLIBDIR) -lilocplex -lcplex -L$(CONCERTLIBDIR) -lconcert -lm -lpthread -ldl

# Diretórios com os source files e com os objs files
SRCDIR = src
OBJDIR = obj

# Lista de todos os srcs e todos os objs
SRCS = $(wildcard $(SRCDIR)/*.cpp)
OBJS = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRCS))

# Regra principal, gera o executável
bpp: $(OBJDIR) $(OBJS)
	@echo  "\033[31m \nLinking all objects files: \033[0m"
	$(CPPC) $(BITS_OPTION) $(OBJS) -o $@ $(CCLNFLAGS)

# Regra para criar o diretório obj
$(OBJDIR):
	mkdir -p $(OBJDIR)

# Regra para cada arquivo objeto: compila e gera o arquivo de dependências
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	@echo  "\033[31m \nCompiling $<: \033[0m"
	$(CPPC) $(CCFLAGS) -c $< -o $@
	@echo  "\033[32m \nCreating $< dependency file: \033[0m"
	$(CPPC) -std=c++11 -MM $< > $(basename $@).d
	@mv -f $(basename $@).d $(basename $@).d.tmp
	@sed -e 's|.*:|$(basename $@).o:|' < $(basename $@).d.tmp > $(basename $@).d
	@rm -f $(basename $@).d.tmp

# Inclui os arquivos de dependências
-include $(OBJS:.o=.d)

# Limpa os arquivos objetos e de dependências
clean:
	@echo "\033[31mCleaning obj directory and binary files\033[0m"
	@rm -rf $(OBJDIR)/*.o $(OBJDIR)/*.d bpp

# Rebuild: limpa e recompila
rebuild: clean bpp
#Nome do Compilador

