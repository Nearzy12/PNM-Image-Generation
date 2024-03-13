// Samuel Near
// Snear2
// CS4481 
// Assignment 1
// Febuary 8, 2024

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "libpnm.h"

// Define a tolerance value for rounding - used to resolve issues with extreme slopes
#define EPSILON 0.00001

// Function that validates the command line inputs from the user
int validateArguments(int argc, char *argv[]){
    // Check if there are the correct number of arguments
    if(argc != 6){
        printf("Error: Incorrect number of arguments.\n");
        printf("Usage: %s <image_type_code> <image_width> <image_height> <output_image_name> <image_format_code>\n", argv[0]);
        return 0;
    }

    // Parse arguments
    // User standard library for ASCII to int conversion.
    // Can't do math with characters
    int imageTypeCode = atoi(argv[1]);
    int imageWidth = atoi(argv[2]);
    int imageHeight = atoi(argv[3]);
    char *outputImageName = argv[4];
    int imageFormatCode = atoi(argv[5]);

    // Validate image type code
    if(imageTypeCode != 1 && imageTypeCode != 2 && imageTypeCode != 3){
        printf("Error: Invalid image type code. Must be 1, 2, or 3.\n");
        return 0;
    }

    // Validate the height, for all image type this needs to be divisable by 4 and greater than or equal to 4
    if(imageHeight % 4 != 0 || imageHeight < 4){
        printf("Error: Invalid image height. Must be divisible by 4, and greater than or equal to 4");
        return 0;
    }

    // Validate image width for pbm and pgm
    if(imageTypeCode == 1 || imageTypeCode == 2){
        if (imageWidth % 4 != 0 || imageWidth < 4){
            printf("Error: Invalid image width. PBM and PGM images must be divisible by 4, and greater than or equal to 4");
            return 0;
        }
    }

    // Validate image width for ppm
    if(imageTypeCode == 3){
        if (imageWidth % 6 != 0 || imageWidth < 6){
            printf("Error: Invalid image width. PPM images must be divisible by 6, and greater than or equal to 6");
            return 0;
        }
    }

    //Check that the image formate code is either 1 or 0
    if(imageFormatCode != 0 && imageFormatCode != 1){
        printf("Error: Invalid image formate code. 0 for ASCII or 1 for raw");
        return 0;
    }

    return 1;

    }

// Function to generate PBM images
int generatePBM(int imgWidth, int imgHeight,char *outputImgName, int imgFormatCode){
    // Generate the pbm image based on the passed parameters
    int row, col;
    struct PBM_Image pbmImage;
    // Slope doesnt need to be neagative becuase down the picture is positive in the row direction.
    float slope; 
    float rowValue, colValue;

    // Create the PBM image
    create_PBM_Image(&pbmImage, imgWidth, imgHeight);
    
    // When looping though each column you need to start at 1 and go to image width
    // If you start from 0 the slope doesn't work well
    // Since the indexing starts at 0 and goes to imgWidth-1
    // We need to subtract 1 when we are accessing the image matrix

    // Depending on the depentions of the image we will need to iterate in a different order
    if(imgWidth >= imgHeight){

        // Loop thorugh each pixel of the image
        // This is a horizontal image, the width is bigger than the height
        // The slope will be defferent based on which direction we are looping.
        slope = (float)imgHeight / imgWidth;

        for (col = 1; col <= imgWidth; col++)
        {
            rowValue = (slope*col);

            // Check if the number is not a whole number
            if (fabs(rowValue - (int)rowValue) > EPSILON) {
                // If the number is not a whole number, round up to the nearest integer
                row = (int)ceil(rowValue);
            }else{
                row = (int)rowValue;
            }

            pbmImage.image[row-1][col-1] = 1;
            pbmImage.image[imgHeight - row][col-1] = 1;
        }
    }

    // Works the same as above how ever we need to loop across the rows not the columns
    // Swamp row and col variables.
    if(imgWidth < imgHeight){
        // This is a vertical image, where the height is bigger than the width
        // Inverse slope .... this took me way to long to figure out the problem
        slope = (float)imgWidth / imgHeight;
        
        for (row = 1; row <= imgHeight; row++)
        {
            colValue = (slope*row);

            // Check if the number is not a whole number
            if (fabs(colValue - (int)colValue) > EPSILON) {
                // If the number is not a whole number, round up to the nearest integer
                col = (int)ceil(colValue);
            }else{
                col = (int)colValue;
            }

            pbmImage.image[row-1][col-1] = 1;
            pbmImage.image[row-1][imgWidth - col] = 1;
        }
    }

    // Fill in all the black values if the row and column are both less than
    // Width/4 and Height/4 
    int heightQuarter = imgHeight/4;
    int widthQuarter = imgWidth/4;

    // This could be more effiecient as the 4 cornors will be set to black twice
    for (row = 1; row <= imgHeight; row++)
    {
        for (col = 1; col <= imgWidth; col++)
        {
            // Not in the middle half vertically
            if(row <= heightQuarter || row > (heightQuarter*3)){
                // Top or Bottom quarters
                pbmImage.image[row-1][col-1] = 1;
            }

            // Not in the middle half horizontally
            if(col <= widthQuarter || col > (widthQuarter*3)){
                // Left or Right Quarter
                pbmImage.image[row-1][col-1] = 1;
            }  
        }
    }
    
    // Save the image to file
    save_PBM_Image(&pbmImage, outputImgName, imgFormatCode);

    // Free the memory of the image
    free_PBM_Image(&pbmImage);
}

int generatePGM(int imgWidth, int imgHeight,char *outputImgName, int imgFormatCode){
    // Generate the pgm image based on the passed parameters
    int row, col, intersect1, intersect2, distanceFromEdge, distanceFromMiddle;
    int tempWidth;
    int max_gray = 255;
    float gray_approx;
    int gray_value;
    float slope, rowValue, colValue;
    float gray_ratio;
    bool verticalImage, horizontalImage;

    struct PGM_Image pgmImageHorizontal;
    struct PGM_Image pgmImageVertical;

    //determine the type of image, vertical or horizontal
    if(imgWidth >= imgHeight){
        verticalImage = false;
        horizontalImage = true;
    }else{
        verticalImage = true;
        horizontalImage = false;
    }

    // if the image is vertical, just swap height and width
    if(verticalImage){
        tempWidth = imgWidth;
        imgWidth = imgHeight;
        imgHeight = tempWidth;
    }

    // Create the PGM image
    create_PGM_Image(&pgmImageHorizontal, imgWidth, imgHeight, max_gray);

    // Fill in all the black values if the row and column are both less than
    // Width/4 and Height/4 
    int heightQuarter = imgHeight/4;
    int widthQuarter = imgWidth/4;

    // These values will be used with the distance from the edge of the image to determine how dark to make the pixel
    int grayIncVertical = max_gray/heightQuarter;
    int grayIncHorizontal = max_gray/widthQuarter;

    // Loop thorugh each pixel of the image
    // This is a horizontal image, the width is bigger than the height
    // The slope will be defferent based on which direction we are looping.
    slope = (float)imgHeight / imgWidth;

    for (col = widthQuarter+1; col <= widthQuarter*3; col++)
    {
        rowValue = (slope*col);

        // Check if the number is not a whole number
        if (fabs(rowValue - (int)rowValue) > EPSILON) {
            // If the number is not a whole number, round up to the nearest integer
            row = (int)ceil(rowValue);
        }else{
            row = (int)rowValue;
        }

        // Save the two intersection points
        intersect1 = row;
        intersect2 = imgHeight - row + 1;

        // Loop though every row in that column
        for(row = heightQuarter+1; row <= heightQuarter*3; row++){
            // Using these intersection points - the points forming the lines from corner to corner
            // Determine which triangle we are in top, bottm, left, right
            if(col <= imgWidth/2){
                // On left side of square
                if(row <= intersect1){
                    // Top Triangle
                    // Determine distance from the edge, if distance is 0 then value is max grey value
                    distanceFromEdge = row - heightQuarter -1;
                    gray_ratio = 1.0f - ((float)distanceFromEdge / (heightQuarter - 1));
                }
                else if(row >= intersect2){
                    // Bottom triangle
                    distanceFromEdge = 3*heightQuarter - row;
                    gray_ratio = 1.0f - ((float)distanceFromEdge / (heightQuarter - 1));

                }
                else if(row > intersect1 && row < intersect2){
                    // Left Triangle
                    distanceFromEdge = col - widthQuarter-1;
                    gray_ratio = 1.0f - ((float)distanceFromEdge / (widthQuarter - 1));

                }
                else{
                    // On one of the lines
                    distanceFromEdge = 0;
                }
            }
            else if(col > imgWidth/2){
                // On the right side of the square
                if(row <= intersect2){
                    // Top Triangle
                    distanceFromEdge = row - heightQuarter -1;
                    gray_ratio = 1.0f - ((float)distanceFromEdge / (heightQuarter - 1));
                }
                else if(row >= intersect1){
                    // Bottom triangle
                    distanceFromEdge = 3*heightQuarter - row;
                    gray_ratio = 1.0f - ((float)distanceFromEdge / (heightQuarter - 1));
                }
                else if(row > intersect2 && row < intersect1){
                    // Right Triangle
                    distanceFromEdge = 3*widthQuarter - col;
                    gray_ratio = 1.0f - ((float)distanceFromEdge / (widthQuarter - 1));

                }
                else{
                    // On one of the lines
                    // Special Case
                    distanceFromEdge = 0;
                }
            }

            // If this distance from the edge is 0 then the gray value should = max gray value
            // If the distance = height of the triangle then value should = 0
            gray_approx = gray_ratio * max_gray;

            // Round this gray value
            // Check if the number is not a whole number
            if (fabs(gray_approx - (int)gray_approx) > EPSILON) {
                // If the number is not a whole number, round to the nearest integer
                gray_value = (int)round(gray_approx + 0.5);
            }else{
                gray_value = (int)gray_approx;
            }
            pgmImageHorizontal.image[row-1][col-1] = gray_value;
        }
    }

    // This could be more effiecient as the 4 cornors will be set to black twice
    // Create outside balck bars
    for (row = 1; row <= imgHeight; row++)
    {
        for (col = 1; col <= imgWidth; col++)
        {
            // Not in the middle half vertically
            if(row <= heightQuarter || row > (heightQuarter*3)){
                // Top or Bottom quarters
                pgmImageHorizontal.image[row-1][col-1] = 0;
            }

            // Not in the middle half horizontally
            else if(col <= widthQuarter || col > (widthQuarter*3)){
                // Left or Right Quarter
                pgmImageHorizontal.image[row-1][col-1] = 0;
            } 
        }
    }

    // If the image is vertial then we need to save the image vertially not horizontally
    // make a second image and copy the values over
    if(verticalImage){

        // Create the PGM image - notice parameter placement, width and height have been swapped back to normal
        create_PGM_Image(&pgmImageVertical, imgHeight, imgWidth, max_gray);
        for(row = 0; row < imgWidth; row++){
            for (col = 0; col < imgHeight; col++){
                pgmImageVertical.image[row][col] = pgmImageHorizontal.image[col][row];
            }
        }

        // Save the image to file
        save_PGM_Image(&pgmImageVertical, outputImgName, imgFormatCode);
        // Free image memory
        free_PGM_Image(&pgmImageVertical);
    }

    if(horizontalImage){
        // Save the image to file
        save_PGM_Image(&pgmImageHorizontal, outputImgName, imgFormatCode);
        // Free the memory of the image
        free_PGM_Image(&pgmImageHorizontal);
    }
}

int generatePPM(int imgWidth, int imgHeight,char *outputImgName, int imgFormatCode){
    // Generate the pbm image based on the passed parameters
    int row, col, intersect1, intersect2, distanceFromEdge;

    // Used to determine gradient
    int distanceFromWhite;
    int max_gray_value = 255;
    float gray_approx;
    float gray_ratio_inverse;
    int gray_value;
    float gray_ratio;

    struct PPM_Image ppmImage;
    struct PGM_Image pgmImageRed;
    struct PGM_Image pgmImageGreen;
    struct PGM_Image pgmImageBlue;

    int vertical_half = imgHeight/2;
    int horizontal_half = imgWidth/2;
    int third = imgWidth/3;

    // Create the ppm image
    create_PPM_Image(&ppmImage, imgWidth, imgHeight, max_gray_value);

    for(row = 0; row < imgHeight; row++){
        for(col = 0; col < imgWidth; col++){
            //determine which section we are in then we can determine the colour
            // Top half of image
            if(row < vertical_half){
                // Left third
                if(col < third){

                    // Calculate distance to white
                    distanceFromWhite = (vertical_half -1) - row;

                    gray_ratio = 1.0f - ((float)distanceFromWhite / (vertical_half - 1));  
                    gray_approx = max_gray_value * gray_ratio;

                    // Round this gray value
                    // Check if the number is not a whole number
                    if (fabs(gray_approx - (int)gray_approx) > EPSILON) {
                        // If the number is not a whole number, round to the nearest integer
                        gray_value = (int)round(gray_approx + 0.5);
                    }else{
                        gray_value = (int)gray_approx;
                    }         

                    ppmImage.image[row][col][RED] = max_gray_value;
                    ppmImage.image[row][col][BLUE] = gray_value;
                    ppmImage.image[row][col][GREEN] = gray_value;
 
                }
                // Middle third
                else if(col >= third && col < (2*third)){

                    // Calculate distance to white
                    distanceFromWhite = row;

                    gray_ratio = 1.0f - ((float)distanceFromWhite / (vertical_half - 1));  
                    gray_approx = max_gray_value * gray_ratio;

                    // Round this gray value
                    // Check if the number is not a whole number
                    if (fabs(gray_approx - (int)gray_approx) > EPSILON) {
                        // If the number is not a whole number, round to the nearest integer
                        gray_value = (int)round(gray_approx + 0.5);
                    }else{
                        gray_value = (int)gray_approx;
                    }         

                    ppmImage.image[row][col][RED] = gray_value;
                    ppmImage.image[row][col][BLUE] = gray_value;
                    ppmImage.image[row][col][GREEN] = max_gray_value;

                }
                // Right third
                else{

                    // Calculate distance to white
                    distanceFromWhite = (vertical_half -1) - row;

                    gray_ratio = 1.0f - ((float)distanceFromWhite / (vertical_half - 1));  
                    gray_approx = max_gray_value * gray_ratio;

                    // Round this gray value
                    // Check if the number is not a whole number
                    if (fabs(gray_approx - (int)gray_approx) > EPSILON) {
                        // If the number is not a whole number, round to the nearest integer
                        gray_value = (int)round(gray_approx + 0.5);
                    }else{
                        gray_value = (int)gray_approx;
                    }         

                    ppmImage.image[row][col][RED] = gray_value;
                    ppmImage.image[row][col][BLUE] = max_gray_value;
                    ppmImage.image[row][col][GREEN] = gray_value;

                }
            }
            // Bottom half of image
            else if(row >= vertical_half){
                // Left half
                if(col < horizontal_half){

                    // Calculate distance to white
                    distanceFromWhite = (imgHeight-1) - row;
                }
                // Right half
                else{

                    // Calculate distance from white
                    distanceFromWhite = row - vertical_half;
                }

                gray_ratio = 1.0f - ((float)distanceFromWhite / (vertical_half - 1));  
                gray_approx = max_gray_value * gray_ratio;

                // Round this gray value
                // Check if the number is not a whole number
                if (fabs(gray_approx - (int)gray_approx) > EPSILON) {
                    // If the number is not a whole number, round to the nearest integer
                    gray_value = (int)round(gray_approx + 0.5);
                }else{
                    gray_value = (int)gray_approx;
                }         

                ppmImage.image[row][col][RED] = gray_value;
                ppmImage.image[row][col][BLUE] = gray_value;
                ppmImage.image[row][col][GREEN] = gray_value;
            }
        }
    }

    // Save the image to file
    save_PPM_Image(&ppmImage, outputImgName, imgFormatCode);

    // Also save the image converted to gray 
    copy_PPM_to_PGM(&ppmImage, &pgmImageRed, RED);
	copy_PPM_to_PGM(&ppmImage, &pgmImageGreen, GREEN);
	copy_PPM_to_PGM(&ppmImage, &pgmImageBlue, BLUE);

    save_PGM_Image(&pgmImageRed, "color_to_pgm_RED.pgm" , imgFormatCode);
    save_PGM_Image(&pgmImageGreen, "color_to_pgm_GREEN.pgm" , imgFormatCode);
    save_PGM_Image(&pgmImageBlue, "color_to_pgm_BLUE.pgm" , imgFormatCode);

    
    // Free the memory of the image
    free_PPM_Image(&ppmImage);

    free_PGM_Image(&pgmImageRed);
    free_PGM_Image(&pgmImageGreen);
    free_PGM_Image(&pgmImageBlue);
}

// Main function to run the program
int main(int argc, char *argv[]) {

    // Validate the command line arguments
    if (!validateArguments(argc, argv)) {
        return 0;
    }

    printf("All arguments are valid.\n");

    // Save arguments to use in main function and convert to int
    int imageTypeCode = atoi(argv[1]);
    int imageWidth = atoi(argv[2]);
    int imageHeight = atoi(argv[3]);
    char *outputImageName = argv[4];
    int imageFormatCode = atoi(argv[5]);

    //generate the correct image based on the imageTypeCode
    // pbm
    if(imageTypeCode == 1){
        if(generatePBM(imageWidth, imageHeight, outputImageName, imageFormatCode)){
            printf("pbm image generated.\n");
        }
    }
    // pgm
    if(imageTypeCode == 2){
        if(generatePGM(imageWidth, imageHeight, outputImageName, imageFormatCode)){
            printf("pgm image generated.\n");
        }
    }
    // ppm
    if(imageTypeCode == 3){
        if (generatePPM(imageWidth, imageHeight, outputImageName, imageFormatCode)){
            printf("ppm image generated.\n");
        }
        
    }
    return 0;
}



