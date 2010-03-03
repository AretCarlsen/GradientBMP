
#define HORIZONTAL_RESOLUTION_DPI 600
#define VERTICAL_RESOLUTION_DPI 600

#define BIT_DEPTH 24
#define MAX_BRIGHTNESS 0xFF

#include <stdlib.h>
#include <math.h>

// See http://easybmp.sourceforge.net/
#include "../EasyBMP/EasyBMP.h"

// Calculate the desired brightness of the current position.
// The reflection edge (center of image) occurs at current_position = max_position,
// whereas the leftmost edge occurs at current_position = 0.
unsigned int brightnessAtPosition(float fractional_position, float brightness_coefficient, float brightness_power){
// Conversion to nonlinear brightness
  // Note that we want the darkest area (lowest brightness) to the right (or center, in reflected mode).
  return round(brightness_coefficient * pow(1.0 - fractional_position, brightness_power) * MAX_BRIGHTNESS);
}

// Get a float argument
float get_argument(float *value, char *argument){
// Abort on error
  if(sscanf(argument, "%f", value) != 1){
    fprintf(stderr, "Invalid argument: %s\n", argument);
    exit(1);
  }
}

// Get an unsigned int argument
float get_argument(unsigned int *value, char *argument){
// Abort on error
  if(sscanf(argument, "%u", value) != 1){
    fprintf(stderr, "Invalid argument: %s\n", argument);
    exit(1);
  }
}

// String comparison
bool stringsEqual(const char* str1, const char* str2){
  return (strcmp(str1, str2) == 0);
}

// Usage
void usage(){
    fprintf(stderr, "  Usage: gradientBMP [--reflection] [--resolution <resolution-DPI>] [--coeff <brightness-coefficient] <output-filename> <width-inches> <height-inches> <brightness-power>\n");
    exit(1);
}

int main(int argc, char **argv){
// Dimensions (pixels and inches)
  float width_inches = 0;
  unsigned int width_pixels = 0;
  float height_inches = 0;
  unsigned int height_pixels = 0;
// Resolution
  unsigned int resolution_DPI = 400;

// Brightness coefficient and power
  float brightness_coefficient = 1.0;
  float brightness_power = 1.0;

// Reflection mode semaphore
  bool reflection_mode = false;

// Usage
  if(argc < 5){
    usage();
  }

/* ARGUMENTS */

  unsigned int arg_i = 1;

/* OPTIONAL ARGUMENTS */

// Check for reflection mode
  if(stringsEqual(argv[arg_i], "--reflection")){
    reflection_mode = true;
    arg_i++;
  }
// Check for resolution
  if(stringsEqual(argv[arg_i], "--resolution")){
    arg_i++;
    get_argument(&resolution_DPI, argv[arg_i]);
    arg_i++;
  }
// Check for brightness coefficient
  if(stringsEqual(argv[arg_i], "--coeff")){
    arg_i++;
    get_argument(&brightness_coefficient, argv[arg_i]);
    if(brightness_coefficient > 1.0){
      fprintf(stderr, "Brightness coefficient (%f) must be less than or equal to 1.0.\n", brightness_coefficient);
      exit(1);
    }
    arg_i++;
  }

/* REQUIRED ARGUMENTS */

// Filename argument
  char* output_filename = argv[arg_i];
    arg_i++;
    if(arg_i >= argc){
      usage();
    }
// Float arguments: width, height, brightness power, brightness coefficient, resolution (DPI)
  get_argument(&width_inches, argv[arg_i]);
    arg_i++;
    if(arg_i >= argc){
      usage();
    }
  get_argument(&height_inches, argv[arg_i]);
    arg_i++;
    if(arg_i >= argc){
      usage();
    }
  get_argument(&brightness_power, argv[arg_i]);
    arg_i++;

  if(arg_i < argc){
    usage();
  }

/* END ARGUMENTS */


// Calculate width in pixels
  width_pixels = round(width_inches * resolution_DPI);
  height_pixels = round(height_inches * resolution_DPI);

// EasyBMP object
  BMP GradientImage;
  // Set size
  GradientImage.SetSize(width_pixels, height_pixels);
  // Set resolution
  GradientImage.SetDPI(resolution_DPI, resolution_DPI);
  // Set its color depth to 16-bits
  GradientImage.SetBitDepth(BIT_DEPTH);

// Max position (pixels)
  unsigned int max_position = width_pixels;
// Check for reflection mode
  if(reflection_mode){
    max_position /= 2;
    if(width_pixels % 2 == 1)
      max_position++;
  }

  printf("Generating gradient\n");
  printf("  Width %f inches (%u pixels); Height %f inches (%u pixels); Resolution %u DPI\n", width_inches, width_pixels, height_inches, height_pixels, resolution_DPI);
  printf("  Brightness coefficient %f; power %f\n", brightness_coefficient, brightness_power);
  printf("  Reflection mode: ");
  if(!reflection_mode)
    printf("NOT ");
  printf("active\n");
  

  // Apply brightness function across width of image
  for( unsigned int i=0 ; i < max_position ; i++){
    // Calculate brightness
    unsigned int brightness = brightnessAtPosition((float) i / max_position, brightness_coefficient, brightness_power);

// fprintf(stderr, "DEBUG 1: Pos = %u, brightness = %u\n", i, brightness);

    // Apply brightness (vertically constant, symmetrical across horizontal center).
    // (The vertical resolution is technically nil, since the value is constant.)
    for( int j=0 ; j < GradientImage.TellHeight() ; j++){
    // The grey spectrum requires red = green = blue.
      GradientImage(i,j)->Red = brightness;
      GradientImage(i,j)->Green = brightness;
      GradientImage(i,j)->Blue = brightness;

  // Check for reflection mode
      if(reflection_mode){
    // Reflect across the horizontal midpoint      
    // Note that in reflection mode, if the pixel width is odd, the center column
    // will be written a second time here. (But with the same values as the first time.)
        GradientImage(width_pixels - i - 1, j)->Red = brightness;
        GradientImage(width_pixels - i - 1, j)->Green = brightness;
        GradientImage(width_pixels - i - 1, j)->Blue = brightness;
      }
    }
  }
 
// Write the image to the specified output file 
  GradientImage.WriteToFile(output_filename);

  printf("Gradient generation successful.\n");
}


