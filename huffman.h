#include <cstdio>
#include <algorithm>

typedef unsigned char char_u;
typedef long long i64;

const int MAX_N = 256;
const int INF   = 1000000000;
const int ROOT  = 2 * MAX_N - 2;
const int BUFF  = 8;

int arb[2][2 * MAX_N];      // Tin minte arborele. Avem 2 * N noduri in arbore.
char_u lung[MAX_N];         // Pentru fiecare caracter memorez care este lungimea codului Huffman.
int leaf[2 * MAX_N];        // Tin minte pentru fiecare nod daca este intern sau este frunza
char huffman[MAX_N][MAX_N]; // Pentru fiecare caracter tin minte codul Huffman pentru scriere
char stiva[MAX_N];
int frecv[2 * MAX_N];
int fii[2 * MAX_N];
int papa[2 * MAX_N];
int q1[MAX_N], q2[MAX_N];
int st1, st2, dr1, dr2;

struct Pereche {
  char_u ch, size;
} huff[MAX_N];

bool cmp1(int a, int b) {
  return (frecv[a] < frecv[b]) || (frecv[a] == frecv[b] && a < b);
}

bool cmp2(Pereche a, Pereche b) {
  return a.size < b.size || (a.size == b.size && a.ch < b.ch);
}

/// initializeaza tot arborele
void cleararb() {
  for(int i = 0; i < 2 * MAX_N; ++i) {
    arb[0][i] = arb[1][i] = -1;
    leaf[i] = -1;
    fii[i] = 0;
    papa[i] = -1;
  }
}

/// scoate frecventele caracterelor dintr-un fisier si returneaza marimea fisierului
i64 getfrecv(FILE *file) {
  char_u ch;
  i64 s = 0;
  for(int i = 0; i < MAX_N; ++i)
    frecv[i] = 0;
  while(fread(&ch, 1, sizeof(char_u), file) == 1) {
    frecv[ch]++;
    ++s;
  }
  return s;
}

/// scoate elementul cu frecventa cea mai mica din cele doua cozi
int best() {
  if(st1 < dr1 && st2 < dr2 && frecv[q1[st1]] < frecv[q2[st2]])
    return q1[st1++];
  else if(st1 < dr1 && st2 < dr2)
    return q2[st2++];
  else if(st1 < dr1)
    return q1[st1++];
  else
    return q2[st2++];
}

/// parcurge arborele si atribuie fiecarei frunze lungimea
void dfs(int nod, int dep) {
  if(nod < MAX_N)
    lung[nod] = dep;
  else {
    dfs(arb[0][nod], dep + 1);
    dfs(arb[1][nod], dep + 1);
  }
}

/// construieste arborele Huffman si afla lungimea codurilor pentru fiecare frunza; radacina va fi mereu ultimul nod adaugat
void buildarb() {
  st1 = st2 = dr1 = dr2 = 0;
  for(int i = 0; i < MAX_N; ++i)
    q1[dr1++] = i;

  std::sort(q1, q1 + MAX_N, cmp1);
  cleararb();
  for(int i = 0; i < MAX_N - 1; ++i) {
    int a = best(), b = best(), nod = i + MAX_N;
    frecv[nod] = frecv[a] + frecv[b];
    arb[0][nod] = a;
    arb[1][nod] = b;
    leaf[nod] = -1;
    if(st2 < dr2 && frecv[nod] == frecv[q2[st2]])
      q2[--st2] = nod;
    else
      q2[dr2++] = nod;
  }

  for(int i = 0; i < MAX_N; ++i)
    lung[i] = 0;
  dfs(ROOT, 0);
}

/// copiaza codul de pe stiva in huffman[i]
void cpyhuffman(int i, int n) {
  for(int p = 0; p < lung[i]; ++p)
    huffman[i][p] = stiva[p];
}

/// construieste arborele Huffman canonic cu radacina in pozitia 0
void buildarbcanonic() {
  cleararb();
  for(int i = 0; i < MAX_N; ++i) {
    huff[i].ch = i;
    huff[i].size = lung[i];
  }
  std::sort(huff, huff + MAX_N, cmp2);

  int last = 0, cursor = 0, depth = 0;
  leaf[0] = -2;
  for(int i = 0; i < MAX_N; ++i) {
    while(fii[cursor] == 2) { //de la nodul curent merg in sus pana cand nu mai am fii
      cursor = papa[cursor];
      --depth;
    }
    if(fii[cursor] == 1) { // daca am un fiu merg pe subarborele drept
      fii[cursor] = 2;
      arb[1][cursor] = ++last; //adaug un nod nou
      papa[last] = cursor;
      cursor = last;
      leaf[last] = -2;
      stiva[depth] = 1;
      ++depth;
    }
    while(depth < huff[i].size) { // toate nodurile prin care trec nu vor avea fii
      stiva[depth] = 0;
      ++depth;
      fii[cursor] = 1;
      arb[0][cursor] = ++last;
      papa[last] = cursor;
      cursor = last;
      leaf[last] = -2;
    }
    leaf[cursor] = huff[i].ch;
    cpyhuffman(huff[i].ch, depth);
    cursor = papa[cursor];
    --depth;
  }
}

/// citirea si scrierea bitilor comprimati
char_u octet;
int curs = 0;

/// citeste urmatorul bit din buffer
int getbit(FILE *fin) {
  --curs;
  if(curs == -1) {
    curs = 7;
    fread(&octet, 1, sizeof(char_u), fin);
  }
  if((1 << curs) & octet)
    return 1;
  return 0;
}

/// citeste un caracter comprimat
char_u gethuffchar(FILE *fin) {
  int cursor = 0, branch;
  while(leaf[cursor] < 0) {
    branch = getbit(fin);
    cursor = arb[branch][cursor];
  }
  return leaf[cursor];
}

/// scrie un bit
void putbit(FILE *fout, int b) {
  --curs;
  if(curs == -1) {
    curs = 7;
    fwrite(&octet, 1, sizeof(char_u), fout);
    octet = 0;
  }
  octet = octet | (b << curs);
}

/// scrie un cod Huffman
void putHuff(FILE *fout, char_u ch) {
  for(int i = 0; i < lung[ch]; ++i)
    putbit(fout, huffman[ch][i]);
}

/// modifica cursorul
void setcurs(int x) {
  curs = x;
}

void emptybuff(FILE *fout) {
  fwrite(&octet, 1, sizeof(char_u), fout);
}
