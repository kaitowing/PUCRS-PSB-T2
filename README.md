# Trabalho Prático 2 - Programação de Software Básico

## Compressão de Imagens por Quadtree
Maio de 2023

### Imagens geradas pelo programa:
- Antes da compressão:
![stretched-1920-1080-764519](https://github.com/kaitowing/PUCRS-PSB-T2/assets/105385276/f8328872-02c2-40eb-82d1-6452428c1f6f)

- Pós compressão:
![Screenshot from 2023-06-26 09-43-06](https://github.com/kaitowing/PUCRS-PSB-T2/assets/105385276/c9855359-f6a8-4f47-a8ba-968780ff7655)



### 1. Introdução
Há diversas técnicas que podem ser usadas para a compressão de imagens. As mais conhecidas são os algoritmos implementados no padrão JPEG ou PNG, por exemplo. Porém, há outras técnicas menos conhecidas: uma delas é baseada no uso de uma árvore para a representação da imagem. Essa técnica, conhecida como representação através de subdivisão por ocupação espacial, utiliza uma estrutura de dados denominada quadtree, pois é uma árvore onde cada nodo pode ter zero ou quatro "filhos".

O objetivo deste trabalho é explorar os conceitos de programação em C, bem como o uso de ponteiros em uma estrutura de árvore, criando um programa capaz de ler uma imagem qualquer e gerar uma quadtree correspondente. Para visualizar o resultado, é fornecido um código que desenha a árvore.

### 2. Compilação

### 2.1 Visual Studio Code
Se você estiver utilizando o Visual Studio Code, basta descompactar o zip e abrir a pasta.
- Para compilar: use Ctrl+Shift+B (⌘+Shift+B no macOS).
- Para executar, use F5 para usar o debugger ou Ctrl+F5 para executar sem o debugger.

### 2.2 Outros ambientes ou terminal
Caso esteja usando outro ambiente de desenvolvimento, fornecemos um Makefile para Linux e macOS, e outro para Windows (Makefile.mk).
- Para compilar no Linux ou macOS, basta digitar: ```make```
- Se estiver utilizando o Windows, o comando é similar: ```mingw32-make -f Makefile.mk```

### 3. Execução
Para executar o programa, após a compilação, execute em seu terminal o comando ```./quadtree "nomeDaImagem.tipo"```


