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


/////////////////////////////////////

// vetor C =   [A, B, D, F]
// vetor P = [0, 1, 2, 3, 4]

// ao se promover uma chave, acompanhamos sempre o p da DIREITA.


// fun(*a)
//   a+10

///////////////////////////////////////////////////////////////////////////////////////////////////

// Funções auxiliares para o manuseio dos dados

cabecalhoIndice_t* createIndexHeader();
void jumpIndexHeader(FILE* indexFileReference);
void freeIndexHeader(cabecalhoIndice_t* header);
void insertIndexHeaderDataInStructure(char status, int raiz, int proxRRN, cabecalhoIndice_t* header);
void writeIndexHeader(FILE* tableFileReference, cabecalhoIndice_t* header);
int createVehicleIndex(char* tableFileName, char* indexFileName);
int insertIndexRecursive(
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
int getRRNraiz(FILE* indexFileReference) ;
noArvoreB_t* getNode(int RRN, FILE* indexFileReference);
void readIndex(noArvoreB_t* node, FILE* indexFileReference);
void insertSorted(int newKey,long referenciaDoArquivoDeDados, int pDireito, noArvoreB_t* currentNode);
long searchRegisterReference(int key, FILE* indexFileReference);
long searchRegisterReferenceRecursive(int currentRRN, int key, FILE* indexfileReference);
chaves_t* searchKeyInsideIndex(int currentRRN, int key, noArvoreB_t* currentNode); 
int keyIsHere(int key, noArvoreB_t* currentNode); 
//////////////////////////////////////////////////////////////////////////////////////////////////

int createVehicleIndex(char* tableFileName, char* indexFileName){
  
  // abre o arquivo da tabela de veiculos para leitura
  FILE* tableFileReference = fopen(tableFileName, "r");   
  //ARQUIVOS - se não conseguir abrir mostra mensagem de erro de processamento de arquivo
  if(!fileDidOpen(tableFileReference))
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

  char campoRemovido;
  int RRNraiz = -1;
  int proxNo = 0;
  // enquanto houver bytes para serem lidos continue escrevendo
  while (fread(&campoRemovido, sizeof(char), 1, tableFileReference) != 0)
  {
    long referenciaNoArquivoDeDados = ftell(tableFileReference)-1;

    //le cada registro 
    setRemovidoVehicle(vehicleRegister, campoRemovido);
    readVehicleRegisterBIN(tableFileReference, vehicleRegister);
    if (campoRemovido != '0')
    {
      char* prefixo = getPrefixo(vehicleRegister);
      int newKey = convertePrefixo(prefixo);
      insertIndex(newKey, referenciaNoArquivoDeDados , &RRNraiz, &proxNo, indexFileReference);
    }
  }
  
  freeVehicleRegister(vehicleRegister);

  //cria um ponteiro novo para o cabecalho do veiculo
  cabecalhoIndice_t* header = createIndexHeader();
  // preechendo a estrutura cabeçalho com a informação colhida do arquivo csv
  insertIndexHeaderDataInStructure('1', RRNraiz, proxNo, header);
  // Com a estrutura completa, escrevemos ela dentro do arquivo binário
  writeIndexHeader(indexFileReference, header);
  //libera estrutura
  freeIndexHeader(header);
    
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


void insertIndex(int newKey, long referenciaDoArquivoDeDados , int* RRNraiz, int* proxNo, FILE* indexFileReference)
{

  if(treeIsEmpty(*RRNraiz))
  { 
    // situação da árvore vazia, devemos criar o primeiro nó
    noArvoreB_t* rootNode = createBtreeNode();
    insertSorted(newKey, referenciaDoArquivoDeDados, -1, rootNode);

    *RRNraiz++;
    fseek(indexFileReference, (*RRNraiz+1*REGISTER_SIZE), SEEK_SET); // +1 para pular o cabeçalho
    writeIndex(indexFileReference, rootNode);

    freeNode(rootNode);

  }else
  {
    noArvoreB_t* currentRoot =  createBtreeNode();
    fseek(indexFileReference, (*RRNraiz+1)*REGISTER_SIZE, SEEK_SET);
    readIndex(currentRoot, indexFileReference);

    // a nova chave nasce sem filhos a principio
    int ponteiroDireito = -1;
    if(hasSpace(currentRoot))
    { // tendo espaço na raiz basta inserir da maneira convencional
      insertIndexRecursive(&newKey, &referenciaDoArquivoDeDados, &ponteiroDireito, currentRoot->RRNdoNo, proxNo,indexFileReference);
    }else
    {
      // Se a raiz não tem espaço disonível temos que saber quando aumentar a altura da arvore
      if(insertIndexRecursive)
      {
        // promoção 
        noArvoreB_t* newRoot = split(currentRoot, newKey, referenciaDoArquivoDeDados, ponteiroDireito);

        // promoção da chave mais à esquerda da particao direita
        promocaoDeChave(&newKey, &referenciaDoArquivoDeDados, &ponteiroDireito, *proxNo, newRoot);
        
        // Escrita no arquivo de índices
        fseek(indexFileReference, (*proxNo+1)*REGISTER_SIZE, SEEK_SET);
        writeIndex(indexFileReference, newRoot);
        *RRNraiz = *proxNo;
        *proxNo++;

        freeNode(newRoot);  

        fseek(indexFileReference , (currentRoot->RRNdoNo+1)*REGISTER_SIZE, SEEK_SET);
        writeIndex(indexFileReference, currentRoot);
      }
    }
    
    freeNode(currentRoot);
  }
}

int treeIsEmpty(int root)
{
  return root == -1;
}

void writeIndex(FILE* indexFileReference, noArvoreB_t* node)
{
  fwrite(node->folha[0], sizeof(char), TAMANHO_FOLHA, indexFileReference);
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

int insertIndexRecursive(
  int* newKey,
  long* referenciaDoArquivoDeDados,
  int* ponteiroDireito,
  int currentRRN, 
  int* proxNo, 
  FILE* indexFileReference
){
  // encontrando o registro atual pelo RRN +1 (+1 para pular o header)
  fseek(indexFileReference, (currentRRN + 1)*REGISTER_SIZE, SEEK_SET);
  
  noArvoreB_t* currentNode = createBtreeNode();
  readIndex(currentNode, indexFileReference);

  int nextTree;
  if (currentNode->folha[0] == '0') 
  { 
    // ainda nao chegamos na folha, devemos continuar procurando
    // buscar pela chave na sub-árvore filha adequada
    int nextTree = searchNextRRN(*newKey, currentNode);
    freeNode(currentNode);
    insertIndexRecursive(newKey, referenciaDoArquivoDeDados, ponteiroDireito, nextTree, proxNo, indexFileReference);
  }

  // caso base, quando a inserção e promoção ja foram realizadas e estamos voltando na pilha de recursao
  if (*newKey == -1)  return 0; 
  
  // INSERÇÃO 

  // verificar disponibilidade de espaço  
  if(hasSpace(currentNode))
  {// caso ajá espaço ná página 
    insertSorted(newKey, referenciaDoArquivoDeDados, ponteiroDireito, currentNode);
    *newKey = -1;// fim da inserção
  } else
  { // sem espaço: necessidade do split
    // promoção 
    noArvoreB_t* newNode = split(currentNode, newKey, referenciaDoArquivoDeDados, ponteiroDireito);

    // promoção da chave mais à esquerda da particao direita
    promocaoDeChave(newKey, referenciaDoArquivoDeDados, ponteiroDireito, *proxNo, newNode);
    
    // Escrita no arquivo de índices
    fseek(indexFileReference, (*proxNo+1)*REGISTER_SIZE, SEEK_SET);
    writeIndex(indexFileReference, newNode);
    *proxNo++;
    freeNode(newNode);  
  }

  fseek(indexFileReference , (currentRRN+1)*REGISTER_SIZE, SEEK_SET);
  writeIndex(indexFileReference, currentNode);
  freeNode(currentNode);

  // houve promoção
  return 1;
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

  newNode->folha[0] = '1';
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
  noArvoreB_t* newNode = createBtreeNode();
  int ocupacaoMin = (M-1)/2;
  for (int i = 0; i < ocupacaoMin; i++)
  {
    // passando as chaves e as referencias para o nó à direita
    newNode->chaves[i]->c = currentNode->chaves[i+ocupacaoMin]->c;
    newNode->chaves[i]->pr = currentNode->chaves[i+ocupacaoMin]->pr;
    // apagando as chaves e referencias da pagina à esquerdsa
    currentNode->chaves[i+ocupacaoMin]->c = EMPTY_FIELD;
    currentNode->chaves[i+ocupacaoMin]->pr = EMPTY_FIELD;

    // mesmo processo com os ponteiros
    newNode->ponteiros[i] = currentNode->ponteiros[i+ocupacaoMin+1];
    currentNode->ponteiros[i+ocupacaoMin+1] = EMPTY_FIELD;
  }

  insertSorted(newKey, referenciaDoArquivoDeDados, ponteiroDireito, newNode);
  
  return newNode;
}

void promocaoDeChave(
  int* promotedKey, 
  int* referenciaDoArquivoDeDados, 
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
}

void insertSorted(int newKey,long referenciaDoArquivoDeDados, int pDireito,noArvoreB_t* currentNode)
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
  while (idxPonteiros >= idxChaves+1) 
  { // shiftar até a posição em que a nova chave foi inserida
    currentNode->ponteiros[idxPonteiros+1] = currentNode->ponteiros[idxPonteiros];
    idxPonteiros--;
  }
  currentNode->ponteiros[idxPonteiros+1] = pDireito;
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

  // se a chave foi encontrada, retorna a referência apropriada do arq
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
  for (int i = 0; i < currentNode->nroChavesIndexadas; i++)
  {
    if(currentNode->chaves[i]->c == key) return 1;
  }
  return 0;
}
