#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Fri Mar 15 08:43:19 2019

@author: Leonardo La Rocca
"""

import pygame
import melopero_vl53l1x as mp

def text(screen, font, size, color, xy, message): 
    font_object = pygame.font.Font(font, size)
    rendered_text = font_object.render(message, True, (color))
    screen.blit(rendered_text,(xy))

def main():
#    constants
    MAX_DST = 250
    width = 800
    height = 480
    font_size = 64
    text_color = (255,255,255)
    text_pos = (10,60)
    message = "Distance: {} cm"
    font_name = ""
    font = pygame.font.match_font(pygame.font.get_fonts()[0])
    
    rect_color = (255, 0, 0)
    
    background_color = (0,0,0)
    
#    initialize pygame
    pygame.init()
    
    pygame.mouse.set_visible(False)
    
    screen = pygame.display.set_mode((width, height))
    pygame.display.set_caption('distance sensor')
    
#    initialize the sensor
    sensor = mp.VL53L1X()
    sensor.start_ranging(mp.VL53L1X.MEDIUM_DST_MODE)
    
#    main loop
    do_main = True
    while do_main:
        pressed = pygame.key.get_pressed()
        pygame.key.set_repeat()
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                do_main = False
            
            if pressed[pygame.K_ESCAPE]:
                do_main = False
                
                
            if event.type is pygame.KEYDOWN and event.key == pygame.K_f:
                if screen.get_flags() & pygame.FULLSCREEN:
                    pygame.display.set_mode((width, height), )
                else:
                    pygame.display.set_mode((width, height), pygame.FULLSCREEN)
        
        screen.fill(background_color)
        
        dst = int(sensor.get_measurement() / 10)
        rect = ((0,height / 2), (dst/MAX_DST * width, height/4))
        pygame.draw.rect(screen, rect_color, rect)
        
        dst = MAX_DST + 1 if dst > MAX_DST or dst == 0 else dst
        format_msg = "> {}".format(MAX_DST) if dst > MAX_DST or dst == 0 else dst
        text(screen, font, font_size, text_color, text_pos, message.format(format_msg))
                
        pygame.display.update()
        
        #time.sleep(0.1)
    
    pygame.quit()
    
    
if __name__ == '__main__':
    main()
