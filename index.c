#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arquivos.h"
#include "veiculos.h"
#include "linhas.h"
#include "index.h"
#include "convertePrefixo.h"
///////////////////////////////////////////////////////////////////////////////////////////////////
// Tamanho dos campos de cabeçalho e de registro (em Bytes) definidos no documento de requisitos

#define STRING_SIZE 100
#define REGISTER_SIZE 77
#define EMPTY_FIELD -1

#define TAMANHO_STATUS 1
#define TAMANHO_LIXO 68
#define TAMANHO_FOLHA 1

typedef struct cabecalhoIndice
{
  char status[TAMANHO_STATUS];
  int noRaiz;
  int RRNproxNo;
  char lixo[TAMANHO_LIXO];
}cabecalhoIndice_t;

typedef struct chaves{
  int c;
  long pr;  
}chaves_t;

struct noArvoreB
{
  char folha[TAMANHO_FOLHA];
  int nroChavesIndexadas;
  int RRNdoNo;
  chaves_t* chaves[M-1];
  int ponteiros[M];
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Funções auxiliares para o manuseio dos dados
// Cabeçalho


cabecalhoIndice_t* createIndexHeader();
void jumpIndexHeader(FILE* indexFileReference);
void freeIndexHeader(cabecalhoIndice_t* header);
void insertIndexHeaderDataInStructure(char status, int raiz, int proxRRN, cabecalhoIndice_t* header);
void writeIndexHeader(FILE* tableFileReference, cabecalhoIndice_t* header);
// veiculo
int createVehicleIndex(char* tableFileName, char* indexFileName);

// indice
void insertIndexRecursive(
  int* newKey,
  long* referenciaDoArquivoDeDados,
  int* pDireito,
  int currentRRN, 
  int* proxNo, 
  FILE* indexFileReference
);
noArvoreB_t* createBtreeNode();
chaves_t* createChave();
void freeNode(noArvoreB_t* node);
void freeChave(chaves_t* chave); 
int hasSpace(noArvoreB_t* node);
int getRRNraiz(FILE* indexFileReference);
int getRRNproxNo(FILE* indexFileReference);
noArvoreB_t* getNode(int RRN, FILE* indexFileReference);
void readIndex(noArvoreB_t* node, FILE* indexFileReference);
long searchRegisterReference(int key, FILE* indexFileReference);
long searchRegisterReferenceRecursive(int currentRRN, int key, FILE* indexfileReference);
chaves_t* searchKeyInsideIndex(int currentRRN, int key, noArvoreB_t* currentNode); 
int keyIsHere(int key, noArvoreB_t* currentNode); 
void insertIndex(int newKey, long referenciaDoArquivoDeDados , int* rootRRN, int* RRNproxNo, FILE* indexFileReference);
void writeIndex(FILE* indexFileReference, noArvoreB_t* node);
void insertIndexRecursive(
  int* newKey,
  long* referenciaDoArquivoDeDados,
  int* ponteiroDireito,
  int currentRRN, 
  int* RRNproxNo, 
  FILE* indexFileReference
);
noArvoreB_t* split(
  noArvoreB_t* currentNode,
  int newKey, 
  int referenciaDoArquivoDeDados, 
  int ponteiroDireito);
void promocaoDeChave(
  int* promotedKey, 
  long* referenciaDoArquivoDeDados, 
  int* ponteiroDireito, int proxNo, 
  noArvoreB_t* newNode);
void insertSortedInNode(int newKey,long referenciaDoArquivoDeDados, int pDireito,noArvoreB_t* currentNode);
void insertSortedInArray(int newKey,long referenciaDoArquivoDeDados, int pDireito, chaves_t* chaves[], int ponteiros[]);
int searchNextRRN(int key, noArvoreB_t* currentNode);

//////////////////////////////////////////////////////////////////////////////////////////////////

int createVehicleIndex(char* tableFileName, char* indexFileName){
  
  // abre o arquivo da tabela de veiculos para leitura
  FILE* tableFileReference = fopen(tableFileName, "r");   
  //ARQUIVOS - se não conseguir abrir mostra mensagem de erro de processamento de arquivo
  if(!fileDidOpen(tableFileReference) || isFileCorrupted(tableFileReference))
  {
    printf("Falha no processamento do arquivo.\n");
    return 0;
  } 

  //cria o arquivo para insercao de índices e o abre para escrita
  FILE* indexFileReference = fopen(indexFileName, "wb+");
  //ARQUIVOS - se não conseguir criar e inserir mostra mensagem de erro de processamento de arquivo
  if (!fileDidOpen(indexFileReference))
  {
    printf("Falha no processamento do arquivo.\n");
    return 0;
  }
  
  //ARQUIVOS - insere o status do arquivo de abertura como 0, para verificar que o arquivo foi manuseado de forma
  // adequada 
  setStatus(indexFileReference, '0');

  //VEICULOS - seta o ponteiro do arquivo para depois do header do arquivo de veículos
  jumpVehicleHeader(tableFileReference);
  //INDICES - seta o ponteiro do arquivo para depois do header do arquivo de índices
  jumpIndexHeader(indexFileReference);

  // cria um registro de veiculos vazio
  vehicle_t* vehicleRegister = createVehicleRegister();


  int noRaiz = -1;
  int RRNproxNo = 0;
  
  char campoRemovido;
  // enquanto houver bytes para serem lidos continue escrevendo
  while (fread(&campoRemovido, sizeof(char), 1, tableFileReference) != 0)
  {
    // voltar 1 byte para pegar a referencia desde o inicio do registro
    // contendo o campo removido que foi lido na verificação do loop;
    long referenciaNoArquivoDeDados = ftell(tableFileReference)-1;

    //le cada registro 
    setRemovidoVehicle(vehicleRegister, campoRemovido);
    readVehicleRegisterBIN(tableFileReference, vehicleRegister);
    if (campoRemovido != '0')
    {
      char* prefixo = getPrefixo(vehicleRegister);
      int newKey = convertePrefixo(prefixo);
      insertIndex(newKey, referenciaNoArquivoDeDados , &noRaiz, &RRNproxNo, indexFileReference);
    }
  }
  
  //cria um ponteiro novo para o cabecalho do veiculo
  cabecalhoIndice_t* header = createIndexHeader();
  insertIndexHeaderDataInStructure('1',noRaiz, RRNproxNo, header);
  // Com a estrutura completa, escrevemos ela dentro do arquivo binário
  writeIndexHeader(indexFileReference, header);
  
  //libera estrutura
  freeIndexHeader(header);
  freeVehicleRegister(vehicleRegister);
    
  //fecha arquivos
  fclose(tableFileReference);
  fclose(indexFileReference);

  return 1;
}

int createLineIndex(char* tableFileName, char* indexFileName){
  
  // abre o arquivo da tabela de veiculos para leitura
  FILE* tableFileReference = fopen(tableFileName, "r");   
  //ARQUIVOS - se não conseguir abrir mostra mensagem de erro de processamento de arquivo
  if(!fileDidOpen(tableFileReference) || isFileCorrupted(tableFileReference))
  {
    printf("Falha no processamento do arquivo.\n");
    return 0;
  } 

  //cria o arquivo para insercao de índices e o abre para escrita
  FILE* indexFileReference = fopen(indexFileName, "wb+");
  //ARQUIVOS - se não conseguir criar e inserir mostra mensagem de erro de processamento de arquivo
  if (!fileDidOpen(indexFileReference))
  {
    printf("Falha no processamento do arquivo.\n");
    return 0;
  }
  
  //ARQUIVOS - insere o status do arquivo de abertura como 0, para verificar que o arquivo foi manuseado de forma
  // adequada 
  setStatus(indexFileReference, '0');

  //LINHAs - seta o ponteiro do arquivo para depois do header do arquivo de linhas
  jumpLineHeader(tableFileReference);
  //INDICES - seta o ponteiro do arquivo para depois do header do arquivo de índices
  jumpIndexHeader(indexFileReference);

  // cria um registro de veiculos vazio
  line_t* lineRegister = createLineRegister();


  int noRaiz = -1;
  int RRNproxNo = 0;
  
  char campoRemovido;
  // enquanto houver bytes para serem lidos continue escrevendo
  while (fread(&campoRemovido, sizeof(char), 1, tableFileReference) != 0)
  {
    long referenciaNoArquivoDeDados = ftell(tableFileReference)-1;

    //le cada registro 
    setRemovidoLine(lineRegister, campoRemovido);
    readLineRegisterBIN(tableFileReference, lineRegister);
    if (campoRemovido != '0')
    {
      int newKey = getCodLinha(lineRegister);
      insertIndex(newKey, referenciaNoArquivoDeDados , &noRaiz, &RRNproxNo, indexFileReference);
    }
  }
  
  //cria um ponteiro novo para o cabecalho do veiculo
  cabecalhoIndice_t* header = createIndexHeader();
  insertIndexHeaderDataInStructure('1',noRaiz, RRNproxNo, header);
  // Com a estrutura completa, escrevemos ela dentro do arquivo binário
  writeIndexHeader(indexFileReference, header);
  
  //libera estrutura
  freeIndexHeader(header);
  freeLineRegister(lineRegister);
    
  //fecha arquivos
  fclose(tableFileReference);
  fclose(indexFileReference);

  return 1;
}


cabecalhoIndice_t* createIndexHeader(){
  cabecalhoIndice_t* newHeader = (cabecalhoIndice_t*)malloc(sizeof(cabecalhoIndice_t));
  if (newHeader == NULL) {
    perror("Falha em alocar memória pra o cabeçalho");
    exit(EXIT_FAILURE);
  }
  return newHeader;
}

// seta o ponteiro do arquivo para depois do header
void jumpIndexHeader(FILE* indexFileReference)
{
  fseek(indexFileReference, REGISTER_SIZE+1, SEEK_SET);
}

void freeIndexHeader(cabecalhoIndice_t* header)
{
  free(header);
}

void insertIndexHeaderDataInStructure(
  char status, 
  int raiz, 
  int proxRRN, 
  cabecalhoIndice_t* header
){
  header->status[0] = status;
  header->noRaiz = raiz;
  header->RRNproxNo = proxRRN;
  for (int i = 0; i < TAMANHO_LIXO; i++)
  {
    header->lixo[i] = '@';
  }
}

void writeIndexHeader(FILE* indexFileReference, cabecalhoIndice_t* header){
  goToFileStart(indexFileReference);
  
  fwrite(header->status, sizeof(char), 1, indexFileReference);
  fwrite(&header->noRaiz, sizeof(int), 1, indexFileReference);
  fwrite(&header->RRNproxNo, sizeof(int), 1, indexFileReference);
  fwrite(header->lixo, sizeof(char), TAMANHO_LIXO, indexFileReference);
}


void insertIndex(int newKey, long referenciaDoArquivoDeDados , int* rootRRN, int* RRNproxNo, FILE* indexFileReference)
{
  // todo novo nó nasce sem filhos
  int newP = -1;

  insertIndexRecursive(&newKey, &referenciaDoArquivoDeDados, &newP, *rootRRN, RRNproxNo, indexFileReference);

  if (newKey != -1)
  {
    // Há uma promoção pendente, logo é
    // necessário aumentar o nível da árvore
    noArvoreB_t* newRoot = createBtreeNode();
    insertSortedInNode(newKey, referenciaDoArquivoDeDados, newP, newRoot);
    // nova rais 'aponta' para raiz anterior
    newRoot->ponteiros[0] = *rootRRN;
    newRoot->ponteiros[1] = newP;
    // se a arvore estava fazia trata-se de uma raiz folha
    newRoot->folha[0] = (*rootRRN == -1)? '1' : '0';
    newRoot->RRNdoNo = *RRNproxNo;

    // atualizando referencia prar no raiz e prox no
    *rootRRN = *RRNproxNo;
    *RRNproxNo += 1;
    fseek(indexFileReference, (*rootRRN+1)*REGISTER_SIZE, SEEK_SET);
    writeIndex(indexFileReference, newRoot);
    free(newRoot);
  }
}

void writeIndex(FILE* indexFileReference, noArvoreB_t* node)
{
  fwrite(node->folha, sizeof(char), TAMANHO_FOLHA, indexFileReference);
  fwrite(&node->nroChavesIndexadas, sizeof(int), 1, indexFileReference);
  fwrite(&node->RRNdoNo, sizeof(int), 1, indexFileReference);
  for (int i = 0; i < M-1; i++)
  {
    fwrite(&node->ponteiros[i], sizeof(int), 1, indexFileReference);
    fwrite(&node->chaves[i]->c, sizeof(int), 1, indexFileReference);
    fwrite(&node->chaves[i]->pr, sizeof(long), 1, indexFileReference);
  }
  fwrite(&node->ponteiros[M-1], sizeof(int), 1, indexFileReference);
}

void insertIndexRecursive(
  int* newKey,
  long* referenciaDoArquivoDeDados,
  int* ponteiroDireito,
  int currentRRN, 
  int* RRNproxNo, 
  FILE* indexFileReference
){

  // caso base, sub-arvore vazia
  if(currentRRN == -1) return;

  // encontrando o registro atual pelo RRN +1 (+1 para pular o header)
  fseek(indexFileReference, (currentRRN + 1)*REGISTER_SIZE, SEEK_SET);
  noArvoreB_t* currentNode = createBtreeNode();
  readIndex(currentNode, indexFileReference);

  // ainda nao chegamos na folha, devemos continuar procurando
  // buscar pela chave na sub-árvore filha adequada
  int nextTree;
  nextTree = searchNextRRN(*newKey, currentNode);
  insertIndexRecursive(newKey, referenciaDoArquivoDeDados, ponteiroDireito, nextTree, RRNproxNo, indexFileReference);

  // caso base, quando a inserção e promoção ja foram realizadas e estamos voltando na pilha de recursao
  if (*newKey == -1) 
  {
    freeNode(currentNode);
    return; 
  }
  
  // INSERÇÃO 

  // verificar disponibilidade de espaço  
  if(hasSpace(currentNode))
  {// caso ajá espaço ná página 
    insertSortedInNode(*newKey, *referenciaDoArquivoDeDados, *ponteiroDireito, currentNode);
    *newKey = -1;// fim da inserção
  } else
  { // sem espaço: necessidade do SPLIT
    // PROMOÇÃO
    noArvoreB_t* newNode = split(currentNode, *newKey, *referenciaDoArquivoDeDados, *ponteiroDireito);
    newNode->RRNdoNo = *RRNproxNo;
    // promoção da chave mais à esquerda da particao direita
    promocaoDeChave(newKey, referenciaDoArquivoDeDados, ponteiroDireito, *RRNproxNo, newNode);
    
    // Escrita no arquivo de índices
    fseek(indexFileReference, (*RRNproxNo+1)*REGISTER_SIZE, SEEK_SET);
    writeIndex(indexFileReference, newNode);
    *RRNproxNo += 1;
    freeNode(newNode);  
  }

  fseek(indexFileReference , (currentRRN+1)*REGISTER_SIZE, SEEK_SET);
  writeIndex(indexFileReference, currentNode);
  freeNode(currentNode);

  // houve promoção
  return;
}

noArvoreB_t* createBtreeNode() 
{
  noArvoreB_t* newNode = (noArvoreB_t*)malloc(sizeof(noArvoreB_t));
  if(!newNode)
  {
    perror("falha ao alocar memoria para o nó da arvore");
    exit(EXIT_FAILURE);
  } 
  for (int i = 0; i < M-1; i++)
  {
    newNode->chaves[i] = createChave();
    newNode->chaves[i]->c = -1;
    newNode->chaves[i]->pr = -1;
    newNode->ponteiros[i] = -1;
  }
  newNode->ponteiros[M-1] = -1;

  newNode->folha[0] = '0';
  newNode->nroChavesIndexadas = 0;
  newNode->RRNdoNo = -1;

  return newNode;
}

chaves_t* createChave()
{
  chaves_t* newChave = (chaves_t*)malloc(sizeof(chaves_t));
  if(!newChave)
  {
    perror("falha ao alocar memoria para o nó da arvore");
    exit(EXIT_FAILURE);
  }
  newChave->c = -1;
  newChave->pr = -1;
  return newChave;
}

void freeNode(noArvoreB_t* node)
{
  for (int i = 0; i < M-1; i++)
  {
    free(node->chaves[i]);
  }
  
  free(node);
}

void freeChave(chaves_t* chave) 
{
  free(chave);
}

void readIndex(noArvoreB_t* node, FILE* indexFileReference)
{
  fread(node->folha, sizeof(char), TAMANHO_FOLHA, indexFileReference);
  fread(&node->nroChavesIndexadas, sizeof(int), 1, indexFileReference);
  fread(&node->RRNdoNo, sizeof(int), 1, indexFileReference);
  
  for (int i = 0; i < M-1; i++)
  {
    fread(&node->ponteiros[i], sizeof(int), 1, indexFileReference);
    fread(&node->chaves[i]->c, sizeof(int), 1, indexFileReference);
    fread(&node->chaves[i]->pr, sizeof(long), 1, indexFileReference);
  }
  fread(&node->ponteiros[M]-1, sizeof(int), 1, indexFileReference);
}

int searchNextRRN(int key, noArvoreB_t* currentNode)
{
  // buscar pela chave na sub-árvore filha adequada
  int nextTree;
  for (int i = 0; i < currentNode->nroChavesIndexadas; i++)
  {
    // se for menor devemos ir para a subarvore esquerda
    if(key < currentNode->chaves[i]->c){
      nextTree = currentNode->ponteiros[i];
      return nextTree;
    }
  }
  // caso nenhuma seja menor, ir pela ultima subarvore à direita
  nextTree = currentNode->ponteiros[currentNode->nroChavesIndexadas];
  return nextTree;
}

int hasSpace(noArvoreB_t* node)
{
  return (node->nroChavesIndexadas < M-1);
}

int getRRNraiz(FILE* indexFileReference) 
{
  // pular o status
  fseek(indexFileReference, sizeof(char), SEEK_SET);

  int RRNraiz;
  fread(&RRNraiz, sizeof(int), 1, indexFileReference);
  return RRNraiz;
}

int getRRNproxNo(FILE* indexFileReference)
{
  // pular o status e RRNraiz
  fseek(indexFileReference, sizeof(char)+sizeof(int), SEEK_SET);

  int RRNproxNo;
  fread(&RRNproxNo, sizeof(int), 1, indexFileReference);
  return RRNproxNo;
}

noArvoreB_t* getNode(int RRN, FILE* indexFileReference)
{
  fseek(indexFileReference, REGISTER_SIZE*RRN+1, SEEK_SET); // +1 para pular o cabeçalho

  noArvoreB_t* node = createBtreeNode();
  readIndex(node, indexFileReference);

  return node;
}

noArvoreB_t* split(
  noArvoreB_t* currentNode,
  int newKey, 
  int referenciaDoArquivoDeDados, 
  int ponteiroDireito) 
{
  // vetores auxiliares com tamanho igual à capacidade max +1, para se insereir o novo elemento
  // inserir de maneira ordenada neles para nao precisar se preocupar em descobrir se a nova chave 
  // deverá ser posicionada na arvore esquerda ou na arvore direita do nó
  chaves_t* chaves[M]; 
  for (int i = 0; i < M; i++)
  { // incializando valores para evitar segfault
    chaves[i] = createChave();
  }
  
  // recebe todos os elementos do no cheio + a nova chave e o novo ponteiro
  int ponteiros[M+1];
  for (int i = 0; i < M-1; i++)
  {
    chaves[i]->c = currentNode->chaves[i]->c;
    chaves[i]->pr = currentNode->chaves[i]->pr;
    ponteiros[i] = currentNode->ponteiros[i];
  }
  ponteiros[M-1] = currentNode->ponteiros[M-1];
  insertSortedInArray(newKey, referenciaDoArquivoDeDados, ponteiroDireito, chaves, ponteiros);

  // sobrescrever os valores na pagina atual e na nova que sera criada

  // criação do novo nó que recebe as M-1/2 + 1 chaves mais à direita
  noArvoreB_t* newNode = createBtreeNode();
  int ocupacaoMin = (M-1)/2;
  for (int i = 0; i <= ocupacaoMin; i++)
  {
    // passando as chaves e as referencias para o nó à direita
    newNode->chaves[i]->c = chaves[i+ocupacaoMin]->c;
    newNode->chaves[i]->pr = chaves[i+ocupacaoMin]->pr;
    // mesmo processo com os ponteiros
    newNode->ponteiros[i] = ponteiros[i+ocupacaoMin+1];
    newNode->nroChavesIndexadas++;
  }

  // chave original recebe os (M-1)/2 filhos mais à esquerda
  // resetando as chaves e referencias que não pertencem mais 
  // à página esquerda (-1s)
  for (int i = 0; i < ocupacaoMin; i++)
  {
    currentNode->chaves[i]->c = chaves[i]->c;
    currentNode->chaves[i]->pr = chaves[i]->pr;
    currentNode->ponteiros[i] = ponteiros[i];

    currentNode->chaves[i+ocupacaoMin]->c = EMPTY_FIELD;
    currentNode->chaves[i+ocupacaoMin]->pr = EMPTY_FIELD;
    currentNode->ponteiros[i+ocupacaoMin+1] = EMPTY_FIELD;
    currentNode->nroChavesIndexadas--;
  }  
  currentNode->ponteiros[currentNode->nroChavesIndexadas] = ponteiros[currentNode->nroChavesIndexadas];

  newNode->folha[0] =  (currentNode->folha[0] == '1')? '1' : '0';

  for (int i = 0; i < M; i++)
  {
    freeChave(chaves[i]);
  }

  return newNode;
}

void promocaoDeChave(
  int* promotedKey, 
  long* referenciaDoArquivoDeDados, 
  int* ponteiroDireito, int proxNo, 
  noArvoreB_t* newNode)
{
  *promotedKey = newNode->chaves[0]->c;
  *referenciaDoArquivoDeDados = newNode->chaves[0]->pr;
  *ponteiroDireito = proxNo;

  // 'shiftar' as chaves do novo nó para a esquerda
  int idxChaves = 0;
  while (idxChaves < newNode->nroChavesIndexadas) {
    newNode->chaves[idxChaves]->c = newNode->chaves[idxChaves+1]->c;
    newNode->chaves[idxChaves]->pr = newNode->chaves[idxChaves+1]->pr;
    idxChaves++;
  }
  newNode->nroChavesIndexadas--;
}

void insertSortedInNode(int newKey,long referenciaDoArquivoDeDados, int pDireito,noArvoreB_t* currentNode)
{  
  // inserindo a nova chave ordenadamente, acompanhada da referencia do arquivo de dados
  // inspirado no algoritmo insertion sort
  // trate-se de um "shift pra direita" dos elementos ate achar a posição correta da nova chave
  int idxChaves = currentNode->nroChavesIndexadas - 1;
  while (idxChaves >= 0 && currentNode->chaves[idxChaves]->c > newKey) {
    currentNode->chaves[idxChaves+1]->c = currentNode->chaves[idxChaves]->c;
    currentNode->chaves[idxChaves+1]->pr = currentNode->chaves[idxChaves]->pr;
    idxChaves--;
  }
  currentNode->chaves[idxChaves+1]->c = newKey;
  currentNode->chaves[idxChaves+1]->pr = referenciaDoArquivoDeDados;
  currentNode->nroChavesIndexadas++;

  // 'shiftando' os ponteiros das subarvores da direita uma unidade ->
  int idxPonteiros = currentNode->nroChavesIndexadas-1;
  while (idxPonteiros > idxChaves+1) 
  { // shiftar até a posição em que a nova chave foi inserida
    currentNode->ponteiros[idxPonteiros+1] = currentNode->ponteiros[idxPonteiros];
    idxPonteiros--;
  }
  currentNode->ponteiros[idxPonteiros+1] = pDireito;
}

void insertSortedInArray(int newKey,long referenciaDoArquivoDeDados, int pDireito, chaves_t* chaves[], int ponteiros[])
{  
  // inserindo a nova chave ordenadamente, acompanhada da referencia do arquivo de dados
  // inspirado no algoritmo insertion sort
  // trate-se de um "shift pra direita" dos elementos ate achar a posição correta da nova chave
  int idxChaves = M - 2;
  while (idxChaves >= 0 && chaves[idxChaves]->c > newKey) {
    chaves[idxChaves+1]->c = chaves[idxChaves]->c;
    chaves[idxChaves+1]->pr = chaves[idxChaves]->pr;
    idxChaves--;
  }
  chaves[idxChaves+1]->c = newKey;
  chaves[idxChaves+1]->pr = referenciaDoArquivoDeDados;

  // 'shiftando' os ponteiros das subarvores da direita uma unidade ->
  int idxPonteiros = M-1;
  while (idxPonteiros > idxChaves+1) 
  { // shiftar até a posição em que a nova chave foi inserida
    ponteiros[idxPonteiros+1] = ponteiros[idxPonteiros];
    idxPonteiros--;
  }
  ponteiros[idxPonteiros+1] = pDireito;
}

int sgdbVehicles(char* tableFileName, char* indexFileName, char* searchValue) 
{
  // abre o arquivo da tabela de veiculos para leitura
  FILE* tableFileReference = fopen(tableFileName, "r");   
  //ARQUIVOS - se não conseguir abrir mostra mensagem de erro de processamento de arquivo
  if(!fileDidOpen(tableFileReference))
  {
    printf("Falha no processamento do arquivo.\n");
    return 0;
  } 

  //cria o arquivo para insercao de índices e o abre para escrita
  FILE* indexFileReference = fopen(indexFileName, "r");
  //ARQUIVOS - se não conseguir criar e inserir mostra mensagem de erro de processamento de arquivo
  if (!fileDidOpen(indexFileReference))
  {
    printf("Falha no processamento do arquivo.\n");
    return 0;
  }

  int key = convertePrefixo(searchValue);
  long referenciaDoRegistro = searchRegisterReference(key, indexFileReference);

  // Se não encontrado retornar 0
  if(referenciaDoRegistro == -1) 
  {// registro removido
    printf("Registro inexistente.\n");
    return 0;
  }

  // Registro encontrado

  // cria um registro de veiculos vazio
  vehicle_t* vehicleRegister = createVehicleRegister();
  
  // vai ate a posicao adequado no arquivo de dados
  fseek(tableFileReference, referenciaDoRegistro, SEEK_SET);
  // Verificar se foi removido
  char campoRemovido;
  fread(&campoRemovido, sizeof(char), 1, tableFileReference);
  if (campoRemovido == '0')
  {// registro removido
    printf("Registro inexistente.\n");
    return 0;
  }
  
  setRemovidoVehicle(vehicleRegister, campoRemovido);
  readVehicleRegisterBIN(tableFileReference, vehicleRegister);

  printVehicleRegister(vehicleRegister);

  //libera estruturas
  freeVehicleRegister(vehicleRegister);
  
  //fecha arquivos
  fclose(tableFileReference);
  fclose(indexFileReference);

  return 1;
}

long searchRegisterReference(int key, FILE* indexFileReference)
{
   // Obtendo o RRN da "raiz" da árvore
  int raiz = getRRNraiz(indexFileReference);

  // começar a busca pela raiz
  return searchRegisterReferenceRecursive(raiz, key, indexFileReference);
}

long searchRegisterReferenceRecursive(int currentRRN, int key, FILE* indexfileReference)
{
  // Caso base, não encontrou
  if (currentRRN == -1) return -1;

  // Procura pela chave de busca na página atual
  noArvoreB_t *currentNode = createBtreeNode();
  fseek(indexfileReference, (currentRRN+1)*REGISTER_SIZE, SEEK_SET);
  readIndex(currentNode, indexfileReference);

  chaves_t* chaveBuscada = searchKeyInsideIndex(currentRRN, key, currentNode);

  // se a chave foi encontrada, retorna a referência apropriada do arquivo
  if(chaveBuscada != NULL) 
  {
    long referenciaDoRegistro = chaveBuscada->pr;
    freeNode(currentNode);
    return referenciaDoRegistro;
  }

  // buscar pela chave na sub-árvore filha adequada
  int nextTree = searchNextRRN(key, currentNode);
  freeNode(currentNode);
  return searchRegisterReferenceRecursive(nextTree, key, indexfileReference);
} 

chaves_t* searchKeyInsideIndex(int currentRRN, int key, noArvoreB_t* currentNode) 
{
  for (int i = 0; i < currentNode->nroChavesIndexadas; i++)
  {
    if (currentNode->chaves[i]->c == key)
    {
      return currentNode->chaves[i];
    }
  }
  
  return NULL;
}

int sgdbLines(char* tableFileName, char* indexFileName, int searchValue) 
{
  // abre o arquivo da tabela de veiculos para leitura
  FILE* tableFileReference = fopen(tableFileName, "r");   
  //ARQUIVOS - se não conseguir abrir mostra mensagem de erro de processamento de arquivo
  if(!fileDidOpen(tableFileReference))
  {
    printf("Falha no processamento do arquivo.\n");
    return 0;
  } 

  //cria o arquivo para insercao de índices e o abre para escrita
  FILE* indexFileReference = fopen(indexFileName, "r");
  //ARQUIVOS - se não conseguir criar e inserir mostra mensagem de erro de processamento de arquivo
  if (!fileDidOpen(indexFileReference))
  {
    printf("Falha no processamento do arquivo.\n");
    return 0;
  }

  long referenciaDoRegistro = searchRegisterReference(searchValue, indexFileReference);

  // Se não encontrado retornar 0
  if(referenciaDoRegistro == -1) 
  {// registro removido
    printf("Registro inexistente.\n");
    return 0;
  }

  // Registro encontrado

  // cria um registro de linha vazio
  line_t* lineRegister = createLineRegister();
  
  // vai ate a posicao adequado no arquivo de dados
  fseek(tableFileReference, referenciaDoRegistro, SEEK_SET);
  // Verificar se foi removido
  char campoRemovido;
  fread(&campoRemovido, sizeof(char), 1, tableFileReference);
  if (campoRemovido == '0')
  {// registro removido
    printf("Registro inexistente.\n");
    return 0;
  }
  
  setRemovidoLine(lineRegister, campoRemovido);
  readLineRegisterBIN(tableFileReference, lineRegister);

  printLineRegister(lineRegister);

  //libera estruturas
  freeLineRegister(lineRegister);
  
  //fecha arquivos
  fclose(tableFileReference);
  fclose(indexFileReference);

  return 1;
}

int keyIsHere(int key, noArvoreB_t* currentNode) 
{
  // se alguma das chaves for igual à key buscada retorna true (1)
  // caso contrario false (0)
  for (int i = 0; i < currentNode->nroChavesIndexadas; i++)
  {
    if(currentNode->chaves[i]->c == key) return 1;
  }
  return 0;
}

int insertNewVehicleRegisters(
  char* indexFileName, 
  char* tableFileName,
  vehicle_t** vehicleRegisters, 
  int numberOfRegisters
)
{
  FILE* indexFileReference = fopen(indexFileName, "rb+");
  if(!fileDidOpen(indexFileReference) 
    || isFileCorrupted(indexFileReference))
  {
    printf("Falha no processamento do arquivo.\n");
    return 0;
  }

  setStatus(indexFileReference, '0');

  FILE* tableFileReference = fopen(tableFileName, "rb+");
  if(!fileDidOpen(tableFileReference) 
    || isFileCorrupted(tableFileReference))
  {
    printf("Falha no processamento do arquivo.\n");
    return 0;
  }

  setStatus(tableFileReference, '0');

 
  int noRaiz = getRRNraiz(indexFileReference);
  int RRNproxNo = getRRNproxNo(indexFileReference);
  for (int i = 0; i < numberOfRegisters; i++)
  {
    // insere o novo registro no arquivo de dados e recupera a referencia de onde foi inserido
    long referenciaNoArquivoDeDados;
    if(!insertVehicleRegisterIntoTable(tableFileReference, vehicleRegisters[i], &referenciaNoArquivoDeDados)) return 0;
    

    char* prefixo = getPrefixo(vehicleRegisters[i]);
    int newKey = convertePrefixo(prefixo);
    insertIndex(newKey, referenciaNoArquivoDeDados , &noRaiz, &RRNproxNo, indexFileReference);
  }
 
  // atualizar as informações do cabeçalho
  cabecalhoIndice_t* newHeader = createIndexHeader();
  insertIndexHeaderDataInStructure('1', noRaiz, RRNproxNo, newHeader);
  writeIndexHeader(indexFileReference, newHeader);

  freeIndexHeader(newHeader);

  fclose(indexFileReference);
  // ocorrendo tudo de acordo, resetar o status para 1 ao fim da função
  setStatus(tableFileReference, '1');
  fclose(tableFileReference);

  return 1;
}


int insertNewLineRegisters(
  char* indexFileName, 
  char* tableFileName,
  line_t** lineRegisters, 
  int numberOfRegisters
)
{
  
  FILE* indexFileReference = fopen(indexFileName, "rb+");
  if(!fileDidOpen(indexFileReference) 
    || isFileCorrupted(indexFileReference))
  {
    printf("Falha no processamento do arquivo.\n");
    return 0;
  }

  setStatus(indexFileReference, '0');

  FILE* tableFileReference = fopen(tableFileName, "rb+");
  if(!fileDidOpen(tableFileReference) 
    || isFileCorrupted(tableFileReference))
  {
    printf("Falha no processamento do arquivo.\n");
    return 0;
  }

  setStatus(tableFileReference, '0');

 
  int noRaiz = getRRNraiz(indexFileReference);
  int RRNproxNo = getRRNproxNo(indexFileReference);
  for (int i = 0; i < numberOfRegisters; i++)
  {
    // insere o novo registro no arquivo de dados e recupera a referencia de onde foi inserido
    long referenciaNoArquivoDeDados;
    if(!insertLineRegisterIntoTable(tableFileReference, lineRegisters[i], &referenciaNoArquivoDeDados)) return 0;
    
    int newKey = getCodLinha(lineRegisters[i]);
    insertIndex(newKey, referenciaNoArquivoDeDados , &noRaiz, &RRNproxNo, indexFileReference);
  }
 
  // atualizar as informações do cabeçalho
  cabecalhoIndice_t* newHeader = createIndexHeader();
  insertIndexHeaderDataInStructure('1', noRaiz, RRNproxNo, newHeader);
  writeIndexHeader(indexFileReference, newHeader);

  freeIndexHeader(newHeader);

  fclose(indexFileReference);

  setStatus(tableFileReference, '1');
  fclose(tableFileReference);

  return 1;
}
