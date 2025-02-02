// EE261 Progressive Programming Project
// Part 5
// Cal Poly, SLO
// Jack Marshall

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

char currentLine[127];
int result = 0;
size_t readLine(FILE *f, char *lineArray);

int packetsCounter = 0;
const char *NEEDLE1 = ":=";
const char *NEEDLE2 = ">";
const char *NEEDLE3 = ":`";
const char *BEST_CALLSIGN = "KE6GYD-5";
char callsign[10];

typedef struct dms
	{
	int degrees;
	int minutes;
	int seconds;
	} Coord;

typedef struct point
	{
	Coord Latitude;
	Coord Longitude;
	} Location;
Location oldLocation;

float getDistance(Location *newLocation, Location *oldLocation);
Location *getCoord(char *currentLine);



int main()
	{
	FILE *fptr; //file pointer

	//open the file
	if((fptr = fopen("..\\APRSIS_DATA.txt", "r")) == NULL)
		{
		// if file not found, throw error
		printf("Error opening input file\n");
		exit(1);
		}

	// while not at the end of the file
	while(result != EOF)
		{
		// call readLine for line string
		result = readLine(fptr, currentLine);

		// end function if end of file
		if(result == -1)
			{
			break;
			}

		// if there is ":=" AND there is ">", but there is NOT ":`" in the current line
		if((strstr(currentLine, NEEDLE1) != NULL) && (strstr(currentLine, NEEDLE2) != NULL) && (strstr(currentLine, NEEDLE3) == NULL))
			{
			char *startOfCallsign = currentLine; // first call sign position

			char *endOfCallsign = strstr(currentLine, NEEDLE2); // last call sign position

			int lengthOfCallsign = endOfCallsign - startOfCallsign; // get size of call sign

			strncpy(callsign, startOfCallsign, lengthOfCallsign); // copy call sign out and put it in a variable

			callsign[lengthOfCallsign] = '\0'; // Null-terminate the string

			if(strstr(callsign, BEST_CALLSIGN) != NULL) // if this is the correct callsign
				{
				Location *newLocation = getCoord(currentLine); // get location information

				// get distance from last position. use & to modify old Location data.
				float distance = getDistance(newLocation, &oldLocation);

				packetsCounter++; // increment packets counted

				printf("%18s:   Lat: %3d %02d %02d   Long: %4d %02d %02d   Miles: %3.1f   %06lu records processed\n",
				       callsign,
				       newLocation->Latitude.degrees,
				       newLocation->Latitude.minutes,
				       newLocation->Latitude.seconds,
				       newLocation->Longitude.degrees,
				       newLocation->Longitude.minutes,
				       newLocation->Longitude.seconds,
				       distance,
				       packetsCounter);
				}
			}
		}

	//Close the file and exit
	fclose(fptr);
	exit(0);
	}



float getDistance(Location *newLocation, Location *oldLocation)
	{
	// convert DMS to radians
	float lat1 = (M_PI / 180) * (oldLocation->Latitude.degrees + oldLocation->Latitude.minutes / 60.0 + oldLocation->Latitude.seconds / 3600.0);
	float long1 = (M_PI / 180) * (oldLocation->Longitude.degrees + oldLocation->Longitude.minutes / 60.0 + oldLocation->Longitude.seconds / 3600.0);
	float lat2 = (M_PI / 180) * (newLocation->Latitude.degrees + newLocation->Latitude.minutes / 60.0 + newLocation->Latitude.seconds / 3600.0);
	float long2 = (M_PI / 180) * (newLocation->Longitude.degrees + newLocation->Longitude.minutes / 60.0 + newLocation->Longitude.seconds / 3600.0);

	// calculate distance between coordinates
	float distance = 3963.0 * acos((sin(lat1) * sin(lat2)) + cos(lat1) * cos(lat2) * cos(long2 - long1));

	// Update the oldLocation values to the current location
	oldLocation->Latitude.degrees = newLocation->Latitude.degrees;
	oldLocation->Latitude.minutes = newLocation->Latitude.minutes;
	oldLocation->Latitude.seconds = newLocation->Latitude.seconds;
	oldLocation->Longitude.degrees = newLocation->Longitude.degrees;
	oldLocation->Longitude.minutes = newLocation->Longitude.minutes;
	oldLocation->Longitude.seconds = newLocation->Longitude.seconds;

	// if this is the first location read, return no distance traveled.
	if(packetsCounter == 0)
		return 0;

	// return distance calculation
	return distance;
	}



Location *getCoord(char *currentLine)
	{
	// allocate memory for the new structure
	Location *newLocation = (Location*) malloc(sizeof(Location));
	if(newLocation == NULL)
		{
		printf("Memory Error.");
		exit(1);
		}

	// go to start of the coordinate string
	char *startOfLocation = strstr(currentLine, NEEDLE1) + 2;

	// setup latitude and longitude variables
	char sLatitude[8];
	char sLongitude[9];
	memset(sLatitude, 0, 8);
	memset(sLongitude, 0, 9);

	// extract data for latitude and longitude
	strncpy(sLatitude, startOfLocation, 7);
	sLatitude[7] = '\0';
	strncpy(sLongitude, startOfLocation + 9, 8);
	sLongitude[8] = '\0';

	// setup cardinal direction variables
	char hemiLat[2];
	char hemiLong[2];
	memset(hemiLat, 0, 2);
	memset(hemiLong, 0, 2);

	// extract data for cardinal directions (N/S/E/W)
	strncpy(hemiLat, startOfLocation + 7, 1);
	hemiLat[1] = '\0';
	strncpy(hemiLong, startOfLocation + 17, 1);
	hemiLong[1] = '\0';

	// turn float decimals into int variables. round up floats.
	float latitude = strtof(sLatitude, NULL);
	int decimalPartLat = (int)(((latitude - (int)latitude) + 0.005) * 100);
	float longitude = strtof(sLongitude, NULL);
	int decimalPartLong = (int)(((longitude - (int)longitude) + 0.005) * 100);

	// put in Latitude DMS
	newLocation->Latitude.degrees = (int)strtol(sLatitude, (char**)NULL, 10) / 100;
	newLocation->Latitude.minutes = (int)strtol(sLatitude, (char**)NULL, 10) % 100;
	newLocation->Latitude.seconds = (decimalPartLat * 60) / 100;

	// put in Longitude DMS
	newLocation->Longitude.degrees = (int)strtol(sLongitude, (char**)NULL, 10) / 100;
	newLocation->Longitude.minutes = (int)strtol(sLongitude, (char**)NULL, 10) % 100;
	newLocation->Longitude.seconds = (decimalPartLong * 60) / 100;

	// adjust for cardinal directions
	newLocation->Latitude.degrees *= (hemiLat[0] == 'S') ? -1 : 1;
	newLocation->Longitude.degrees *= (hemiLong[0] == 'W') ? -1 : 1;

	return newLocation;
	}


// read a line from the file, ignoring the terminating 0x0a
// place characters read into specified array
// return number of characters read
// return -1 if EOF
size_t readLine(FILE *f, char *lineArray)
	{
	char readChar;
	char count = 0;
	//read the first character
	readChar = getc(f);
	//continue until EOF
	while(readChar != EOF)
		{
		if(readChar == 0x0a)
			{
			//we have reached the end of the line
			*lineArray++ = 0; //null terminate the string
			return(count); //return the count
			}
		*lineArray++ = readChar; //store the character
		count++; //increment the count
		readChar = getc(f); //get the next character
		}
	if(readChar == EOF) return(-1); //return a -1 if we reached the end of the file
	}


