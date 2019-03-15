#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Fri Mar 15 08:43:19 2019

@author: Leonardo La Rocca
"""

import pygame
import melopero_vl53l1x.VL53L1X as mp


def main():
#    constants
    MAX_DST = 2500
    
#    initialize pygame
    pygame.init()
    width = 480
    height = 320
    game_screen = pygame.display.set_mode((width, height))
    pygame.display.set_caption('grid eye')
    
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
        
        game_screen.fill((0,0,0))
        
        dst = sensor.get_measurement()
        color = (255,0,0)
        rect = ((0,height / 2), (dst/MAX_DST * width, height/2))
        pygame.draw.rect(game_screen, color, rect)
                

                
        pygame.display.update()
        
        #time.sleep(0.1)
    
    pygame.quit()
    
    
if __name__ == '__main__':
    main()
