CC := gcc
CFLAGS := -g -Wall

all: slug profilic generation
#
# ~uncomment during usage
#
# explorer: explorer.c
# 	$(CC) $(CFLAGS) -o explorer explorer.c

generation: generation.c
	$(CC) $(CFLAGS) -o generation generation.c

profilic: profilic.c
	$(CC) $(CFLAGS) -o profilic profilic.c

slug: slug.c
	$(CC) $(CFLAGS) -o slug slug.c

# slugrace: slugrace.c
# 	$(CC) $(CFLAGS) -o slugrace slugrace.c

clean:
	$(RM) profilic generation slug