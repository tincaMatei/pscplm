#include <cstdio>
#include <assert.h>
#include "rlutil.h"
#include "huffman.h"

using namespace rlutil;

const int CHUNK = 256;
const int MAX_BUFF = 256;
const int SIGMA = 26;
const int X_SCREEN = 101;
const int Y_SCREEN = 51;
const int X_MID = X_SCREEN / 2;
const int Y_MID = Y_SCREEN / 2;
const int PX = 10;
const int PY = 5;
const int PX2 = PX + X_MID + X_MID;
const int PY2 = PY + Y_MID + Y_MID;
const int CULORI = 12;
const int MAX_NUME = 100;
const int BAZA = 62;
const int MAX_CHUNKS = 5000;
const int HASH = 666013;
const int X_COORD = 2;
const int Y_COORD = 70;
const int MAX_HUFF = 119;
const int MAX_BITS = 7;
const int PALETA = 16;
int speed = 1;

int tasta;

int culori[] = {BLACK, GREY, WHITE, BLUE,
                RED, GREEN, LIGHTMAGENTA, YELLOW,
                LIGHTCYAN, CYAN, MAGENTA, BROWN};
char nume[MAX_NUME];
char prefix[MAX_NUME] = "canvas/\0";
char cache[X_SCREEN + 1];
struct Keyboard {
  int top;
  char ch[MAX_BUFF];
  Keyboard() {
    top = 0;
  }
  void read() {
    /*for(int i = 0; i < SIGMA; ++i)
      if(getKey(i + 'A'))
        ch[top++] = i + 'a';*/
  }
};

void putChar(int x, int y, char c) {
  gotoxy(x, y);
  setChar(c);
}

void putStr(int x, int y, const char *s) {
  gotoxy(x, y);
  setString(s);
}

void setBgColor(int x) {
  printf("\033[48;5;%dm", x);
}

void setFgColor(int x) {
  printf("\033[38;5;%dm", x);
}

char convert(int x) {
  if(x < 10)
    return x + '0';
  else if(10 <= x && x < 10 + 26)
    return x - 10 + 'a';
  else
    return x - 10 - 26 + 'A';
}

void getName(int x, int y) {
  int topstr = 0;
  unsigned int xn = x, yn = y;
  int i = 0;
  while(prefix[i] != '\0') {
    nume[topstr++] = prefix[i];
    ++i;
  }
  for(i = 0; i < 6; ++i) {
    nume[topstr++] = convert(xn % BAZA);
    xn = xn / BAZA;
  }
  for(i = 0; i < 6; ++i) {
    nume[topstr++] = convert(yn % BAZA);
    yn = yn / BAZA;
  }
  nume[topstr++] = '.';
  nume[topstr++] = 'd';
  nume[topstr++] = 'a';
  nume[topstr++] = 't';
  nume[topstr++] = '\0';
}

struct Bucata {
  int x, y;
  unsigned char matr[CHUNK][CHUNK];
  FILE *fin, *fout;
  Bucata() {
    x = y = 0;
  }
  Bucata(int _x, int _y) {
    curs = 0;
    x = _x;
    y = _y;
    getName(x, y);
    fin = fopen(nume, "rb");
    if(fin == NULL) { //nu exista fisierul cu chunkul (x, y)
      for(int l = 0; l < CHUNK; ++l)
        for(int c = 0; c < CHUNK; ++c)
          matr[l][c] = 0;
    } else { //exista acest fisier
      for(int i = 0; i < MAX_N; ++i)
        fread(&lung[i], 1, sizeof(char_u), fin);

      buildarbcanonic();

      setcurs(0);
      for(int l = 0; l < CHUNK; ++l)
        for(int c = 0; c < CHUNK; ++c)
          matr[l][c] = gethuffchar(fin);
      fclose(fin);
    }
  }
  void save() {
    bool mustsave = false;
    for(int l = 0; l < CHUNK; ++l)
      for(int c = 0; c < CHUNK; ++c)
        if(matr[l][c] != 0)
          mustsave = true;

    if(mustsave) {
      getName(x, y);
      FILE *fout = fopen(nume, "wb");
      for(int i = 0; i < MAX_N; ++i)
        frecv[i] = 0;

      for(int l = 0; l < CHUNK; ++l)
        for(int c = 0; c < CHUNK; ++c)
          frecv[matr[l][c]]++;

      buildarb();
      buildarbcanonic();
      for(int i = 0; i < MAX_N; ++i)
        fwrite(&lung[i], 1, sizeof(char_u), fout);
      setcurs(8);
      for(int l = 0; l < CHUNK; ++l)
        for(int c = 0; c < CHUNK; ++c)
          putHuff(fout, matr[l][c]);
      emptybuff(fout);
      fclose(fout);
    }
  }

  bool operator< (Bucata r) {
    return x < r.x || (x == r.x && y < r.y);
  }
};

bool move(int &x, int &y) {
  bool apasat = false;
  if(tasta == 'a') {
    x -= speed;
    apasat = true;
  }
  if(tasta == 'd') {
    x += speed;
    apasat = true;
  }
  if(tasta == 'w') {
    y -= speed;
    apasat = true;
  }
  if(tasta == 's') {
    y += speed;
    apasat = true;
  }
  return apasat;
}

void scrie(Keyboard key, int x, int y) {
  for(int i = 0; i < key.top; ++i)
    putChar(x + i, y, key.ch[i]);
}

void bordura() {
  setBackgroundColor(RED);
  for(int x = -1; x <= X_SCREEN; ++x) {
    putChar(PX + x, PY - 1, ' ');
    putChar(PX + x, PY + Y_SCREEN, ' ');
  }
  for(int y = -1; y <= Y_SCREEN; ++y) {
    putChar(PX - 1, PY + y, ' ');
    putChar(PX + X_SCREEN, PY + y, ' ');
  }
}

void init() {
  setBackgroundColor(BLACK);
  cls();
  hidecursor();
  bordura();
  for(int i = 0; i < X_SCREEN; ++i)
    cache[i] = ' ';
  cache[X_SCREEN] = '\0';
}

Bucata v[1+MAX_CHUNKS];
int next[1+MAX_CHUNKS], last[HASH], tophash;

inline int hash(int x, int y) {
  int r = (long long)x * y % HASH;
  if(r < 0)
    return r + HASH;
  return r;
}

inline Bucata* query(int x, int y) {
  int h = hash(x, y);
  int i = last[h];
  while(i != 0 && !(v[i].x == x && v[i].y == y))
    i = next[i];
  if(i == 0) {
    tophash++;
    if(tophash > MAX_CHUNKS) {
      for(int i = 1; i <= tophash; ++i) {
        last[hash(v[i].x, v[i].y)] = 0;
        v[i].save();
      }
      tophash = 1;
    }
    v[tophash] = Bucata(x, y);
    next[tophash] = last[h];
    last[h] = tophash;
    return &v[tophash];
  } else
    return &v[i];
}

unsigned char &cell(int x, int y) {
  Bucata *t;
  int chx, chy;
  if(x >= 0) {
    chx = x / CHUNK;
    x = x % CHUNK;
  } else {
    chx = (x + 1) / CHUNK - 1;
    x = (x + 1) % CHUNK + CHUNK - 1;
  }

  if(y >= 0) {
    chy = y / CHUNK;
    y = y % CHUNK;
  } else {
    chy = (y + 1) / CHUNK - 1;
    y = (y + 1) % CHUNK + CHUNK - 1;
  }
  t = query(chx, chy);
  return (t -> matr[x][y]);
}

int screen[X_SCREEN][Y_SCREEN];

inline void clearLine(int y) {
  setBackgroundColor(BLACK);
  putStr(1, y, cache);
}

inline void display(int x, int y) { //x si y este centrul ecranului
  setBackgroundColor(BLACK);
  cls();
  bordura();
  for(int xi = -X_MID; xi <= X_MID; xi++)
    for(int yi = -Y_MID; yi <= Y_MID; yi++) {
      screen[xi + X_MID][yi + Y_MID] = cell(x + xi, y + yi);
    }


  for(int yi = 0; yi < Y_SCREEN; ++yi) {
    int cons = 0, last = screen[0][yi];
    for(int xi = 0; xi < X_SCREEN; ++xi)
      if(screen[xi][yi] == last)
        ++cons;
      else {
        setBgColor(last);
        putStr(PX + xi - cons, PY + yi, &cache[X_SCREEN - cons]);
        cons = 1;
        last = screen[xi][yi];
      }
    setBgColor(last);
    putStr(PX + X_SCREEN - cons, PY + yi, &cache[X_SCREEN - cons]);
  }
}

void displaydata(int x, int y, int color, int scula, int sizescula) {
  clearLine(Y_COORD);
  gotoxy(X_COORD, Y_COORD);
  setBackgroundColor(BLACK);
  setColor(WHITE);
  printf("%d %d ", x, y);

  setBgColor(color);
  printf("   ");

  setBackgroundColor(BLACK);
  setColor(WHITE);
  if(scula == 0)
    printf("Square ");
  else if(scula == 1)
    printf("Manhattan ");
  else
    printf("Circle ");
  printf("%d", sizescula);
}

bool changetool(int &scula, int &sculasize) {
  if(tasta == '[' && sculasize > 0) {
    --sculasize;
    return true;
  }
  if(tasta == ']') {
    sculasize++;
    return true;
  }
  if(tasta == '1') {
    scula = 0;
    return true;
  }
  if(tasta == '2') {
    scula = 1;
    return true;
  }
  if(tasta == '3') {
    scula = 2;
    return true;
  }
  return false;
}

void tool(int x, int y, unsigned char color, int n, int scula) {
  for(int xi = -n; xi <= n; ++xi)
    for(int yi = -n; yi <= n; ++yi) {
      int xn = x + xi;
      int yn = y + yi;
      if(scula == 0)
        cell(xn, yn) = color;
      else if(scula == 1 && abs(xi) + abs(yi) <= n)
        cell(xn, yn) = color;
      else if(scula == 2 && xi * xi + yi * yi <= n * n)
        cell(xn, yn) = color;
    }
}

void paleta(int &color) {
  int l = color / PALETA, c = color % PALETA, x;
  bool ok = false;
  setBackgroundColor(BLACK);
  cls();
  x = 0;
  for(int l1 = 0; l1 < PALETA; ++l1) {
    gotoxy(1, l1 * 2 + 1);
    for(int c1 = 0; c1 < PALETA; ++c1) {
      setBgColor(x++);
      printf("   ");
    }
  }

  setColor(WHITE);
  setBackgroundColor(BLACK);
  while(!ok) {
    tasta = getkey();
    gotoxy(c * 3 + 1, l * 2 + 2);
    printf("   ");
    if(tasta == 'a' && c > 0) {
      --c;
      --color;
    } else if(tasta == 'd' && c < PALETA - 1) {
      ++c;
      ++color;
    } else if(tasta == 's' && l < PALETA - 1) {
      ++l;
      color = color + PALETA;
    } else if(tasta == 'w' && l > 0) {
      --l;
      color = color - PALETA;
    } else if(tasta == '\n')
      ok = true;
    gotoxy(c * 3 + 1, l * 2 + 2);
    printf("^^^");
  }
}

int main() {
#ifndef DEBUG
  int x, y, color;
  int scula = 0;
  int sizescula = 0;
  bool close = false, cursor = true, enter = false;
  Keyboard keyb;
  x = y = 0;
  color = 0;
  init();
  while(!close) {
    tasta = getkey();
    if(enter) {
      tasta = 0;
      enter = false;
    }
    fflush(stdin);
    if(tasta == 'X')
      close = true;
    else if(move(x, y)) {
      display(x, y);
      if(cursor) {
        setBgColor(color);
        putChar(PX + X_MID, PY + Y_MID, ' ');
      }
    } else if(tasta == '\n') {
      tool(x, y, color, sizescula, scula);
      display(x, y);
    } else if(tasta == 't') {
      cursor ^= 1;
      display(x, y);
    } else if(tasta == 'j') {
      setBackgroundColor(BLACK);
      setColor(WHITE);
      gotoxy(1, 1);
      printf("Scrie coordonatele (x, y):");
      fflush(stdin);
      scanf("%d%d", &x, &y);
      enter = true;
      fflush(stdin);
      clearLine(1);
      bordura();
      display(x, y);
      tasta = 0;
      if(cursor) {
        setBgColor(color);
        putChar(PX + X_MID, PY + Y_MID, ' ');
      }
    } else if(tasta == 'v') {
      setBackgroundColor(BLACK);
      setColor(WHITE);
      gotoxy(1, 1);
      printf("Scrie viteza:");
      fflush(stdin);
      scanf("%d", &speed);
      enter = true;
      fflush(stdin);
      clearLine(1);
    } else if(changetool(scula, sizescula))
      displaydata(x, y, color, scula, sizescula);
    else if(tasta == 'p') {
      paleta(color);
      enter = true;
      display(x, y);
    } else if(tasta == 'f') {
      cell(x, y) = 0;
    }
    //keyb.read();
    displaydata(x, y, color, scula, sizescula);
    msleep(10);
  }
  for(int i = 1; i <= tophash; ++i)
    v[i].save();
  return 0;
#else
  setBgColor(202);
  printf("     ");
  return 0;
#endif
}
