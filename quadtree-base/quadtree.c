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
    RGBPixel(*pixels)[pic->width] = (RGBPixel(*)[pic->height])pic->img;
    RGBPixel(*newPixels)[newPic->width] = (RGBPixel(*)[newPic->height])newPic->img;

    for (size_t i = 0; i < pic->height; i++)
    {
        for (size_t j = 0; j < pic->width; j++)
        {
            RGBPixel *pixel = &pixels[i][j];
            RGBPixel *newPixel = &newPixels[i][j];
            unsigned char cor = (0.3 * pixel->r + 0.59 * pixel->g + 0.11 * pixel->b);
            newPixel->r = cor;
            newPixel->g = cor;
            newPixel->b = cor;
        }
    }
    return newPic;
}

int calculaCorMedia(QuadNode *node, Img *pic)
{
    RGBPixel(*pixels)[pic->width] = (RGBPixel(*)[pic->height])pic->img;
    unsigned int totalR = 0;
    unsigned int totalG = 0;
    unsigned int totalB = 0;

    for (size_t i = node->y; i < node->height + node->y; i++)
    {
        for (size_t j = node->x; j < node->width + node->x; j++)
        {
            RGBPixel *pixel = &pixels[i][j];
            totalR += pixel->r;
            totalG += pixel->g;
            totalB += pixel->b;
        }
    }

    unsigned int numPixels = node->height * node->width;
    node->color[0] = (totalR / numPixels);
    node->color[1] = (totalG / numPixels);
    node->color[2] = (totalB / numPixels);

    return 0;
}

void calculaHistograma(QuadNode *node, Img *pic, long long *histograma)
{
    const int NUM_CINZA = 256;
    RGBPixel(*pixels)[pic->width] = (RGBPixel(*)[pic->height])pic->img;
    for (int i = 0; i < NUM_CINZA; i++)
    {
        histograma[i] = 0;
    }

    for (size_t i = node->y; i < node->y + node->height; i++)
    {
        for (size_t j = node->x; j < node->x + node->width; j++)
        {
            RGBPixel *pixel = &pixels[i][j];
            histograma[pixel->r]++;
        }
    }
}

unsigned char calculaIntensidadeMedia(long long *histograma, int tamanho)
{
    int soma = 0;

    for (size_t i = 0; i < 256; i++)
    {
        soma += histograma[i] * i;
    }
    soma /= tamanho;
    return (unsigned char)soma;
}

int achaIntensidade(int *histograma, int i)
{

    return histograma[i] * i;
}

double calculaErroRegiao(unsigned char intensidadeMedia, QuadNode *node, Img *pic)
{
    float erro = 0.0;
    float soma = 0.0;
    double diferenca = 0.0;
    int tamanho = node->width * node->height;
    RGBPixel(*pixels)[pic->width] = (RGBPixel(*)[pic->width])pic->img;

    for (int i = node->y; i < node->y + node->height; i++)
    {
        for (int j = node->x; j < node->x + node->width; j++)
        {
            RGBPixel *pixel = &pixels[i][j];
            diferenca = pow((double)(pixel->r - intensidadeMedia), 2);
            soma += diferenca;
        }
    }

    erro = sqrt(soma / tamanho);
    return erro;
}

QuadNode *gerarQuadtree(Img *picCinza, float minError, int x, int y, int width, int height, Img *pic)
{
    QuadNode *raiz = newNode(x, y, width, height);
    calculaCorMedia(raiz, pic);
    long long *histogram = malloc(256 * sizeof(long long));

    calculaHistograma(raiz, picCinza, histogram);

    int tamanho = raiz->height * raiz->width;
    unsigned char intensidade = calculaIntensidadeMedia(histogram, tamanho);
    int halfWidth = width / 2;
    int halfHeight = height / 2;

    if (halfHeight<=1 || halfWidth <= 1 || calculaErroRegiao(intensidade, raiz, picCinza) <= minError)
    {
        raiz->status = CHEIO;
        return raiz;
    }

    raiz->status = PARCIAL;
    raiz->NW = gerarQuadtree(picCinza, minError, x, y, halfWidth, halfHeight, pic);
    raiz->NE = gerarQuadtree(picCinza, minError, x + halfWidth, y, halfWidth, halfHeight, pic);
    raiz->SW = gerarQuadtree(picCinza, minError, x, y + halfHeight, halfWidth, halfHeight, pic);
    raiz->SE = gerarQuadtree(picCinza, minError, x + halfWidth, y + halfHeight, halfWidth, halfHeight, pic);

    return raiz;
}

QuadNode *geraQuadtree(Img *pic, float minError)
{
    Img *picCinza = converteParaCinza(pic);
    return gerarQuadtree(picCinza, minError, 0, 0, pic->width, pic->height, pic);
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
