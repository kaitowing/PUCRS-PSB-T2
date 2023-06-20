#include "quadtree.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h> /* OpenGL functions */
#endif

unsigned int first = 1;
char desenhaBorda = 1;

QuadNode *newNode(int x, int y, int width, int height)
{
    QuadNode *n = malloc(sizeof(QuadNode));
    n->x = x;
    n->y = y;
    n->width = width;
    n->height = height;
    n->NW = n->NE = n->SW = n->SE = NULL;
    n->color[0] = n->color[1] = n->color[2] = 0;
    n->id = first++;
    return n;
}

Img *converteParaCinza(Img *pic)
{
    Img *newPic = malloc(sizeof(Img));
    newPic->width = pic->width;
    newPic->height = pic->height;
    newPic->img = malloc(pic->width * pic->height * sizeof(RGBPixel));
    
    for (size_t i = 0; i < pic->height; i++)
    {
        for (size_t j = 0; j < pic->width; j++)
        {
            RGBPixel *pixel = &pic->img[i * pic->width + j];
            RGBPixel *newPixel = &newPic->img[i * pic->width + j];
            newPixel->r = 0.3 * pixel->r + 0.59 * pixel->g + 0.11 * pixel->b;
            newPixel->g = 0.3 * pixel->r + 0.59 * pixel->g + 0.11 * pixel->b;
            newPixel->b = 0.3 * pixel->r + 0.59 * pixel->g + 0.11 * pixel->b;
            //printf("%c", newPixel->r);
        }
    }
    return newPic;
}


int calculaCorMedia(QuadNode *node, Img *pic)
{
    int tamanhoaux = node->width;
    unsigned char totalR = 0;
    unsigned char totalG = 0;
    unsigned char totalB = 0;

    for (size_t i = node->y; i < node->height + node->y; i++)
    {
        for (size_t j = node->x; j < node->width + node->x; j++)
        {
            RGBPixel *pixel = &pic->img[i * tamanhoaux + j];

            totalR += pixel->r;
            totalG += pixel->g;
            totalB += pixel->b;
        }
    }

    node->color[0] = totalR / ((node->height + 1) * (node->width + 1));
    node->color[1] = totalG / ((node->height + 1) * (node->width + 1));
    node->color[2] = totalB / ((node->height + 1) * (node->width + 1));

    return 0;
}

void calculaHistograma(QuadNode *node, Img *pic, int* histograma)
{
    #define NUM_CINZA 256
    int tamanhoaux = node->width;

    // Inicializa o histograma com zeros
    for (int i = 0; i < NUM_CINZA; i++) {
        histograma[i] = 0;
    }
    

    // Calcula o histograma
    for (size_t i = node->y; i < node->height + node->y; i++)
    {
        for (size_t j = node->x; j < node->width + node->x; j++)
        {
            RGBPixel *cinza = &pic->img[i * tamanhoaux + j];
            histograma[cinza->r]++;
        }
    }
}

int calculaIntensindadeMedia(int* histograma, int tamanho)
{
    #define NUM_CINZA 256
    int soma = 0;
    int divisao = 0;

    for (size_t i = 0; i < 256; i++)
    {
        soma += histograma[i] * i;
    }

    return divisao = soma/tamanho;
}

int calculaErroRegiao(int intensidadeMedia, QuadNode *node, Img *pic, float minError)
{
    int tamanhoaux = node->width;
    double erro = 0;
    double soma = 0;
    double diferenca = 0;

    //printf("Calc Erro");
    for (size_t i = node->y; i < node->height + node->y; i++)
    {
        for (size_t j = node->x; j < node->width + node->x; j++)
        {
            RGBPixel *cinza = &pic->img[i * tamanhoaux + j];
            diferenca = pow(cinza->r - intensidadeMedia, 2);
            soma += diferenca;
        }
    }

    erro = sqrt((1.0 / (node->width * node->height)) * soma);
    return erro <= minError;
}

QuadNode *gerarQuadtree(Img *pic, float minError, int x, int y, int width, int height)
{
    // Aloca memória para armazenar o histograma
    QuadNode *raiz = newNode(x,y,width,height);
    int* histogram = (int*)malloc(256 * sizeof(int));

    calculaHistograma(raiz, pic, histogram);
    int tamanho = raiz->height * raiz->width;
    int intensidade = calculaIntensindadeMedia(histogram, tamanho);
    if (calculaErroRegiao(intensidade, raiz, pic, minError)) {
        //printf("Entrou no if");
        calculaCorMedia(raiz, pic);
        return raiz;
    }
    else {
        int halfWidth = width / 2;
        int halfHeight = height / 2;

        if (halfWidth <= 1 || halfHeight <= 1) {
            raiz->status = CHEIO;
            return raiz;
        }

        // Gera os quadtrees para as quatro partes menores
        raiz->status = PARCIAL;
        raiz->NW = gerarQuadtree(pic, minError, x, y, halfWidth, halfHeight);
        raiz->NE = gerarQuadtree(pic, minError, x + halfWidth, y, halfWidth, halfHeight);
        raiz->SW = gerarQuadtree(pic, minError, x, y + halfHeight, halfWidth, halfHeight);
        raiz->SE = gerarQuadtree(pic, minError, x + halfWidth, y + halfHeight, halfWidth, halfHeight);
    }
    return raiz;
}


QuadNode *geraQuadtree(Img *pic, float minError)
{
    Img* newPic = converteParaCinza(pic);
    return gerarQuadtree(newPic, minError, 0, 0, pic->width, pic->height);
}


// Limpa a memória ocupada pela árvore
void clearTree(QuadNode *n)
{
    if (n == NULL)
        return;
    if (n->status == PARCIAL)
    {
        clearTree(n->NE);
        clearTree(n->NW);
        clearTree(n->SE);
        clearTree(n->SW);
    }
    // printf("Liberando... %d - %.2f %.2f %.2f %.2f\n", n->status, n->x, n->y, n->width, n->height);
    free(n);
}

// Ativa/desativa o desenho das bordas de cada região
void toggleBorder()
{
    desenhaBorda = !desenhaBorda;
    printf("Desenhando borda: %s\n", desenhaBorda ? "SIM" : "NÃO");
}

// Desenha toda a quadtree
void drawTree(QuadNode *raiz)
{
    if (raiz != NULL)
        drawNode(raiz);
}

// Grava a árvore no formato do Graphviz
void writeTree(QuadNode *raiz)
{
    FILE *fp = fopen("quad.dot", "w");
    fprintf(fp, "digraph quadtree {\n");
    if (raiz != NULL)
        writeNode(fp, raiz);
    fprintf(fp, "}\n");
    fclose(fp);
    printf("\nFim!\n");
}

void writeNode(FILE *fp, QuadNode *n)
{
    if (n == NULL)
        return;

    if (n->NE != NULL)
        fprintf(fp, "%d -> %d;\n", n->id, n->NE->id);
    if (n->NW != NULL)
        fprintf(fp, "%d -> %d;\n", n->id, n->NW->id);
    if (n->SE != NULL)
        fprintf(fp, "%d -> %d;\n", n->id, n->SE->id);
    if (n->SW != NULL)
        fprintf(fp, "%d -> %d;\n", n->id, n->SW->id);
    writeNode(fp, n->NE);
    writeNode(fp, n->NW);
    writeNode(fp, n->SE);
    writeNode(fp, n->SW);
}

// Desenha todos os nodos da quadtree, recursivamente
void drawNode(QuadNode *n)
{
    if (n == NULL)
        return;

    glLineWidth(0.1);

    if (n->status == CHEIO)
    {
        glBegin(GL_QUADS);
        glColor3ubv(n->color);
        glVertex2f(n->x, n->y);
        glVertex2f(n->x + n->width - 1, n->y);
        glVertex2f(n->x + n->width - 1, n->y + n->height - 1);
        glVertex2f(n->x, n->y + n->height - 1);
        glEnd();
    }

    else if (n->status == PARCIAL)
    {
        if (desenhaBorda)
        {
            glBegin(GL_LINE_LOOP);
            glColor3ubv(n->color);
            glVertex2f(n->x, n->y);
            glVertex2f(n->x + n->width - 1, n->y);
            glVertex2f(n->x + n->width - 1, n->y + n->height - 1);
            glVertex2f(n->x, n->y + n->height - 1);
            glEnd();
        }
        drawNode(n->NE);
        drawNode(n->NW);
        drawNode(n->SE);
        drawNode(n->SW);
    }
    // Nodos vazios não precisam ser desenhados... nem armazenados!
}
