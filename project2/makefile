CC := gcc
CFLAGS := -g -Wall

all: omnisportscomplex flightsimulator

omnisportscomplex: OmniSportsComplex.c
	$(CC) $(CFLAGS) -o omnisportscomplex OmniSportsComplex.c -lm

flightsimulator: FlightSimulator.c
	$(CC) $(CFLAGS) -o flightsimulator FlightSimulator.c -lm

clean:
	$(RM) omnisportscomplex flightsimulator