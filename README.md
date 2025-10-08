# cowboy-shootout : Jogo 2D em C++ usando Sockets e Raylib
## Membros do Grupo

- Gabriella Castelari Gonçalves NºUSP : 14755082

- Matheus Lopes Ponciano Lima NºUSP: 14598358

- Murilo Cury Pontes NºUSP: 13830417

- Thaís Laura Anício Andrade NºUSP : 14608765

## Descrição do Projeto
Este projeto é um jogo um contra um 2D. Cada jogador controla um personagem na partida através de seu cliente que estabelece uma conexão com um servidor. O servidor realiza o pareamento entre os clientes, fornece ID's à eles e serve de intermédio dos comandos executados por eles.

## Protoclos Utilizados
- **TCP**: Para relações críticas, como estabelecimento de conexão e inicialização de uma partida.
- **UDP**: Para transmissão de dados de personagem/estado do jogo, como vida e posição atuais.

## Tecnologias Utilizadas

- Linguagem C++ (C++17)

- Biblioteca Gráfica [Raylib](https://www.raylib.com/index.html)

- Bibliotecas de Conexão <netinet/in.h> <arpa/inet.h> <sys/socket.h>

## Como Executar o Projeto

### Pré-Requisitos
1. Tenha o C++ instalado (compilador como 'g++').
2. Baixe e instale a biblioteca gráfica **raylib**.

### Instalação da Raylib
**Linux** 

```
sudo apt install cmake git build-essential
git clone https://github.com/raysan5/raylib.git
cd raylib/src
make PLATFORM=PLATFORM_DESKTOP
sudo make install
```

### Execução
1. Clone Repositório
```
git clone https://github.com/murilocpontes/cowboy-shootout.git
cd cowboy-shootout
```
2. Compile o código
```
make
```
3. Inicie o Servidor em um Terminal
```
./bin/server
```
4. Inicie os Clientes (2+) em Outros Terminais
```
./bin/client
```

## Controles do Jogo:

**<-** Movimenta personagem para cima

**->** Movimenta personagem para baixo

**z** Atira lateral superior

**x** Atira ao centro

**c** Atira lateral inferior

**r** Pronto

**q** Sair
