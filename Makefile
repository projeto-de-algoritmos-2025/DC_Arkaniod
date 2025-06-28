CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
LDFLAGS = -lSDL2 -lSDL2_ttf -lm

TARGET = arkanoid
SOURCES = main.c
OBJECTS = $(SOURCES:.c=.o)

# Diretório para fontes
FONT_DIR = fonts

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Cria o diretório de fontes se não existir
$(FONT_DIR):
	mkdir -p $(FONT_DIR)
	@echo "Por favor, coloque a fonte 'Sony_Sketch_Bold_205.ttf' no diretório $(FONT_DIR)/"

# Regra para executar o jogo
run: $(TARGET) $(FONT_DIR)
	./$(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET)

# Instalação das dependências (Ubuntu/Debian)
install-deps:
	sudo apt-get update
	sudo apt-get install -y libsdl2-dev libsdl2-ttf-dev

help:
	@echo "Comandos disponíveis:"
	@echo "  make        - Compila o jogo"
	@echo "  make run    - Compila e executa o jogo"
	@echo "  make clean  - Remove arquivos compilados"
	@echo "  make install-deps - Instala dependências (Ubuntu/Debian)"
	@echo "  make help   - Mostra esta ajuda" 