/*
============================================================
                        DEMO INFO

DEMO NAME:          hello_world.c
DEMO WRITTEN BY:    Muukid
CREATION DATE:      2024-09-30
LAST UPDATED:       2024-10-29

============================================================
                        DEMO PURPOSE

This demo tests if muGraphics and muTrueType work together 
by rendering the text "Hello, world." on a window. This demo
only uses the APIs of muGraphics and muTrueType, and doesn't
use anything else that muSDK provides.

If the user presses escape, the window will close.

============================================================
                        LICENSE INFO

This software is licensed under:
(MIT license OR public domain) AND Apache 2.0.
More explicit license information at the end of file.

============================================================
*/

/* Inclusion */
	
	// Include muSDK for mug + mutt
	#define MUSDK_MUG // (mug support)
	#define MU_SUPPORT_OPENGL // (For OpenGL support)
	#define MUSDK_MUTT // (mutt support)
	#define MUSDK_ALL_NAMES // (All name functions)
	#define MUSDK_IMPLEMENTATION
	#include "muSDK.h"

	// Include stdio for printing and file reading
	#include <stdio.h>
	
	// Include stdlib for allocation
	#include <stdlib.h>

	// Include inttypes for integer printf formats
	#include <inttypes.h>

	// Include string for string functions
	#include <string.h>

/* mug boilerplate variables */
	
	// Global mug context
	mugContext mug;
	// Graphic handle
	muGraphic gfx;
	// Graphic system
	muGraphicSystem gfx_system = MU_GRAPHIC_OPENGL;
	// Name of graphic system
	const char* gfx_system_name = "OpenGL";
	// The window system
	muWindowSystem window_system = MU_WINDOW_NULL; // (Auto)

	// Pixel format
	muPixelFormat format = {
		// RGBA bits
		8, 8, 8, 8,
		// Depth bits
		24,
		// Stencil bits
		0,
		// Samples
		1
	};
	// Window information
	muWindowInfo wininfo = {
		// Title
		(char*)"Hello, world.",
		// Resolution (width & height)
		800, 600,
		// Min/Max resolution (none)
		0, 0, 0, 0,
		// Coordinates (x and y)
		50, 50,
		// Pixel format
		&format,
		// Callbacks (default)
		0
	};
	// Window handle
	muWindow win;
	// Window keyboard map
	muBool* keyboard;

/* mutt boilerplate variables */

	// Font information
	muttFont font;
	// mutt result tracker
	muttResult mutt_res = MUTT_SUCCESS;

	// Default point size of rasterization
	float point_size = 50.f;
	// PPI of display for rasterization
	float PPI = 96.f;

/* Text character loading logic */

	// The text to be rendered:
	const char* text = "Hello, world.";
	// Length of the text in chars (including null-termination):
	// This number cannot be over MUG_MIN_TEXTURE_DEPTH; otherwise,
	// things may start not working
	#define text_length 14

	// Struct holding the unique information about each
	// character in the string that will be displayed
	struct Character {
		// The glyph ID of the character
		uint16_m glyph_id;
		// The rglyph for the character;
		// usually doesn't have its point/contour data loaded
		muttRGlyph rglyph;
		// The header of the character
		muttGlyphHeader header;
		// The codepoint value for the character
		char codepoint;
		// The "depth" of the character; its index in the array
		// if each empty character were discarded
		uint32_m depth;
	};
	typedef struct Character Character;

	// Loads a character given a glyph ID
	// Returns whether or not the character actually loaded
	muBool load_character(Character* character, uint16_m glyph_id) {
		// Set glyph ID
		character->glyph_id = glyph_id;

		// Get header
		mutt_res = mutt_glyph_header(&font, glyph_id, &character->header);
		// - Fail case:
		if (mutt_result_is_fatal(mutt_res)) {
			printf("Warning: glyph ID %" PRIu16 " failed to load: %s\n", glyph_id, mutt_result_get_name(mutt_res));
			return MU_FALSE;
		}

		// Get metrics of the glyph
		mutt_rglyph_metrics(&font, &character->header, glyph_id, &character->rglyph, point_size, PPI);

		// Do nothing else if the glyph is empty
		if (character->header.number_of_contours == 0) {
			return MU_TRUE;
		}

		// Calculate rglyph x/y max
		// These values are technically not validated by now,
		// but it doesn't cause anything major like crashing
		// as far as I'm aware.
		mutt_funits_punits_min_max(&font, &character->header, &character->rglyph, point_size, PPI);

		return MU_TRUE;
	}

	// Each unique character in the text
	// chars[0] is the missing character fallback; for
	// that, it's "sizeof(text)" including the '\0' to make
	// extra space for that slot if needed.
	Character chars[text_length];
	// The number of unique characters in the text
	uint32_m num_chars = 0;
	// The number of non-empty characters
	uint32_m num_filled_chars = 0;
	// The largest x_max and y_max of any loaded rglyph
	float max_x_max = 0.f;
	float max_y_max = 0.f;

	// Loops through each char in the "text" string and loads
	// it into the unique character map if need be
	// Returns if it went bad; error would already be printed,
	// just exit.
	muBool load_unique_characters(void) {
		printf("Loading character information for each char in the text \"%s\"\n", text);

		// Load missing character for chars[0]
		// There is no standard "missing glyph ID" in TrueType,
		// so I try and load codepoint 'U+25A1' which is
		// recommended by TrueType to be used as the missing
		// ideograph
		uint16_m missing_glyph_id = mutt_get_glyph(&font, 0x25A1);
		// If the glyph could not be found, it defaults to 0,
		// which we'll just use as the missing character glyph
		// ID then.
		// Load the missing character
		if (!load_character(chars, missing_glyph_id)) {
			printf("Character that failed to load was the needed fallback for a missing character; exiting.\n\n");
			return MU_FALSE;
		}
		// Increment relevant counters
		++num_chars;
		if (chars[0].header.number_of_contours != 0) {
			++num_filled_chars;
			chars[0].depth = 0;
		}

		// Loop through each char in the text string
		// (Not including null-termination)
		for (size_m i = 0; i < text_length-1; ++i) {
			// Exit if it's not unique
			muBool should_continue = MU_FALSE;
			for (size_m j = 0; j < num_chars; ++j) {
				if (chars[j].codepoint == text[i]) {
					should_continue = MU_TRUE;
					break;
				}
			}
			if (should_continue) {
				continue;
			}

			// Get glyph ID of char
			uint16_m glyph_id = mutt_get_glyph(&font, text[i]);
			// Exit if it's the missing character
			if (glyph_id == missing_glyph_id) {
				continue;
			}
			// Exit if it's not unique
			for (size_m j = 0; j < num_chars; ++j) {
				if (chars[j].glyph_id == glyph_id) {
					should_continue = MU_TRUE;
					break;
				}
			}
			if (should_continue) {
				continue;
			}

			// Attempt to load character
			// - Success:
			if (load_character(&chars[num_chars], glyph_id)) {
				chars[num_chars++].codepoint = text[i];
			}
			// - Failure:
			else {
				// Exit
				continue;
			}

			// Consider it with current largest x/y max's if
			// it has any contours
			if (chars[num_chars-1].header.number_of_contours != 0) {
				chars[num_chars-1].depth = num_filled_chars++;
				if (chars[num_chars-1].rglyph.x_max > max_x_max) {
					max_x_max = chars[num_chars-1].rglyph.x_max;
				}
				if (chars[num_chars-1].rglyph.y_max > max_y_max) {
					max_y_max = chars[num_chars-1].rglyph.y_max;
				}
			}
		}

		// Print information about loaded characters
		printf("Successfully loaded characters; %" PRIu32 " unique characters (%" PRIu32 " are empty):\n", num_chars, num_chars-num_filled_chars);
		printf("Fallback missing character - glyphID = %" PRIu16 "\n", chars[0].glyph_id);
		for (size_m i = 1; i < num_chars; ++i) {
			printf("'%c'\t - glyphID = %" PRIu16 "\n", chars[i].codepoint, chars[i].glyph_id);
		}

		printf("\n");
		return MU_TRUE;
	}

/* Text rasterization logic */

	// Rasterization method
	muttRMethod rmethod = MUTTR_FULL_PIXEL_AA8X8;

	// Handle to the text texture
	mugTexture text_texture = 0;

	// Memory for glyph/rglyph data loading:
	muByte* rdata;
	uint32_m rdata_len;
	uint32_m rdata_written;

	// Rasterizes an individual character
	// Must only be called with non-empty glyphs
	int count = 0;
	muBool rasterize_character(Character* c, muttRBitmap* bitmap) {
		printf("Rasterizing character '%c'; ", c->codepoint);
		// Load rglyph fully from header
		mutt_res = mutt_header_rglyph(&font, &c->header, &c->rglyph, point_size, PPI, rdata, &rdata_written);
		// - Non-success printing:
		if (mutt_res != MUTT_SUCCESS) {
			printf("conversion to rglyph issue: %s", mutt_result_get_name(mutt_res));
			if (mutt_result_is_fatal(mutt_res)) {
				printf(" (fatal; exiting.)\n");
				return MU_FALSE;
			} else {
				printf(" (non-fatal), ");
			}
		}

		// Rasterize glyph to pixels
		mutt_res = mutt_raster_glyph(&c->rglyph, bitmap, rmethod);
		// - Non-success printing:
		if (mutt_res != MUTT_SUCCESS) {
			printf("rasterization issue: %s", mutt_result_get_name(mutt_res));
			if (mutt_result_is_fatal(mutt_res)) {
				printf(" (fatal; exiting.)\n");
				return MU_FALSE;
			} else {
				printf(" (non-fatal), ");
			}
		}

		// Increment pixel pointer and move on
		bitmap->pixels += bitmap->stride * bitmap->height;
		printf("successful\n");
		return MU_TRUE;
	}

	// Rasterizes each unique character to pixel data
	// and creates texture for it
	muBool rasterize_characters(void) {
		printf("Rasterizing characters...\n");

		// If no character was filled, this is a strange edge case
		// that I'm not considering for now; ignore.
		if (num_filled_chars == 0) {
			printf("Edge case of all characters being empty; exiting.\n");
			return MU_FALSE;
		}

		// Allocate memory for glyph/rglyph data loading
		rdata_len = mutt_header_rglyph_max(&font);
		printf("Allocating %" PRIu32 " bytes for loading each individual rglyph\n", rdata_len);
		rdata = (muByte*)malloc(rdata_len);
		if (!rdata) {
			printf("Failed to allocate rglyph data; exiting.\n");
			return MU_FALSE;
		}

		// Describe bitmap
		muttRBitmap bitmap;
		bitmap.width = ((uint32_m)max_x_max) + 2; // (+ 2 for bleeding)
		bitmap.height = ((uint32_m)max_y_max) + 2; // (+ 2 for bleeding)
		bitmap.channels = MUTTR_RGBA; // (Full 4 channels for transparency)
		bitmap.stride = bitmap.width*4; // (Stride per horizontal row, in bytes)
		bitmap.io_color = MUTTR_BW; // (Black = outside glyph, white = inside glyph)
		printf("Describing bitmap...");
		printf(" %" PRIu32 "x%" PRIu32 "x%" PRIu32 "", bitmap.width, bitmap.height, num_filled_chars);
		printf(", RGBA (4 bytes per pixel)\n");

		// Allocate pixels, with num_filled_chars being depth (4 for RGBA):
		muByte* pixels = (muByte*)malloc(bitmap.width * bitmap.height * num_filled_chars * 4);
		if (!pixels) {
			printf("Failed to allocate pixels; exiting.\n");
			free(rdata);
			return MU_FALSE;
		}
		bitmap.pixels = pixels;

		// Loop through each unique character:
		printf("Rasterizing each character...\n");
		for (uint32_m c = 0; c < num_chars; ++c) {
			// Skip if it's empty
			if (chars[c].header.number_of_contours == 0) {
				continue;
			}
			// Rasterize
			if (!rasterize_character(&chars[c], &bitmap)) {
				free(rdata);
				free(pixels);
				return MU_FALSE;
			}
		}
		// Free rglyph data; no longer needed
		free(rdata);

		// Create texture based on this data
		printf("Creating texture...\n");
		mugTextureInfo texinfo;
		texinfo.type = MUG_TEXTURE_2D_ARRAY;
		texinfo.format = MUG_TEXTURE_U8_RGBA;
		texinfo.wrapping[0]  = texinfo.wrapping[1]  = MUG_TEXTURE_CLAMP;
		texinfo.filtering[0] = texinfo.filtering[1] = MUG_TEXTURE_NEAREST;
		uint32_m dim[3] = { bitmap.width, bitmap.height, num_filled_chars };
		text_texture = mu_gtexture_create(gfx, &texinfo, dim, pixels);

		// Deallocate and return
		free(pixels);
		printf("Successfully rasterized characters\n\n");
		return MU_TRUE;
	}

	// Deallocates resources used in text rasterization
	void dealloc_text_rasterization(void) {
		// Destroy text texture if it exists
		if (text_texture != 0) {
			printf("Destroying texture...\n");
			mu_gtexture_destroy(gfx, text_texture);
		}
	}

/* Text layout logic */

	// Array of 2D texture array objects;
	// each graphical character to be rendered
	mug2DTextureArrayRect arr_rects[text_length];
	// Originally calculated x/y values of arr_rects
	float xs[text_length], ys[text_length];
	// Corresponding buffer
	mugObjects text_buf = 0;
	// Length of this array as we progress
	uint32_m num_arr_rects = 0;
	// Total text width
	float text_width;
	// Middle of text-y based on ascender/descender
	float text_y_mid;

	// A graphical character, being each character being
	// individually rendered on screen
	struct GraphicalChar {
		// Corresponding character
		Character* c;
		// Corresponding 2D texture array object
		// Kind of unnecessary since graphical_chars has a
		// 1:1 correspondance with arr_rects, but eh, it's
		// nice to work with.
		mug2DTextureArrayRect* r;
	};
	typedef struct GraphicalChar GraphicalChar;

	// Existing graphical chars
	// length == num_arr_rects
	GraphicalChar graphical_chars[text_length];

	// Lays out the position of each character in arr_rects
	void layout_characters(void) {
		// Initial X and Y; to be set
		float x = 0.f, y;
		printf("Setting positions of each graphical character...\n");

		// Loop through each char in text
		// Not considering null-terminating character
		for (uint32_m i = 0; i < text_length-1; ++i) {
			// Find the corresponding character;
			// default on missing
			uint32_m c = 0;
			for (uint32_m j = 0; j < num_chars; ++j) {
				if (chars[j].codepoint == text[i]) {
					c = j;
					break;
				}
			}

			// Set the mid-point ascender/descender values
			// Doesn't really matter what glyph, so just do
			// the first one
			if (i == 0) {
				text_y_mid = (chars[c].rglyph.ascender - chars[c].rglyph.descender) / 2.f;
			}
			// Y is at a value that puts the top of the text
			// right at the top of the window
			y = chars[c].rglyph.ascender;

			// Skip if it's empty, but still move X forward
			if (chars[c].header.number_of_contours == 0) {
				x += chars[c].rglyph.advance_width;
				continue;
			}

			// Set values of graphical char
			graphical_chars[num_arr_rects].c = &chars[c];
			graphical_chars[num_arr_rects].r = &arr_rects[num_arr_rects];
			uint32_m gc = num_arr_rects++;

			// Set width/height
			arr_rects[gc].dim[0] = ((uint32_m)max_x_max) + 2;
			arr_rects[gc].dim[1] = ((uint32_m)max_y_max) + 2;
			// Set position
			xs[gc] = arr_rects[gc].center.pos[0] = x + (max_x_max / 2.f) + chars[c].rglyph.lsb;
			ys[gc] = arr_rects[gc].center.pos[1] = y - (max_y_max / 2.f);
			arr_rects[gc].center.pos[2] = ((float)i) / ((float)text_length);
			// Set color
			arr_rects[gc].center.col[0] = arr_rects[gc].center.col[1] = 
			arr_rects[gc].center.col[2] = arr_rects[gc].center.col[3] = 1.f;
			// Set rotation
			arr_rects[gc].rot = 0.f;
			// Set texture position
			arr_rects[gc].tex_pos[0] = arr_rects[gc].tex_pos[1] = 0.f;
			arr_rects[gc].tex_pos[2] = chars[c].depth;
			// Set texture dimensions
			arr_rects[gc].tex_dim[0] = arr_rects[gc].tex_dim[1] = 1.f;

			// Increment X by advance width
			// (If it's the last point, factor in lsb)
			if (i+1 < text_length-1) {
				x += chars[c].rglyph.advance_width;
			} else {
				x += chars[c].rglyph.x_max + chars[c].rglyph.lsb;
			}
		}

		// Calculate total text width
		// I get best results when I add first char's lsb here
		text_width = x + chars[0].rglyph.lsb;

		// Print information about completed graphical layout
		printf("Set positions of all %" PRIu32 " graphical characters\n", num_arr_rects);
		printf("Total text width: %f pixels\n", text_width);

		printf("\n");
	}

	// Centers the text in the middle of the screen
	uint32_m dim[2] = { 0, 0 };
	void layout_middle_characters(void) {
		// Get window dimensions
		uint32_m this_dim[2];
		mu_window_get(win, MU_WINDOW_DIMENSIONS, this_dim);

		// Return if dimensions are the same
		if (this_dim[0] == dim[0] && this_dim[1] == dim[1]) {
			return;
		}
		// Update dimensions variable
		dim[0] = this_dim[0];
		dim[1] = this_dim[1];

		// Calcualate x- and y-offset
		float xoffset = ((float)dim[0]) / 2.f; xoffset -= (text_width / 2.f);
		float yoffset = ((float)dim[1]) / 2.f; yoffset -= text_y_mid;

		// Loop through each graphical char
		for (uint32_m c = 0; c < num_arr_rects; ++c) {
			// Recalculate position
			arr_rects[c].center.pos[0] = xs[c] + xoffset;
			arr_rects[c].center.pos[1] = ys[c] + yoffset;
		}

		// Update buffer
		mu_gobjects_fill(gfx, text_buf, arr_rects);
	}

	// Creates the buffer for the characters
	void layout_buffer(void) {
		printf("Creating graphical object buffer...\n");
		// Create buffer
		text_buf = mu_gobjects_create(gfx, MUG_OBJECT_TEXTURE_2D_ARRAY, num_arr_rects, arr_rects);
		// Set texture for buffer
		mu_gobjects_texture(gfx, text_buf, text_texture);
		printf("Buffer created\n\n");
	}

	// Renders the buffer for the characters
	void layout_render(void) {
		// Render buffer
		mu_gobjects_render(gfx, text_buf);
	}

	// Deallocates all text layout resources
	void dealloc_layout(void) {
		if (text_buf != 0) {
			printf("Destroying graphical object buffer...\n");
			mu_gobjects_destroy(gfx, text_buf);
		}
	}

int main(void)
{
/* Print explanation */

	printf("\n===================\n");

	printf("Window with dark grey background should appear\n");
	printf("The text \"Hello, world\" should appear in the middle\n");

	printf("===================\n\n");

/* mutt initialization */

	// Open file in binary
	printf("Opening font file \"resources/fonts/font.ttf\"\n\n");
	FILE* fptr = fopen("resources/fonts/font.ttf", "rb");
	if (!fptr) {
		printf("Unable to find font; exiting.\n\n");
		return -1;
	}

	// Get size of file
	printf("Getting size of file...");
	fseek(fptr, 0L, SEEK_END);
	size_m fptr_size = ftell(fptr);
	fseek(fptr, 0L, SEEK_SET);
	printf(" size: %d bytes\n", (int)fptr_size);

	// Load data into buffer
	printf("Loading data into buffer...\n");
	muByte* data = (muByte*)malloc(fptr_size);
	if (!data) {
		printf("Unable to allocate buffer to load file; exiting.\n\n");
		fclose(fptr);
		return -1;
	}
	fread(data, fptr_size, 1, fptr);

	// Close file
	printf("Closing file...\n");
	fclose(fptr);

	// Load everything in the font
	printf("Loading font...\n");
	mutt_res = mutt_load(data, (uint64_m)fptr_size, &font, MUTT_LOAD_ALL);
	free(data); // (No longer need the original raw data)

	// Print result of loading
	if (mutt_res != MUTT_SUCCESS) {
		printf("'mutt_load' returned non-success value: %s\n", mutt_result_get_name(mutt_res));
		// Exit if result is fatal
		if (mutt_result_is_fatal(mutt_res)) {
			printf("Result is fatal; exiting.\n\n");
			return -1;
		}
	}
	printf("Successfully loaded font file \"resources/fonts/font.ttf\"\n");

	// Print if some tables failed to load
	if (font.fail_load_flags) {
		printf("Some tables failed to load\n");
	} else {
		printf("All tables loaded successfully\n");
	}

	printf("\n");

/* mug initialization */

	// Initialize mug
	printf("Initializing mug...\n");
	mug_context_create(&mug, window_system, MU_TRUE);

	// Print currently running window system
	printf("Running window system \"%s\"\n", mu_window_system_get_nice_name(muCOSA_context_get_window_system(&mug.cosa)));

	// Create graphic + window using graphic system
	printf("Creating graphic for graphic system \"%s\"...\n", gfx_system_name);
	gfx = mu_graphic_create_window(gfx_system, &wininfo);
	// Get window handle
	win = mu_graphic_get_window(gfx);
	// Get window keyboard map
	mu_window_get(win, MU_WINDOW_KEYBOARD_MAP, &keyboard);

	printf("\n");

/* Text rendering setup */

	// Load unique characters
	if (!load_unique_characters()) {
		goto termination;
	}
	// Rasterize unique characters
	if (!rasterize_characters()) {
		goto termination;
	}
	// Laying out graphical characters
	layout_characters();
	layout_buffer();

/* Main loop */

	// Run frame-by-frame while graphic exists
	while (mu_graphic_exists(gfx))
	{
		// Close if escape is pressed
		if (keyboard[MU_KEYBOARD_ESCAPE]) {
			mu_window_close(win);
			continue;
		}

		// Clear the graphic with a slightly green-ish very dark grey
		mu_graphic_clear(gfx, 15.f/255.f, 17.f/255.f, 15.f/255.f);

		// Update graphical objects
		layout_middle_characters();
		// Render graphical objects
		layout_render();

		// Swap graphic buffers (to present image)
		mu_graphic_swap_buffers(gfx);
		// Update graphic at ~100 FPS
		mu_graphic_update(gfx, 100.f);
	}

	// Termination jump marker:
	termination:

/* mug termination */

	// Destroy layout stuff (required)
	dealloc_layout();

	// Destroy rasterization stuff (required)
	dealloc_text_rasterization();

	// Destroy graphic (required)
	printf("Destroying graphic...\n");
	gfx = mu_graphic_destroy(gfx);

	// Terminate mug (required)
	printf("Terminating mug...\n");
	mug_context_destroy(&mug);

	// Print possible error

	// - mug
	if (mug.result != MUG_SUCCESS) {
		printf("Something went wrong with mug during the program's life; result: %s\n", mug_result_get_name(mug.result));
	} else {
		printf("mug worked successfully\n");
	}

	printf("\n");

/* mutt termination */

	// Deload font
	printf("Deloading font...\n");
	mutt_deload(&font);

	printf("Successful\n\n");
	return 0;
}

/*
For all source code:

	------------------------------------------------------------------------------
	This software is available under 2 licenses -- choose whichever you prefer.
	------------------------------------------------------------------------------
	ALTERNATIVE A - MIT License
	Copyright (c) 2024 Hum
	Permission is hereby granted, free of charge, to any person obtaining a copy of
	this software and associated documentation files (the "Software"), to deal in
	the Software without restriction, including without limitation the rights to
	use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
	of the Software, and to permit persons to whom the Software is furnished to do
	so, subject to the following conditions:
	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.
	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
	------------------------------------------------------------------------------
	ALTERNATIVE B - Public Domain (www.unlicense.org)
	This is free and unencumbered software released into the public domain.
	Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
	software, either in source code form or as a compiled binary, for any purpose,
	commercial or non-commercial, and by any means.
	In jurisdictions that recognize copyright laws, the author or authors of this
	software dedicate any and all copyright interest in the software to the public
	domain. We make this dedication for the benefit of the public at large and to
	the detriment of our heirs and successors. We intend this dedication to be an
	overt act of relinquishment in perpetuity of all present and future rights to
	this software under copyright law.
	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
	ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
	WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
	------------------------------------------------------------------------------

For Khronos specifications:
	
	Copyright (c) 2013-2020 The Khronos Group Inc.

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

		http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.

*/

