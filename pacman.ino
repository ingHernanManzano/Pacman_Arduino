#include <LiquidCrystal.h>

//  CONFIGURACIÓN 
#define VITESSE_PAC 150     // Velocidad de Pac-Man
#define VITESSE_FANT 2000   // Velocidad del fantasma
#define MAXX 15
#define MAXY 1

// Botones
#define btnRight   0
#define btnUp      1
#define btnDown    2
#define btnLeft    3
#define btnSelect  4
#define btnNone    5

// Pines
#define BUZZER_PIN 12
#define RIGHT_PIN  8
#define LEFT_PIN   9
#define DOWN_PIN   10
#define UP_PIN     11
#define SELECT_PIN 13

// LCD
LiquidCrystal lcd(7, 6, 5, 4, 3, 2); 

//  PERSONAJES 
byte pacman[8] = {
  B00000, B00000, B01110, B11011, B11100, B01110, B00000, B00000
};
byte fantome[8] = {
  B00000, B00000, B01110, B10101, B11111, B11111, B10101, B00000
};
byte point[8] = {
  B00000, B00000, B00000, B01110, B01110, B00000, B00000, B00000
};

//  VARIABLES DE JUEGO 
byte points[MAXX+1][MAXY+1];
int xpac = 2, ypac = 1;     // posicion Pac-Man
int xfant = 15, yfant = 0;  // posicion Fantasma
long keystruck = 0;         // ultimo movimiento Pac-Man
long poursuite = 0;         // ultimo movimiento Fantasma
byte partieEnCours = false; // partida activa?
byte vide = false;          // se comieron todos los puntos?
byte level = 1;             // nivel
int score = 0;              // puntuacion

//  FUNCIONES 

// Movimiento Pac-Man
void bouge(int x,int y) {
  int oldx = xpac, oldy = ypac;
  if (((xpac+x) >= 0) && ((xpac+x) <= MAXX)) xpac += x;
  if (((ypac+y) >= 0) && ((ypac+y) <= MAXY)) ypac += y;

  lcd.setCursor(xpac, ypac); lcd.write(byte(0));
  lcd.setCursor(oldx, oldy); 
  if ((xpac != oldx) || (ypac != oldy)) lcd.print(" ");

  if(points[xpac][ypac]){
    points[xpac][ypac] = false;
    score++;
  }

  // Verificar si comió todo
  vide = true;
  for (int i=0; i<=MAXX; i++)
    for (int j=0; j<=MAXY; j++)
      if (points[i][j]) vide = false;

  if (vide && partieEnCours) gagne();
}

// Fantasma persigue
void poursuis() {
  int oldx = xfant, oldy = yfant;
  if (yfant < ypac) yfant++;
  else if (yfant > ypac) yfant--;
  else if (xfant < xpac) xfant++;
  else if (xfant > xpac) xfant--;

  lcd.setCursor(xfant,yfant); lcd.write(1);
  lcd.setCursor(oldx,oldy);
  if ((oldx!=xfant)||(oldy!=yfant)) {
    if (points[oldx][oldy]) lcd.write(2);
    else lcd.print(" ");
  }
}

// Inicializa nivel
void initLevel() {
  lcd.clear();
  for (int i=0; i<=MAXX; i++)
    for (int j=0; j<=MAXY; j++){
      points[i][j]=true;
      lcd.setCursor(i, j);
      lcd.write(2);
    }
  xpac=2; ypac=1;
  xfant=15; yfant=0;
  lcd.setCursor(xpac,ypac); lcd.write(byte(0));
  lcd.setCursor(xfant,yfant); lcd.write(1);
  poursuite = millis();
  vide = false;
  partieEnCours = true;
}

// Sonido "Game Over"
void sonidoGameOver() {
  for (int freq = 1000; freq > 200; freq -= 100) {
    tone(BUZZER_PIN, freq, 150);
    delay(200);
  }
  noTone(BUZZER_PIN);
}

// Sonido "Next Level"
void sonidoNextLevel() {
  for (int freq = 200; freq < 1000; freq += 200) {
    tone(BUZZER_PIN, freq, 150);
    delay(200);
  }
  noTone(BUZZER_PIN);
}

// Pierde
void perdu() {
  lcd.clear();
  lcd.setCursor(0,0); lcd.print(" ***Game Over*** ");
  lcd.setCursor(0,1); lcd.print(" Puntaje: "); lcd.print(score);

  sonidoGameOver();

  delay(2000);
  level = 1; score = 0;
  partieEnCours = false;
}

// Gana
void gagne() {
  level++;
  lcd.clear();
  lcd.setCursor(0,0); lcd.print(" *** Next Level *** ");
  lcd.setCursor(0,1); lcd.print(" Level "); lcd.print(level);

  sonidoNextLevel();

  delay(2000);
  initLevel();
}

// Lectura de botones digitales
int getKey() {
  if (digitalRead(RIGHT_PIN) == LOW)  return btnRight;
  if (digitalRead(LEFT_PIN)  == LOW)  return btnLeft;
  if (digitalRead(DOWN_PIN)  == LOW)  return btnDown;
  if (digitalRead(UP_PIN)    == LOW)  return btnUp;
  if (digitalRead(SELECT_PIN)== LOW)  return btnSelect;
  return btnNone;
}

// SETUP 
void setup() {
  lcd.begin(16, 2);
  lcd.createChar(0, pacman);
  lcd.createChar(1, fantome);
  lcd.createChar(2, point);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RIGHT_PIN, INPUT_PULLUP);
  pinMode(LEFT_PIN, INPUT_PULLUP);
  pinMode(DOWN_PIN, INPUT_PULLUP);
  pinMode(UP_PIN, INPUT_PULLUP);
  pinMode(SELECT_PIN, INPUT_PULLUP);

  lcd.setCursor(0,0); lcd.print(" Pacman :V ");
  lcd.setCursor(0,1); lcd.print(" Press START ");
}

//  LOOP
void loop() {
  if (!partieEnCours) {
    if (getKey() == btnSelect) {
      initLevel(); // Solo inicia cuando presiona START
    }
    return;
  }

  // Movimiento Pac-Man
  if (millis()-keystruck > VITESSE_PAC) {
    int joy = getKey();
    switch (joy) {
      case btnLeft:  bouge(-1,0); keystruck=millis(); break;
      case btnRight: bouge(1,0);  keystruck=millis(); break;
      case btnUp:    bouge(0,-1); keystruck=millis(); break;
      case btnDown:  bouge(0,1);  keystruck=millis(); break;
    }
  }

  // Movimiento Fantasma
  if (millis()-poursuite > VITESSE_FANT/(level+1)+10) {
    poursuis();
    poursuite = millis();
  }

  // Colision
  if ((xpac==xfant)&&(ypac==yfant)&&partieEnCours) {
    perdu();
  }
}
