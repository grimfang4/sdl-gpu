#! /bin/bash

gcc main.c stb_image.c `sdl-config --cflags --libs`
