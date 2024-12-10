# Derleyici ve bayraklar
CC = gcc
CFLAGS = -Wall -g

# Çıktı dosyası
TARGET = program

# Kaynak ve başlık dosyaları
SRCS = program.c
HDRS = program.h

# Hedef: tüm projeyi derle
all: $(TARGET)

# Çıktı dosyasını oluştur
$(TARGET): $(SRCS) $(HDRS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

# Temizlik işlemi
clean:
	rm -f $(TARGET) *.o

# Yeniden derleme (önce temizler, sonra derler)
rebuild: clean all
