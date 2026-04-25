/*
    Author: Aaron Rodriguez
    Date: 11/11/2025
    Description: prompts user for input then drops specified dirtball(s)
    Usage: ./dirtball
*/

#include <iostream>
#include <cmath>
#include <ctime>
#include <fstream>
#include <iomanip>



// Recommended functions

double frand(){
    return (double) rand() / ((double) RAND_MAX + 1);
}

int boundCheck(int width, int height, int x, int y) {
    return (x >= 0 && x < width && y >= 0 && y < height);
}

//Calculate circle (dirtball)
void dropDirtball(int** grid, int width, int height, int x, int y, int radius, int strength){
    for (int dy = -radius; dy <=radius; ++dy) {
        for (int dx = -radius; dx <= radius; ++dx) {
            int nx = x + dx;
            int ny = y + dy;
            if (boundCheck(width, height, nx, ny)) {
                double dist = sqrt(dx * dx + dy * dy); //Distance formula
                if (dist <= radius) {
                    int impact = strength - (int)std::floor(dist); //Impact formula
                    if (impact > 0) {
                        grid[ny][nx] += impact; //Apply impact
                }
            }
                
            }
        }
    }
}


int findMax(int** grid, int width, int height) {
    int maxVal = 0;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (grid[y][x] > maxVal) {
                maxVal = grid[y][x];
            }
        }
    }
    return maxVal;
}

//Normalize to 0-255
void normalize(int** grid, int width, int height, int max) {
    if (max == 0) return;
    for (int y = 0; y < height; ++y) {
        for(int x = 0; x < width; ++x) {
            grid[y][x] = (int)std::floor((grid[y][x] / (double)max) * 255.0);

        }
    }
}


//Convert to char map
char** polish(int** grid, int width, int height, int waterLine) {
    char** charGrid = new char*[height];
    for (int y = 0; y < height; ++y) {
        charGrid[y] = new char[width];
    }

    int landArea = 255 - waterLine;
    int deepWaterThresh = waterLine / 2; // Deep water < 50% water line
    int coastThresh = 0.15 * landArea; // Coast = 15% of landArea
    int plainsThresh = 0.40 * landArea; // Plains = 40% of landArea
    int forestThresh = 0.80 * landArea; // Forest = 80% of landArea

    //Map vals to chars 
    for (int y = 0; y < height; ++y) {
        for(int x = 0; x < width; ++x) {
            int val = (int)std::floor(grid[y][x]);

            if (val < deepWaterThresh)
                charGrid[y][x] = '#';   //Deep
            else if (val <= waterLine)
                charGrid[y][x] = '~';   //Shallow
            else {
                int landVal = val - waterLine; //Value above water
                if (landVal < coastThresh) charGrid[y][x] = '.';    //coast
                else if (landVal < plainsThresh) charGrid[y][x] = '-';  //plains
                else if (landVal < forestThresh) charGrid [y][x] = '*'; //forest
                else charGrid[y][x] = '^';      //mountain

            }
        }
    }
    return charGrid;
}

//BMP file conversion
void saveBMP(const char* filename, int** landMass, int width, int height, int waterLine) {
    int rowSize = (3 * width + 3) & (~3);
    int dataSize = rowSize * height;
    int fileSize = 54 + dataSize;

    unsigned char header[54] = {
        'B','M', 0,0,0,0, 0,0,0,0, 54,0,0,0, //offset to pixel data 54 bytes 
        40,0,0,0, 0,0,0,0, 0,0,0,0,         //header size
        1,0, 24,0, 0,0,0,0, 0,0,0,0,        //just 1 color plane, 24 bit per pixel (RGB)
        0,0,0,0, 0,0,0,0, 0,0,0,0,
        0,0,0,0,
    };

    header[2] = (unsigned char)(fileSize);
    header[3] = (unsigned char)(fileSize >> 8);
    header[4] = (unsigned char)(fileSize >> 16);
    header[5] = (unsigned char)(fileSize >> 24);

    header[18] = (unsigned char)(width);
    header[19] = (unsigned char)(width >> 8);
    header[20] = (unsigned char)(width >> 16);
    header[21] = (unsigned char)(width >> 24);

    header[22] = (unsigned char)(height);
    header[23] = (unsigned char)(height >> 8);
    header[24] = (unsigned char)(height >> 16);
    header[25] = (unsigned char)(height >> 24);

    header[34] = (unsigned char)(dataSize);
    header[35] = (unsigned char)(dataSize >> 8);
    header[36] = (unsigned char)(dataSize >> 16);
    header[37] = (unsigned char)(dataSize >> 24);

    FILE* f = fopen(filename, "wb");
    fwrite(header, 1, 54, f);

    unsigned char* row = new unsigned char[rowSize]; //Write BMP and add color
    for (int i = height - 1; i >= 0; --i) {
        for (int j = 0; j < width; ++j) {
            int val = landMass[i][j];
            unsigned char r, g, b;

            if (val < waterLine / 2) {
                r= 0; g = 0; b = 80;        //Deep wate = blue
            } else if (val < waterLine) {
                r = 100; g = 150; b = 255; //Shallow water = light blue
            } else {
                int landVal = val - waterLine;
                int landRange = 255 - waterLine;

                if (landVal < 0.15 * landRange) {
                    r = 194; g = 139; b = 128;  //Coast = light brown
                } else if (landVal < 0.40 * landRange) {
                    r = 34; g = 139; b = 34;    //Plains = green
                } else if (landVal < 0.80 * landRange) {
                    r = 0; g = 100; b = 0;      //Forest = dark green
                } else {
                    r = 33; g = 16; b = 9; //Mountains = brown
                }
            }

                row[j * 3+ 0] = b;
                row[j * 3 + 1] = g;
                row[j * 3 + 2] = r;
            
        }
        fwrite(row, 1, rowSize, f);

    }

    delete[] row;
    fclose(f);
}

//Main function
int main(){

    char* seedEnv = getenv("SEED"); //get seed RNG
    if (seedEnv) {
        srand(atoi(seedEnv));
    } else {
        srand(time(NULL));
    }

    //Declare variables
    int width, height, waterLine, radius, strength, numDirtballs;

    //Prompt user for inputs
    std::cout << "Enter a width: ";
    std::cin >> width;
    std::cout << "Enter a height: ";
    std::cin >> height;

    do {
        std::cout << "Enter a water line value (between 40 and 200): "; 
        std::cin >> waterLine;

} while (waterLine < 40 || waterLine > 200); //Range 40-200

    std::cout << "Enter a radius value: ";
    std::cin >> radius;

    do {
        std::cout << "Enter a strength value (cannot be less than radius): ";
        std::cin >> strength;

    } while (strength < radius); // Range no less than radius

    std::cout << "How many dirtballs would you like to drop? ";
    std::cin >> numDirtballs;

// Allocate array
int** landMass = new int*[height];   //Row of pointers
    for (int i = 0; i < height; i++) {
        landMass[i] = new int[width]();   //Allocate each row
    }
    
for (int i = 0; i < numDirtballs; ++i) { 
    int x = (int)(frand() * width); //Randomize drop location
    int y = (int)(frand() * height);
    dropDirtball(landMass, width, height, x, y, radius, strength);
}

//Output to land_raw.txt
std::ofstream outFile("land_raw.txt");

for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
        outFile << std::setw(4) << landMass[y][x];
        }
        outFile << '\n';
    }
    outFile.close();

    //Normalize
    int maxVal = findMax(landMass, width, height);
    normalize(landMass, width, height, maxVal);

    //Save BMP
    saveBMP("land_map.bmp", landMass, width, height, waterLine);

    std::ofstream normFile("land_normalized.txt");
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            normFile << std::setw(4) << landMass[y][x];
        }
        normFile << '\n';
    }
    normFile.close();

    //Polish
    char** charGrid = polish(landMass, width, height, waterLine);
    std::ofstream polFile("land_polished.txt");
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            polFile << charGrid[y][x];
        }
        polFile << '\n';
    }
    polFile.close();

    
    //free memory
    for (int y = 0; y < height; ++y) {
        delete[] landMass[y];
        delete[] charGrid[y];
    }
    delete[] landMass;
    delete[] charGrid;

    return 0;

}

