################################################################################
# Fakulta informačních technologií Vysokého učení technického v Brně           #
# Počítačové vidění                                                            #
# Domácí úloha 3                                                               #
# Autor: Michal Jurzykowski <ijurzy@fit.vutbr.cz>                              #
#                                                                              #
# Makefile k 3 domácí úloze - detetekce chodců pomocí Viola@Jones a HOGu.      #
# Makefile lze použít jak pro kompilaci jednotlivých úloh, tak obou současně   #
# (výchozí chování).                                                           #
# Pro správné fungování je nutné nastavit několik hodnot, více poznámky        #
# přímo v souboru.                                                             #
#                                                                              #
# Pozn: kodovani utf-8                                                         #
################################################################################
CC=g++
CFLAGS=-std=c++03 `pkg-config --cflags opencv`
LIBS=`pkg-config --libs opencv`

# Nazvy zdrojovych souboru pro jednotlive detektory
SRC_FILE_FACES=process-video.cpp

# Nastavte vstupní parametry potřebné pro spuštění aplikace, pokud chcete spouštět
# detekce pomocí Makefile.
INPUT_VIDEO_NAME=
POSITIVE_IMAGE_FILE=
NEGATIVE_IMAGE_FILE=
DETECTOR_NAME=

# Kompilace obou detektoru
all:  process-video


processVideo: $(SRC_FILE_FACES)
	$(CC) $(CFLAGS) $(LIBS) $(SRC_FILE_FACES) -o process-video

clean:
	rm -f *.o *~  process-video

# Výchozí použití pro detekci prostřednictvím HOGu, lze přidat/pozměnit parametry, více zdrojový soubor.

run:
	./process-video -i $(INPUT_VIDEO_NAME) -d $(DETECTOR_NAME)
