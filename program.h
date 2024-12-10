#ifndef PROGRAM_H
#define PROGRAM_H

// Maksimum komut uzunluðu ve argüman sayýsý
#define MAX_CMD_LENGTH 1024
#define MAX_ARGS 100

// Gömülü komutlar için fonksiyon bildirimleri
int kocsh_help(char** args);
int kocsh_quit(char** args);

// Komut yorumlama ve çalýþma fonksiyonlarý
char** komutYorumla(char* line);
int calistir(char** args);
int tekliKomutCalistir(char** args);
int girisYonlendirme(char** args);
int cikisYonlendirme(char** args);

// Eksik fonksiyonlar (arka plan ve pipe iþlemleri için)
int arkaPlandaCalistir(char** args); // & sembolü ile arka planda çalýþma
int boruCalistir(char** args);       // | sembolü ile pipe iþlemi

// Sinyal iþleyici fonksiyonu
void sig_chld(int signo);

#endif // PROGRAM_H

