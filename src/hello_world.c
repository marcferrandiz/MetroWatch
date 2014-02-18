/* 
Created & Designed by: Marc Ferrandiz Borras 
Metro Watch
www.marcferrandiz.com
*/

#include <pebble.h>
#include <time.h>



/*
We have one "slot" per digit location on screen.
Because layers can only have one parent we load a digit for each
slot--even if the digit image is already in another slot.

Slot on-screen layout:
     0 1
     2 3
	 
*/
#define WIDTH  58 // dimension image number
#define HEIGHT  70
#define TOTAL_SLOTS 4 // slots
#define NUMBER_OF_IMAGES 10 //total numbers
#define EMPTY_SLOT -1 
#define UUID {0x7d,0x04,0xcb,0x24,0x49,0x88,0x42,0xb8,0xbb,0xa0,0x15,0x35,0xba,0xf2,0x90,0x43}//Universally Unique Identifier

Window *window;//initialize window
BitmapLayer *bitmap_layer[TOTAL_SLOTS];
GBitmap *bitmap[NUMBER_OF_IMAGES];
int image_slot_state[TOTAL_SLOTS] = {EMPTY_SLOT, EMPTY_SLOT, EMPTY_SLOT, EMPTY_SLOT};
// save positions of the 4 slots on the screen
struct{
	int x;
	int y;
	
}position[4]; 

const int IMAGE_RESOURCE_IDS[NUMBER_OF_IMAGES] = {
  RESOURCE_ID_IMAGE_N0, RESOURCE_ID_IMAGE_N1, RESOURCE_ID_IMAGE_N2,
  RESOURCE_ID_IMAGE_N3, RESOURCE_ID_IMAGE_N4, RESOURCE_ID_IMAGE_N5,
  RESOURCE_ID_IMAGE_N6, RESOURCE_ID_IMAGE_N7, RESOURCE_ID_IMAGE_N8,
  RESOURCE_ID_IMAGE_N9
};

// this load an image to the slot
void load_image_into_slot(int slot_number, int digit_value) {
  
     //Loads the digit image from the application's resources and
     //displays it on-screen in the correct location.
	 //The slot have to be EMPTY_SLOT.
     //Each slot is a "quarter" of the screen.
	
	// control the errors
  if ((slot_number < 0) || (slot_number >= TOTAL_SLOTS)) {
    return;
  }

  if ((digit_value < 0) || (digit_value > 9)) {
    return;
  }

  if (image_slot_state[slot_number] != EMPTY_SLOT) {
    return;
  }
	
	//update state of the slot
  	image_slot_state[slot_number] = digit_value;
	//create the bitmap
	bitmap[digit_value]=gbitmap_create_with_resource(IMAGE_RESOURCE_IDS[digit_value]);
	//create the layer
	bitmap_layer[slot_number]= bitmap_layer_create(GRect(position[slot_number].x,position[slot_number].y,WIDTH,HEIGHT));
	//set the bitmap on the layer
	bitmap_layer_set_bitmap(bitmap_layer[slot_number],bitmap[digit_value]);
	//add the bitmap(layer) to the window
  	layer_add_child(window_get_root_layer(window),bitmap_layer_get_layer(bitmap_layer[slot_number]));

}

void unload_image_from_slot(int slot_number, int digit_value) {
  /*
     Removes the digit from the display and unloads the image resource
     to free up RAM.
   */

  if (image_slot_state[slot_number] != EMPTY_SLOT) {
	// destroy bitmap
	gbitmap_destroy(bitmap[digit_value]);  
	layer_remove_from_parent(bitmap_layer_get_layer(bitmap_layer[slot_number]));
	 //destroy layer
	bitmap_layer_destroy(bitmap_layer[slot_number]);
	//update state of the slot  
    image_slot_state[slot_number] = EMPTY_SLOT;
  }
}

unsigned short get_display_hour(unsigned short hour) {
	// according to Hour configuration
  if (clock_is_24h_style()) {
    return hour;
  }
  unsigned short display_hour = hour % 12;

  // Converts "0" to "12"
  return display_hour ? display_hour : 12;

}
// 
void display_value(unsigned short value, unsigned short row_number, bool show_first_leading_zero) {
  /*
     Displays a numeric value between 0 and 60 on screen.

     Rows are ordered on screen as:

       Row 0
       Row 1

     Includes optional blanking of first leading zero,
     i.e. displays ' 0' rather than '00'.
   */
  value = value % 100; // Maximum of two digits per row.

  // Column order is: | Column 0 | Column 1 |
  // (We process the columns in reverse order because that makes
  // extracting the digits from the value easier.)
	
	for (int column_number = 1; column_number >= 0; column_number--) {
	  
		int slot_number = (row_number * 2) + column_number;
		unload_image_from_slot(slot_number, value % 10);// deinit the slot

		load_image_into_slot(slot_number, value % 10); // upload the slot
		
		value = value / 10;//Column 0
	}
}



void tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
    //Here we will update the watchface display
	display_value(get_display_hour(tick_time->tm_hour), 0, false);//HOUR
  	display_value(tick_time->tm_min, 1, true);//MINUTE
}


void handle_init(void) {
	//set positions of the layers(numbers) on the screen
	position[0].x = 9;
	position[0].y = 9;
	position[1].x = 76;
	position[1].y = 9;
	position[2].x = 9;
	position[2].y = 86;
	position[3].x = 76;
	position[3].y = 86;
	// Create a window 
	window = window_create();
	//Background Color
	window_set_background_color(window, GColorBlack);
	// Push the window into the top of the stack
	window_stack_push(window, true);
	//Get a time structure so that the face doesn't start blank
	struct tm *tick_time; //time
	time_t temp;
	temp = time(NULL);
	tick_time = localtime(&temp);// upload time on start watch

	//Manually call the tick handler when the window is loading to put tick_time
	// for default
	tick_handler(tick_time, MINUTE_UNIT);
	
	//Tick Timer every minute to upload the time
	tick_timer_service_subscribe(MINUTE_UNIT, (TickHandler) tick_handler);
	
	// App Logging!
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Just pushed a window!");
}

///// DESTROY ALL
void handle_deinit(void) {
	//destroy tick timer 
	tick_timer_service_unsubscribe();
	// Destroy the bitmaps layer
	for (int i = 0; i < TOTAL_SLOTS; i++) {
		for(int j = 0;j<NUMBER_OF_IMAGES;j++){
			unload_image_from_slot(i,j);
			
		}
    	
 	}
	// Destroy the window
	window_destroy(window);
}

/////	MAIN
int main(void) {
	handle_init();//Initialize the app elements here
	app_event_loop();//Loop
	handle_deinit();// Destroy all
}
